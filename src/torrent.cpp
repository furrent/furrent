#include "torrent.hpp"

#include <limits>
#include <sstream>
#include <stdexcept>

#include "bencode/bencode_parser.hpp"
#include "bencode/bencode_value.hpp"
#include "hash.hpp"
#include "spdlog/spdlog.h"

using namespace fur::bencode;

namespace fur {

std::string File::filename() const {
  if (filepath.size() > static_cast<size_t>(std::numeric_limits<int64_t>::max())) {
    throw std::invalid_argument("file's path is too long");
  }

  std::stringstream sstream;

  int64_t i = 0;
  for (const auto& section : filepath) {
    sstream << section;
    if (i != static_cast<int64_t>(filepath.size()) - 1) sstream << "/";
    i += 1;
  }
  return sstream.str();
}

TorrentFile::TorrentFile(const BencodeValue& tree) {
  // This function receives a const reference and rightfully so: we don't need
  // to mutate the bencode tree but `BencodeValue::value()` (which we need to
  // use) returns a non-const reference. I hereby solemnly promise not to mutate
  // the argument!
  auto& tree_pinky_promise = const_cast<BencodeValue&>(tree);

  // We know the torrent starts with a dict
  auto& dict = dynamic_cast<BencodeDict&>(tree_pinky_promise).value();

  this->announce_url =
      dynamic_cast<BencodeString&>(*dict.at("announce")).value();

  auto& bencode_info_dict = dynamic_cast<BencodeDict&>(*dict.at("info"));

  std::string encoded_info_dict = BencodeParser::encode(bencode_info_dict);
  this->info_hash = hash::compute_info_hash(encoded_info_dict);

  auto& info_dict = bencode_info_dict.value();

  this->name = dynamic_cast<BencodeString&>(*info_dict.at("name")).value();

  auto it = info_dict.find("files");
  if (it != info_dict.end()) {
    // This is a multifile torrent

    auto& files_list =
        dynamic_cast<BencodeList&>(*info_dict.at("files")).value();
    for (auto& file_node : files_list) {
      auto& file_dict = dynamic_cast<BencodeDict&>(*file_node).value();

      File file;
      file.length = dynamic_cast<BencodeInt&>(*file_dict.at("length")).value();
      this->length += file.length;

      auto& filepath =
          dynamic_cast<BencodeList&>(*file_dict.at("path")).value();
      for (auto& path_node : filepath) {
        auto& segment = dynamic_cast<BencodeString&>(*path_node).value();
        file.filepath.push_back(segment);
      }

      this->files.push_back(file);
    }

  } else {
    // This is a single file torrent
    this->length = dynamic_cast<BencodeInt&>(*info_dict.at("length")).value();

    File file;
    file.length = this->length;
    file.filepath.push_back(name);
    this->files.push_back(file);
  }

  this->piece_length = static_cast<int>(
      dynamic_cast<BencodeInt&>(*info_dict.at("piece length")).value());

  auto pieces = dynamic_cast<BencodeString&>(*info_dict.at("pieces")).value();

  auto r_hashes = hash::split_piece_hashes(pieces);

  if (!r_hashes.valid()) {
    auto logger = spdlog::get("custom");
    logger->error("Could not split piece hashes: {}",
                  hash::error_to_string(r_hashes.error()));
    throw std::invalid_argument("Malformed piece hashes string");
  }

  if (r_hashes->size() > static_cast<size_t>(std::numeric_limits<int64_t>::max())) {
    throw std::out_of_range("number of pieces is too high");
  }
  this->piece_hashes = *r_hashes;

  // Integer ceil division
  this->pieces_count =
      (this->length + this->piece_length - 1) / this->piece_length;
  if (this->pieces_count != static_cast<int64_t>(this->piece_hashes.size())) {
    throw std::logic_error(
        "number of piece hashes does not match with length/pieceLength");
  }
}

// =================================================================================================
// TORRENT

Torrent::Torrent()
    : _tid{0},
      _update_interval{0},
      state{TorrentState::Error},
      pieces_processed{0} {}

Torrent::Torrent(TorrentID tid, const TorrentFile& descriptor)
    : _tid{tid},
      _descriptor{descriptor},
      _update_interval{0},
      state{TorrentState::Loading},
      pieces_processed{0} {
  announce();
}

void Torrent::announce() {
  auto response = peer::announce(_descriptor);
  if (!response.valid()) {
    auto logger = spdlog::get("custom");
    logger->critical("Error announcing to tracker!");
    return;
  }

  _update_interval = response->interval;
  _peers = response->peers;

  if (_peers.size() > static_cast<size_t>(std::numeric_limits<int64_t>::max())) {
    throw std::out_of_range("too many peers");
  }

  // Initial score is 1 for every peer
  _peers_score.clear();
  for (int64_t i = 0; i < static_cast<int64_t>(_peers.size()); i++) {
    _peers_score.emplace_back(1);
  }
}

void Torrent::atomic_add_peer_score(int64_t peer_index) {
  if (peer_index < static_cast<int64_t>(_peers_score.size())) {
    _peers_score[peer_index].fetch_add(1, std::memory_order_relaxed);
  }
}

std::discrete_distribution<int64_t> Torrent::distribution() const {
  std::vector<int64_t> scores;
  for (auto& atomic_score : _peers_score) {
    int64_t score = atomic_score.load(std::memory_order_relaxed);
    scores.push_back(score);
  }

  return {scores.begin(), scores.end()};
}

TorrentID Torrent::tid() const { return _tid; }

const TorrentFile& Torrent::descriptor() const { return _descriptor; }

std::vector<peer::Peer> Torrent::peers() const { return _peers; }

/// Generate all pieces of this torrent
std::vector<Piece> Torrent::pieces() const {
  const int64_t PIECES_COUNT = _descriptor.pieces_count;

  std::vector<Piece> pieces;
  pieces.reserve(PIECES_COUNT);

  int64_t cur_file = 0;
  int64_t cur_file_tot_size = _descriptor.files[cur_file].length;
  int64_t cur_file_rem_size = cur_file_tot_size;

  // Iterate all pieces
  for (int64_t index = 0; index < PIECES_COUNT; index++) {
    std::vector<Subpiece> subpieces;

    int64_t piece_length = _descriptor.piece_length;
    if (index == PIECES_COUNT - 1) {
      int64_t before_this_piece =
          (_descriptor.pieces_count - 1) * _descriptor.piece_length;
      piece_length = _descriptor.length - before_this_piece;
    }

    // Must create multiple subpieces
    if (cur_file_rem_size < piece_length) {
      // Keep incrementing file index until there is no
      // more space available in the piece
      int64_t piece_rem_len = piece_length;
      while (piece_rem_len > 0) {
        // Can fill entire file in the piece
        if (cur_file_rem_size <= piece_rem_len) {
          int64_t offset = cur_file_tot_size - cur_file_rem_size;
          std::string piece_filename = _descriptor.files[cur_file].filename();

          subpieces.push_back({piece_filename, offset, cur_file_rem_size});
          piece_rem_len -= cur_file_rem_size;

          // Next file
          cur_file += 1;
          cur_file_tot_size = _descriptor.files[cur_file].length;
          cur_file_rem_size = cur_file_tot_size;

        }
        // Cannot fill entire file in the piece
        else {
          int64_t offset = cur_file_tot_size - cur_file_rem_size;
          std::string piece_filename = _descriptor.files[cur_file].filename();
          subpieces.push_back({piece_filename, offset, piece_rem_len});

          cur_file_rem_size -= piece_rem_len;
          piece_rem_len = 0;
        }
      }
    }
    // Single "piece/file" mapping
    else {
      int64_t offset = cur_file_tot_size - cur_file_rem_size;
      std::string piece_filename = _descriptor.files[cur_file].filename();

      subpieces.push_back(Subpiece{piece_filename, offset, piece_length});
      cur_file_rem_size -= piece_length;
    }

    pieces.push_back(Piece{index, subpieces});
  }

  return pieces;
}

}  // namespace fur

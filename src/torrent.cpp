#include "torrent.hpp"

#include "bencode/bencode_parser.hpp"
#include "bencode/bencode_value.hpp"
#include "hash.hpp"
#include "spdlog/spdlog.h"

#include <sstream>

using namespace fur::bencode;

namespace fur {
 
std::string File::filename() const {
  std::stringstream sstream;
  for(int i = 0; i < filepath.size(); i++) {
    sstream << filepath[i];
    if (i != filepath.size() - 1)
      sstream << "/";
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

  BencodeParser parser;
  std::string encoded_info_dict = parser.encode(bencode_info_dict);
  this->info_hash = hash::compute_info_hash(encoded_info_dict);

  auto& info_dict = bencode_info_dict.value();

  this->name = dynamic_cast<BencodeString&>(*info_dict.at("name")).value();

  auto it = info_dict.find("files");
  if (it != info_dict.end()) {
    // This is a multifile torrent

    auto& files_list = dynamic_cast<BencodeList&>(*info_dict.at("files")).value();
    for (auto& file_node : files_list) {
      auto& file_dict = dynamic_cast<BencodeDict&>(*file_node).value();

      File file;
      file.length = dynamic_cast<BencodeInt&>(*file_dict.at("length")).value();
      this->length += file.length;

      auto& filepath = dynamic_cast<BencodeList&>(*file_dict.at("path")).value();
      for (auto& path_node : filepath) {
        auto& segment = dynamic_cast<BencodeString&>(*path_node).value();
        file.filepath.push_back(segment);
      }

      this->files.push_back(file);
    }

  }
  else {
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

  this->piece_hashes = *r_hashes;
  this->pieces_count = this->length / this->piece_length;
}

// =================================================================================================
// TORRENT

Torrent::Torrent()
: _tid{0}, state{TorrentState::Error}, pieces_processed{0} { }

Torrent::Torrent(TorrentID tid, const TorrentFile& descriptor)
: _tid{tid}, _descriptor{descriptor}, state{TorrentState::Loading}, pieces_processed{0} 
{
  announce();
}

std::vector<peer::Peer> Torrent::announce() {

  std::vector<peer::Peer> result;
  auto response = peer::announce(_descriptor);
  if (response.valid()) {

    _update_interval = response->interval;
    _peers = response->peers;

    // Initial score is 1 for every peer
    _peers_score.clear();
    for (int i = 0; i < _peers.size(); ++i)
      _peers_score.emplace_back(1u);

    return _peers;
  }

  auto logger = spdlog::get("custom");
  logger->critical("Error announcing to tracker!");
  // TODO: better error!

  return result;
}

void Torrent::atomic_add_peer_score(size_t peer_index) {
  if (peer_index < _peers_score.size())
    _peers_score[peer_index].fetch_add(1, std::memory_order_relaxed);
}

std::discrete_distribution<size_t> Torrent::distribution() const {

  std::vector<size_t> scores;
  for (auto& atomic_score : _peers_score) {
    size_t score = atomic_score.load(std::memory_order_relaxed);
    scores.push_back(score);
  }

  return std::discrete_distribution<size_t>(
    scores.begin(), scores.end());
}

const TorrentFile& Torrent::descriptor() const {
  return _descriptor;
}

std::vector<peer::Peer> Torrent::peers() const {
  return _peers;
}

/// Generate all pieces of this torrent
std::vector<Piece> Torrent::pieces() const {

  const size_t PIECES_COUNT = _descriptor.pieces_count;
  const size_t PIECE_LENGTH = _descriptor.piece_length;

  std::vector<Piece> pieces;
  pieces.reserve(PIECES_COUNT);

  size_t cur_file = 0;
  size_t cur_file_tot_size = _descriptor.files[cur_file].length;
  size_t cur_file_rem_size = cur_file_tot_size;

  // Iterate all pieces
  for (size_t index = 0; index < PIECES_COUNT; index++) {
    std::vector<Subpiece> subpieces;

    // Must create multiple subpieces
    if (cur_file_rem_size < PIECE_LENGTH) {

      // Keep incrementing file index until there is no
      // more space available in the piece
      size_t piece_rem_len = PIECE_LENGTH;
      while(piece_rem_len > 0) {

        // Can fill entire file in the piece
        if (cur_file_rem_size <= piece_rem_len) {

          size_t offset = cur_file_tot_size - cur_file_rem_size;
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

          size_t offset = cur_file_tot_size - cur_file_rem_size;
          std::string piece_filename = _descriptor.files[cur_file].filename();
          subpieces.push_back({piece_filename, offset, piece_rem_len});

          cur_file_rem_size -= piece_rem_len;
          piece_rem_len = 0;
        }
      }
    }
    // Single "piece/file" mapping
    else {

      size_t offset = cur_file_tot_size - cur_file_rem_size;
      std::string piece_filename = _descriptor.files[cur_file].filename();

      subpieces.push_back(Subpiece{piece_filename, offset, PIECE_LENGTH});
      cur_file_rem_size -= PIECE_LENGTH;  
    }

    pieces.push_back(Piece{ index, subpieces });
  }

  return pieces;
}

}  // namespace fur

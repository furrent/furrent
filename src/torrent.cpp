#include "torrent.hpp"

#include "bencode/bencode_parser.hpp"
#include "bencode/bencode_value.hpp"
#include "hash.hpp"
#include "spdlog/spdlog.h"

#include <sstream>

using namespace fur::bencode;

namespace fur::torrent {
 
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

      torrent::File file;
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
    
    torrent::File file;
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
}  // namespace fur::torrent

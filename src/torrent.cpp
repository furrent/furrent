#include "torrent.hpp"

#include "bencode_parser.hpp"
#include "bencode_value.hpp"
#include "hash.hpp"

namespace fur::torrent {
TorrentFile::TorrentFile(const bencode::BencodeValue& tree) {
  // We know the torrent starts with a dict
  auto& dict = dynamic_cast<const bencode::BencodeDict&>(tree).value();

  this->announce_url =
      dynamic_cast<bencode::BencodeString&>(*dict.at("announce")).value();

  auto& bencode_info_dict =
      dynamic_cast<const bencode::BencodeDict&>(*dict.at("info"));

  bencode::BencodeParser parser;
  std::string encoded_info_dict = parser.encode(bencode_info_dict);
  this->info_hash = hash::compute_info_hash(encoded_info_dict);

  auto& info_dict = bencode_info_dict.value();

  this->name =
      dynamic_cast<bencode::BencodeString&>(*info_dict.at("name")).value();

  this->length =
      dynamic_cast<bencode::BencodeInt&>(*info_dict.at("length")).value();

  this->piece_length =
      dynamic_cast<bencode::BencodeInt&>(*info_dict.at("piece_length")).value();

  auto pieces =
      dynamic_cast<bencode::BencodeString&>(*info_dict.at("pieces")).value();
  this->piece_hashes = hash::split_piece_hashes(pieces);
}
}  // namespace fur::torrent

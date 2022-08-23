#include "torrent.hpp"

#include "bencode/bencode_parser.hpp"
#include "bencode/bencode_value.hpp"
#include "hash.hpp"

using namespace fur::bencode;

namespace fur::torrent {
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

  auto& bencode_info_dict =
      dynamic_cast<BencodeDict&>(*dict.at("info"));

  BencodeParser parser;
  std::string encoded_info_dict = parser.encode(bencode_info_dict);
  this->info_hash = hash::compute_info_hash(encoded_info_dict);

  auto& info_dict = bencode_info_dict.value();

  this->name =
      dynamic_cast<BencodeString&>(*info_dict.at("name")).value();

  this->length =
      dynamic_cast<BencodeInt&>(*info_dict.at("length")).value();

  this->piece_length =
      dynamic_cast<BencodeInt&>(*info_dict.at("piece length")).value();

  auto pieces =
      dynamic_cast<BencodeString&>(*info_dict.at("pieces")).value();
  this->piece_hashes = hash::split_piece_hashes(pieces);
}
}  // namespace fur::torrent

#include "torrent.hpp"

#include <memory>

#include "bencode_value.hpp"

// TODO Add test
TorrentFile::TorrentFile(const bencode::BencodeValue& tree) {
  // We know the torrent starts with a dict
  auto& dict = dynamic_cast<const bencode::BencodeDict&>(tree).value();

  this->announce_url =
      dynamic_cast<bencode::BencodeString&>(*dict.at("announce")).value();

  // TODO Finish
}

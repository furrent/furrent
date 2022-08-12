#include "furrent.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include "bencode_parser.hpp"
#include "torrent_manager.hpp"

using namespace fur;

// Create constructor
Furrent::Furrent() {
  _downloads = std::vector<fur::manager::TorrentManager>();
}

// Add torrent to downloads

void Furrent::add_torrent(const std::string& path) {
  // Read all file data located in the path
  std::ifstream file(path);
  std::string content;
  if(file) {
    std::ostringstream ss;
    ss << file.rdbuf(); // reading data
    content = ss.str();
  }else{
    // Trow exception invalid path
    throw std::invalid_argument("fur::Furrent::add_torrent: invalid path");
  }
  // Create torrent_manager for the file
  auto parser = fur::bencode::BencodeParser();
  auto b_tree = parser.decode(content);
  auto torrent = fur::torrent::TorrentFile(*b_tree);
  manager::TorrentManager t{torrent};
  std::cout << torrent.announce_url << std::endl;

}

void Furrent::print_status() {
  std::cout << _downloads.size() << std::endl;
}

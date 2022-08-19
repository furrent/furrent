#include "furrent.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include "bencode_parser.hpp"
#include "torrent_manager.hpp"

using namespace fur;

// Create constructor
Furrent::Furrent() { _downloads = std::vector<fur::manager::TorrentManager>(); }

// Add torrent to downloads
void Furrent::add_torrent(const std::string& path) {
  // Read all file data located in the path
  std::ifstream file(path);
  std::string content;
  if (file) {
    std::ostringstream ss;
    ss << file.rdbuf();
    content = ss.str();
  } else {
    // Trow exception invalid path
    throw std::invalid_argument("fur::Furrent::add_torrent: invalid path");
  }
  // Create torrent_manager for the file
  auto parser = fur::bencode::BencodeParser();
  auto b_tree = parser.decode(content);
  auto torrent = fur::torrent::TorrentFile(*b_tree);

  manager::TorrentManager t{torrent};

  _downloads.push_back(t);
}

void Furrent::print_status() {
  std::cout << "Download length: " << _downloads.size() << std::endl;
  for (auto& t_manager : _downloads) {
    t_manager.print_status();
  }
}

manager::TorrentManager Furrent::pick_torrent() {
  // TODO: implement some kind of scheduling system
  return _downloads.front();
}

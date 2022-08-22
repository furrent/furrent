#include "furrent.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include "bencode_parser.hpp"
#include "torrent_manager.hpp"

using namespace fur;

// Create constructor
Furrent::Furrent() :
         _downloads(std::vector<manager::TorrentManager>()),
        _index(0) {
}

// Add torrent to downloads
void Furrent::add_torrent(const std::string& path){
  // Read all file data located in the path
  std::ifstream file(path);
  std::string content;
  if (file) {
    std::ostringstream ss;
    ss << file.rdbuf();
    content = ss.str();
  } else {
    // Trow exception invalid path
    throw std::invalid_argument("fur::Furrent::add_torrent: invalid path or "
        "missing permission");
  }
  // Create torrent_manager for the file
  auto parser = fur::bencode::BencodeParser();
  auto b_tree = parser.decode(content);
  auto torrent = fur::torrent::TorrentFile(*b_tree);

  manager::TorrentManager t{torrent};

  _downloads.push_back(t);
}

void Furrent::print_status() const {
  std::cout << "Files in queue:" << std::endl;
  for (auto& t_manager : _downloads) {
    t_manager.print_status();
  }
}

manager::TorrentManager& Furrent::pick_torrent() {
  // TODO: implement some kind of scheduling system
  manager::TorrentManager& picked = _downloads[_index];
  // Increment the index be careful not to go out of bounds
  _index = static_cast<int>((_index + 1) % _downloads.size());
  return picked;

}

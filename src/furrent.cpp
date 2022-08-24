#include "furrent.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <random>

#include <strategies/uniform.hpp>
#include "bencode/bencode_parser.hpp"
#include "torrent_manager.hpp"

namespace fur {

class TMTStrategy : public strategy::UniformStrategy<TorrentManager, Piece> {

 public:
  TMTStrategy()
      : UniformStrategy<TorrentManager, Piece>(false) { }

  std::optional<Piece> transform(TorrentManager& manager) override {
    auto socket = manager.get_lender_pool().get();
    auto task = manager.pick_task();
    if (task)
        return std::optional{ Piece{ *task, socket }};
    return std::nullopt;
  }
};

// Create constructor
Furrent::Furrent()
    : _downloads(std::make_shared<mt::VectorRouter<TorrentManager, Piece>>(std::make_unique<TMTStrategy>())),
      _thread_pool{_downloads, [](Piece&){ }} { }

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
    throw std::invalid_argument(
        "fur::Furrent::add_torrent: invalid path or "
        "missing permission");
  }
  // Create torrent_manager for the file
  auto parser = fur::bencode::BencodeParser();
  auto b_tree = parser.decode(content);
  auto torrent = fur::torrent::TorrentFile(*b_tree);

  /*
  _downloads->mutate([&](std::vector<TorrentManager>& vec) {
    vec.emplace_back(torrent);
  });
   */
}

void Furrent::print_status() const {
  std::cout << "Files in queue:" << std::endl;
  /*
  for (auto& t_manager : _downloads) {
    t_manager.print_status();
  }
   */
}
}

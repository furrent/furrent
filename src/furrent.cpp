#include "furrent.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <random>

#include <strategies/uniform.hpp>
#include "bencode/bencode_parser.hpp"
#include "torrent_manager.hpp"

namespace fur {

class TMTStrategy : public strategy::UniformStrategy<TorrentManagerRef, Piece> {

 public:
  TMTStrategy()
      : UniformStrategy<TorrentManagerRef, Piece>(false) { }

  std::optional<Piece> transform(TorrentManagerRef& ptr) override {
    // Check if manager exists
    if (auto manager = ptr.lock()) {
      auto socket = manager->get_lender_pool().get();
      auto task = manager->pick_task();
      if (task)
        return std::optional{ Piece{ *task, socket }};
    }
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

  // Create new shared torrent manager from torrent file
  auto manager = std::make_shared<TorrentManager>(torrent);
  _downloads.push_back(manager);

  // Create weak reference to newly added torrent manager
  TorrentManagerRef weak = manager;
  _router->insert(weak);
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

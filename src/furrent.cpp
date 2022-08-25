#include "furrent.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <random>

#include <strategies/uniform.hpp>
#include "bencode/bencode_parser.hpp"
#include "torrent_manager.hpp"

namespace fur {

// Strategy to pick a task from a list of TorrentManager, every TorrentManager
// has a local strategy to pick a task from its own list of tasks.
class TManagerStrategy : public strategy::UniformStrategy<TorrentManagerRef, Piece> {

 public:
  TManagerStrategy()
      : UniformStrategy<TorrentManagerRef, Piece>(false) { }

  std::optional<Piece> transform(TorrentManagerRef& ptr) override {
    // Check if manager exists
    if (auto manager = ptr.lock()) {
      // TODO: add LenderPool
      //auto socket = manager->get_lender_pool().get();
      auto task = manager->pick_task();
      if (task.has_value()){
        std::cout << "Manager::transform() -> piece" << std::endl;
        // TODO add socket picker from LenderPool
        return std::optional{ Piece{ task.value() }};
      }
    }
    std::cout << "Manager::transform() -> null" << std::endl;
    return std::nullopt;
  }
};

// Create constructor
Furrent::Furrent()
    : _downloads{},
      _router(std::make_shared<mt::VectorRouter<TorrentManagerRef, Piece>>(std::make_unique<TManagerStrategy>())),
      _thread_pool{_router, [](Piece&){ }} { }

// Constructor for the tests
Furrent::Furrent(std::function<void(Piece&)> fn)
    : _downloads{},
      _router(std::make_shared<mt::VectorRouter<TorrentManagerRef, Piece>>(std::make_unique<TManagerStrategy>())),
      _thread_pool{_router, std::move(fn)} {
}

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
    // TODO: manage the exception
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
}
}
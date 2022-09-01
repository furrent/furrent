#include "furrent.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <random>

#include "bencode/bencode_parser.hpp"
#include "torrent_manager.hpp"

namespace fur {

using namespace strategy;

// Create constructor
Furrent::Furrent() {
  _strategy = make_strategy_global<GlobalStrategyType::RoundRobin>();
  _workers.launch([this] (mt::Runner runner, WorkerState& state, size_t index) {
    
    // Custom download logic
    while(runner.alive()) {
      auto piece = _work_channel.extract(_strategy.get());
      // TODO
    }

  });
}

// Constructor for the tests
Furrent::Furrent(std::function<void(Piece&)> fn) {

  _strategy = make_strategy_global<GlobalStrategyType::RoundRobin>();
  _workers.launch([&] (mt::Runner runner, WorkerState& state, size_t index) {
    
    // Custom download logic
    while(runner.alive()) {
      auto piece = _work_channel.extract(_strategy.get());
      if (piece.has_value())
        fn(*piece); // TODO downloading work
    }

  });
}


Furrent::~Furrent() {
  _work_channel.set_serving(false);
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
  _downloads.push_front(manager);

  // Create weak reference to newly added torrent manager
  TorrentManagerRef weak = manager;
  _work_channel.insert(weak);
}

void Furrent::print_status() const {
  std::cout << "Files in queue:" << std::endl;
}
}
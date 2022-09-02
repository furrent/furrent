#include "furrent.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <random>

#include "bencode/bencode_parser.hpp"
#include "torrent_manager.hpp"
#include <strategy/global.hpp>

namespace fur {

using namespace strategy;

// Create constructor
Furrent::Furrent() {
  _strategy = make_strategy_global<GlobalStrategyType::RoundRobin>();
  _workers.launch([this] (mt::Runner runner, WorkerState& state, size_t index) {
    while(runner.alive()) {

      // Wait for a valid torrent to work on 
      auto torrent_result = _torrent_channel.extract(_strategy.get());
      if (torrent_result.has_error()) return;

      // If torrent is in a valid state
      auto torrent_ref = torrent_result.get_value();
      if (auto torrent = torrent_ref.lock()) {

        // If torrent still has work to do reinsert it inside channel
        if (torrent->has_tasks())
          _torrent_channel.insert(torrent_ref, _strategy.get());

        // Extract piece to work on
        auto piece_result = torrent->pick_piece();
        if (piece_result.has_error()) return;

        // TODO
        
      }
    }
  });
}

// Constructor for the tests
Furrent::Furrent(std::function<void(PieceDownloader&)> fn) {

  _strategy = make_strategy_global<GlobalStrategyType::RoundRobin>();
  _workers.launch([&] (mt::Runner runner, WorkerState& state, size_t index) {
    while(runner.alive()) {

      // Wait for a valid torrent to work on 
      auto torrent_result = _torrent_channel.extract(_strategy.get());
      if (torrent_result.has_error()) return;

      // If torrent is in a valid state
      auto torrent_ref = torrent_result.get_value();
      if (auto torrent = torrent_ref.lock()) {

        // If torrent still has work to do reinsert it inside channel
        if (torrent->has_tasks())
          _torrent_channel.insert(torrent_ref, _strategy.get());

        // Extract piece to work on
        auto piece_result = torrent->pick_piece();
        if (piece_result.has_error()) return;

        PieceDownloader downloader{ piece_result.get_value() };
        fn(downloader);
      }
    }
  });
}


Furrent::~Furrent() {
  _torrent_channel.set_serving(false);
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
  TorrentManagerWeakRef weak = manager;
  _torrent_channel.insert(weak, _strategy.get());
}

void Furrent::print_status() const {
  std::cout << "Files in queue:" << std::endl;
}
}
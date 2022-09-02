#include "furrent.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <random>
#include <sstream>

#include "bencode/bencode_parser.hpp"
#include "torrent_manager.hpp"
#include <strategy/global.hpp>

namespace fur {

using namespace strategy;

// Create constructor
Furrent::Furrent() {
  _strategy = make_strategy_global<GlobalStrategyType::RoundRobin>();
  _workers.launch([&] (mt::Runner runner, WorkerState& state, size_t index) {

    std::stringstream ss;
    while(runner.alive()) {
      // Wait for a valid torrent to work on
      auto torrent_result = _torrent_channel.extract(_strategy.get());
      if (torrent_result.has_error()) {
        switch (torrent_result.get_error())
        {
          // TODO: Decide how to recover, for now skip this iteration
          case mt::channel::StrategyChannelError::StoppedServing:
          case mt::channel::StrategyChannelError::StrategyFailed:
          case mt::channel::StrategyChannelError::Empty:
            continue;
        }
      }

      // From now on noone except us owns this TorrentManagerRef
      TorrentManager& tm = torrent_result.get_value().get();

      TorrentManager::Result descriptor_result = tm.pick_piece();
      if (descriptor_result.has_error()) {
        switch (descriptor_result.get_error())
        {
          // There are no more torrents pieces to distribute at the moment.
          // This doesn't mean that the torrent has been downloaded, so
          // for now reinsert it in the queue and continue
          case strategy::StrategyError::Empty:
            _torrent_channel.insert(tm, _strategy.get());
            continue;
        }
      }

      // With some condition decide if we should reinsert it to the list
      if(tm.unfinished())
        _torrent_channel.insert(tm, _strategy.get());

      // Now we dont have ownership of the torrent and it can be shared elsewhere

      PieceDescriptor descriptor = descriptor_result.get_value();
      // HEAVY DOWNLOAD WORK

      PieceResult result;
      result.index = descriptor.index;

      // Notify systems that this piece has been completed
      state.processed_pieces += 1;
      tm.task_done(result);
    }
  }, 1);
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
  _downloads.emplace_back(torrent);
  _torrent_channel.insert(_downloads.back(), _strategy.get());
}

void Furrent::print_status() const {
  std::cout << "Files in queue:" << std::endl;
}

int Furrent::get_total_processed_pieces() {
  int total = 0;
  for (size_t i = 0; i < _workers.get_worker_count(); i++) {
    auto& state = _workers.get_thread_state(i);
    total += state.processed_pieces;
  }
  return total;
}

}
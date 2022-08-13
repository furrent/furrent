#include "torrent_manager.hpp"
#include <iostream>

using namespace fur::manager;

TorrentManager::TorrentManager(fur::torrent::TorrentFile& torrent)
    : _torrent(torrent),
      _priority(0){
  // Create all tasks for the file
  _tasks = std::queue<manager::Task>(); // TODO change type
  auto num_tasks = static_cast<unsigned int>(torrent.length / torrent.piece_length);
  for (unsigned int i = 0; i < num_tasks; i++) {
    _tasks.push(manager::Task{i, torrent});
  }

  // Update the peer list and the announcement interval
  this->update_peers();

}

void TorrentManager::update_peers() {
  auto result = fur::peer::announce(_torrent);
  _peers = result.peers;
  _announce_interval = result.interval;
  _last_announce = time(nullptr);
}

bool TorrentManager::should_announce() const {
  return time(nullptr) - _last_announce > _announce_interval;
}
Task TorrentManager::pick_task() {
  // TODO: implement some kind of priority system
  return _tasks.front();
}

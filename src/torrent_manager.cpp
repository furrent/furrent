#include "torrent_manager.hpp"
#include <iomanip>
#include <iostream>

using namespace fur::manager;

TorrentManager::TorrentManager(fur::torrent::TorrentFile& torrent)
    : _torrent(torrent), _priority(0) {
  // Create all tasks for the file
  _tasks = std::list<manager::Task>();  // TODO change type of list
  _result = std::list<manager::Result>();  // TODO change type of list
  auto num_tasks =
      static_cast<unsigned int>(torrent.length / torrent.piece_length);
  for (unsigned int i = 0; i < num_tasks; i++) {
    _tasks.push_back(manager::Task{i, torrent});
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
  std::cout << _tasks.size() << std::endl;
  return _tasks.front();
}

void TorrentManager::task_done(const Result& r) {
  // TODO: actually the _tasks is a list, change it
  // Remove the task from the list of tasks to be done
  for(auto it = _tasks.begin(); it != _tasks.end(); ++it) {
    if(it->index == r.index) {
      _tasks.erase(it);
      break;
    }
  }
  // Add the result to the list of results
  _result.push_back(r);
}

void TorrentManager::print_status() const {
  auto num_tasks = _tasks.size()+_result.size();
  std::cout << "Status of " << _torrent.name << ": "
            << std::setw(6) << std::setfill('0') << _result.size() << "/"
            << std::setw(6) << std::setfill('0') << num_tasks <<  std::endl;
}

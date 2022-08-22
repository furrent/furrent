#include "torrent_manager.hpp"
#include <iomanip>
#include <iostream>

using namespace fur::manager;

TorrentManager::TorrentManager(fur::torrent::TorrentFile& torrent)
    : _torrent(torrent),
      _announce_interval(0),
      _last_announce(0),
      priority(0),
      num_tasks((torrent.length / torrent.piece_length)),
      num_done(0){
  // Create all tasks for the file
  result = std::list<manager::Result>();  // TODO change list
  _tasks = std::queue<manager::Task>();  // TODO change queue

  for (int i = 0; i < static_cast<int>(num_tasks); i++) {
    _tasks.push(manager::Task{i, torrent});
  }
  // Update the peer list and the announcement interval
  this->update_peers();
}

void TorrentManager::update_peers() {
  auto r = fur::peer::announce(_torrent);
  _peers = r.peers;
  _announce_interval = r.interval;
  _last_announce = time(nullptr);

}

bool TorrentManager::should_announce() const {
  return (time(nullptr) - _last_announce > _announce_interval);
}

Task TorrentManager::pick_task() {
  // TODO: implement some kind of priority system
  auto t = _tasks.front();
  _tasks.pop();
  return t;
}

void TorrentManager::task_done(const Result& r) {
  // TODO: actually the _tasks is a queue, change it
  // Add the result to the list of results
  result.push_back(r);
  num_done++;

}

void TorrentManager::task_failed(const Task& t) {
  _tasks.push(t);
  // TODO: should we do something else?
  // this->update_peers();
}

void TorrentManager::print_status() const {
  std::cout << "\t" << "Status of " << _torrent.name << ": "
            << " " << std::setw(6) << num_done
            << "/" << num_tasks << std::endl;
}
bool TorrentManager::has_tasks() const {
  // If all tasks are done, return false
  if(num_done == num_tasks) {
    return false;
  }
  return !_tasks.empty();
}

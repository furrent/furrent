#include "torrent_manager.hpp"
#include <iomanip>
#include <iostream>

namespace fur {

TorrentManager::TorrentManager(fur::torrent::TorrentFile& torrent)
    : _torrent(torrent),
      _announce_interval(0),
      _last_announce(0),
      priority(0),
      num_tasks((torrent.length / torrent.piece_length)),
      num_done(0) {
  // Create all tasks for the file
  result = std::list<Result>();  // TODO change list
  _tasks = std::vector<Task>();   // TODO change queue

  for (int i = 0; i < static_cast<int>(num_tasks); i++) {
    _tasks.push_back(Task{i, torrent, });
  }
  // Update the peer list and the announcement interval
  this->update_peers();
}
/*
TorrentManager::TorrentManager(TorrentManager&& other) noexcept {
    _torrent = other._torrent;
    _tasks = std::move(other._tasks);
    _peers = std::move(other._peers);
    _announce_interval = other._announce_interval;
    _last_announce = other._last_announce;
    //_lender_pool = std::move(other._lender_pool);
    priority = other.priority;
    num_tasks = other.num_tasks;
    num_done = other.num_done;
    result = std::move(other.result);
}

TorrentManager& TorrentManager::operator= (TorrentManager&& other) noexcept {
  _torrent = other._torrent;
  _tasks = std::move(other._tasks);
  _peers = std::move(other._peers);
  _announce_interval = other._announce_interval;
  _last_announce = other._last_announce;
  //_lender_pool = std::move(other._lender_pool);
  priority = other.priority;
  num_tasks = other.num_tasks;
  num_done = other.num_done;
  result = std::move(other.result);
  return *this;
}
*/
void TorrentManager::update_peers() {
  auto r = fur::peer::announce(_torrent);
  _peers = r.peers;
  _announce_interval = r.interval;
  _last_announce = time(nullptr);
}

bool TorrentManager::should_announce() const {
  return (time(nullptr) - _last_announce > _announce_interval);
}

std::optional<Task> TorrentManager::pick_task() {
  return (*_task_strategy)(_tasks);
}

void TorrentManager::task_done(const Result& r) {
  // TODO: actually the _tasks is a queue, change it
  // Add the result to the list of results
  result.push_back(r);
  num_done++;
}

void TorrentManager::task_failed(const Task& t) {
  _tasks.push_back(t);
  // TODO: should we do something else?
  // this->update_peers();
}

void TorrentManager::print_status() const {
  std::cout << "\t"
            << "Status of " << _torrent.name << ": "
            << " " << std::setw(6) << num_done << "/" << num_tasks << std::endl;
}
bool TorrentManager::has_tasks() const {
  // If all tasks are done, return false
  if (num_done == num_tasks) {
    return false;
  }
  return !_tasks.empty();
}

LenderPool<Socket>& TorrentManager::get_lender_pool() {
  return _lender_pool;
}

void TorrentManager::set_strategy(std::unique_ptr<mt::IVectorStrategy<Task, Task>> strategy) {
  _task_strategy = std::move(strategy);
}

}
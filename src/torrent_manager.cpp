#include "torrent_manager.hpp"
#include <iomanip>
#include <iostream>

using namespace fur::manager;

TorrentManager::TorrentManager(fur::torrent::TorrentFile& torrent)
    : _torrent(torrent), priority(0), state(TorrentState::Download) {
  // Create all tasks for the file
  _tasks = std::queue<manager::Task>();  // TODO change queue
  _result = std::list<manager::Result>();  // TODO change list
  _num_tasks = static_cast<unsigned int>(torrent.length / torrent.piece_length);
  for (unsigned int i = 0; i < _num_tasks; i++) {
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
  should_announce();
}

void TorrentManager::should_announce() {
  if(time(nullptr) - _last_announce > _announce_interval){
    // The time has passed, we should announce ourselves to the tracker
    state = TorrentState::Refresh;
  }
}

Task TorrentManager::pick_task() {
  // TODO: implement some kind of priority system
  if(state == TorrentState::Paused){
    // TODO: how to handle paused torrents?
  }
  auto t = _tasks.front();
  _tasks.pop();
  if(_tasks.empty()){
    state = TorrentState::Finished;
  }
  return t;
}

void TorrentManager::task_done(const Result& r) {
  // TODO: actually the _tasks is a queue, change it
  // Add the result to the list of results
  _result.push_back(r);
  // No more task to be done, the torrent is downloaded
  if(_result.size() == _num_tasks){
    state = TorrentState::Downloaded;
  }
}

void TorrentManager::print_status() const {
  std::cout << "\t" << "Status of " << _torrent.name << ": "
            << std::setw(10)<< TorrentStateNames[static_cast<int>(state)]
            << " " << std::setw(6) << _result.size()
            << "/" << _num_tasks << std::endl;
}

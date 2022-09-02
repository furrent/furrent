#include "torrent_manager.hpp"
#include <iomanip>
#include <iostream>

namespace fur {

TorrentManager::TorrentManager(fur::torrent::TorrentFile& torrent)
    : _torrent(torrent),
      _pieces_channel(),
      _announce_interval(0),
      _last_announce(0),
      //_task_strategy(std::make_unique<UniformTaskStrategy>()),
      priority(0),
      num_tasks((torrent.length / torrent.piece_length)),
      num_done(0),
      result(){

  // Uses mutate instead of insert to acquire lock only one time,
  // insert all pieces descriptors
  _pieces_channel.mutate([&] (std::list<PieceDescriptor>& descriptors) -> bool {
    for (int i = 0; i < (num_tasks); i++)
      descriptors.push_back({ i, 0, 0, torrent });
    return true;
  });

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

auto TorrentManager::pick_piece() -> ChannelResult {
  return _pieces_channel.try_extract(_strategy.get());
}

void TorrentManager::task_done(const PieceResult& r) {
  // TODO: actually the _tasks is a queue, change it
  // Add the result to the list of results
  result.push_back(r);
  num_done++;
}

void TorrentManager::task_failed(const PieceDescriptor& t) {
  _pieces_channel.insert(t, _strategy.get());
  // TODO: should we do something else?
  // this->update_peers();
}

void TorrentManager::print_status() const {
  std::cout << "\t"
            << "Status of " << _torrent.name << ": "
            << " " << std::setw(6) << num_done << "/" << num_tasks
            << std::endl;
}
bool TorrentManager::has_tasks() const {
  // If all tasks are done, return false
  return num_done == num_tasks;
}

LenderPool<Socket>& TorrentManager::get_lender_pool() {
  return _lender_pool;
}

/*
void TorrentManager::set_strategy(std::unique_ptr<mt::IVectorStrategy<TaskRef, TaskRef>> strategy) {
  _task_strategy = std::move(strategy);
}
*/

}
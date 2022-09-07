#include "torrent_manager.hpp"
#include <iomanip>
#include <iostream>

namespace fur {

#if 0

TorrentManager::TorrentManager(fur::torrent::TorrentFile& torrent)
    : _torrent(torrent),
      _pieces(),
      _announce_interval(0),
      _last_announce(0),
      //_task_strategy(std::make_unique<UniformTaskStrategy>()),
      priority(0),
      num_tasks((torrent.length / torrent.piece_length)),
      num_done(0),
      result(){

  // Base default strategy
  _strategy = strategy::make_strategy_local<strategy::LocalStrategyType::Streaming>(*this);

  /*
  // Uses mutate instead of insert to acquire lock only one time,
  // insert all pieces descriptors
  _pieces.mutate([&] (std::list<PieceDescriptor>& descriptors) -> bool {
    for (int i = 0; i < num_tasks; i++)
      descriptors.push_back({ i, 0, 0, torrent });
    return true;
  });
  */

  for (int i = 0; i < num_tasks; i++)
      _pieces.push_back({ i, 0, 0, torrent });

  // Update the peer list and the announcement interval
  this->update_peers(); // TODO: how to handle errors !?
}

auto TorrentManager::update_peers() -> util::Result<bool> {
  auto r = fur::peer::announce(_torrent);
  if(!r){
    return util::Result<bool>::ERROR(const_cast<util::Error&&>(r.error()));
  }
  _peers = (*r).peers;
  _announce_interval = (*r).interval;
  _last_announce = time(nullptr);
  return util::Result<bool>::OK(true);
}

bool TorrentManager::should_announce() const {
  return (time(nullptr) - _last_announce > _announce_interval);
}

auto TorrentManager::pick_piece() -> Result {
  return _strategy->extract(_pieces);
}

void TorrentManager::task_done(const PieceResult& r) {
  // TODO: actually the _tasks is a queue, change it
  // Add the result to the list of results
  num_done++;
}

void TorrentManager::task_failed(const PieceDescriptor& t) {
  _strategy->insert(t, _pieces);
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

download::lender_pool::LenderPool<Socket>& TorrentManager::get_lender_pool() {
  return _lender_pool;
}

bool TorrentManager::unfinished() {
  return !_pieces.empty() && num_done != num_tasks;
}

#endif

}
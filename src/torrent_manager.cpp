#include "torrent_manager.hpp"
#include <iomanip>
#include <iostream>
#include <strategies/uniform.hpp>

namespace fur {

class UniformTaskStrategy : public strategy::UniformStrategy<TaskRef, TaskRef> {

 public:
  UniformTaskStrategy()
      : UniformStrategy<TaskRef, TaskRef>(true) { }

  std::optional<TaskRef> transform(TaskRef& task) override {
    // Return optional of task
    return std::optional<TaskRef>{ task };
  }
};

TorrentManager::TorrentManager(fur::torrent::TorrentFile& torrent)
    : _torrent(torrent),
      _tasks(),
      _announce_interval(0),
      _last_announce(0),
      _task_strategy(std::make_unique<UniformTaskStrategy>()),
      num_tasks((torrent.length / torrent.piece_length)),
      num_done(0),
      result(){
  // Create the list of tasks
  for (int i = 0; i < static_cast<int>(num_tasks); i++) {
    _tasks.push_back(std::make_shared<Task>(Task{i, torrent}));
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

std::optional<TaskRef> TorrentManager::pick_task() {
  std::cout << "TorrentManager::pick_task()" << std::endl;
  return (*_task_strategy)(_tasks);
}

void TorrentManager::task_done(const Result& r) {
  // TODO: actually the _tasks is a queue, change it
  // Add the result to the list of results
  result.push_back(r);
  num_done++;
}

void TorrentManager::task_failed(const TaskRef& t) {
  _tasks.push_back(t);
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
  if (num_done == num_tasks) {
    return false;
  }
  return !_tasks.empty();
}

LenderPool<Socket>& TorrentManager::get_lender_pool() {
  return _lender_pool;
}

void TorrentManager::set_strategy(std::unique_ptr<mt::IVectorStrategy<TaskRef, TaskRef>> strategy) {
  _task_strategy = std::move(strategy);
}

}
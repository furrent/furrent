#include <bencode/bencode_parser.hpp>
#include <fstream>
#include <furrent.hpp>
#include <iostream>
#include <log/logger.hpp>
#include <policy/policy.hpp>
#include <random>
#include <sstream>
#include <tasks/torrent_task.hpp>

namespace fur {

TorrentHandle::TorrentHandle(TorrentHandleID uid, const std::string& filename)
  : uid{uid}, filename{filename},
      pieces_processed{0},
      last_announce_time{{}}
{ 
  // At the beginning the announce time is the creation time
  last_announce_time = std::chrono::high_resolution_clock::now();
}

// This task is very expensive but it is executed one time every X minutes
bool TorrentHandle::regenerate_peers() {
  if (!torrent.has_value()) return false;

  // Default global logger
  auto logger = spdlog::get("custom");
  logger->info("Regenerating list of peers for {}, may take a few seconds",
               filename);

  auto response = peer::announce(*torrent);
  if (!response.valid()) return false;

  // New peer discovery interval
  announce_interval.exchange(response->interval);

  std::stringstream log_text;
  log_text << "Regenerated list of peers for " << filename << " (next interval "
           << announce_interval << " s):\n";

  std::vector<Peer> new_peers;
#if 1
  for (auto& peer : response->peers) {
    // Check if it is a good peer
    download::downloader::Downloader d(*torrent, peer);
    auto result = d.ensure_connected();
    if (!result.valid()) {
      log_text << "\t" << peer.address() << " REFUSED\n";
      continue;
    }

    log_text << "\t" << peer.address() << " OK\n";
    new_peers.push_back(peer);
  }
#else
  new_peers = response->peers;
#endif

  // If no peer is valid then the operation failed
  if (new_peers.empty()) {
    logger->info(log_text.str());
    return false;
  }

  {
    // Update peers list
    std::unique_lock<std::shared_mutex> lock(mtx);
    peers.clear();
    peers = new_peers;
  }

  logger->info(log_text.str());
  return true;
}

TorrentSnapshot TorrentHandle::snapshot() const {

  // Allow only concurrent reads of the handle
  std::shared_lock<std::shared_mutex> read_lock(mtx);
  TorrentSnapshot snapshot;

  // Constant data
  snapshot.uid = uid;
  snapshot.filename = filename;
  snapshot.torrent = torrent;
  snapshot.peers = peers;

  // Loading is relaxed because read_lock is already a memory fence
  snapshot.state = state.load(std::memory_order_relaxed);
  snapshot.priority = priority.load(std::memory_order_relaxed);
  snapshot.pieces_processed = pieces_processed.load(std::memory_order_relaxed);

  return snapshot;
}

Furrent::Furrent() {

  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;

  /// This is the core of all workers
  const size_t concurrency = std::thread::hardware_concurrency();
  const size_t threads_cnt = (concurrency > 1) ? concurrency - 1 : 1; 

  _workers.launch(std::bind(&Furrent::thread_main, this, _1, _2, _3), threads_cnt);
}

Furrent::~Furrent() {
  _global_queue.begin_skip_waiting();
  _workers.terminate();
}

void Furrent::thread_main(mt::Runner runner, WorkerState& state, size_t index) {
  
  // Default global logger
  auto logger = spdlog::get("custom");
  
  mt::PriorityPolicy task_policy;
  while(runner.alive()) {
    
    auto global_work = _global_queue.try_extract(task_policy);
    if (global_work.valid()) {
        (*global_work)->execute();
    } else {
      logger->info("thread {:02d} is waiting for work on global queue", index);
      _global_queue.wait_work();
    }
  }
}

size_t Furrent::add_torrent(const std::string& filename) {
  std::unique_lock<std::shared_mutex> lock(_mtx);

  const size_t torrent_uid = descriptor_next_uid;
  descriptor_next_uid += 1;

  /// Allocate descriptor for the new torrent
  auto descriptor = std::make_shared<TorrentHandle>(torrent_uid, filename);
  _torrents.emplace(torrent_uid, descriptor);

  /// Begin loading task
  _global_queue.insert(std::make_unique<mt::TorrentFileLoad>(&_global_queue, descriptor));

  return torrent_uid;
}

std::vector<TorrentSnapshot> Furrent::get_torrents_snapshot() const {
  std::shared_lock<std::shared_mutex> lock(_mtx);

  std::vector<TorrentSnapshot> result;
  for (auto& item : _torrents)
    result.push_back(item.second->snapshot());
  return result;
}

std::optional<TorrentSnapshot> Furrent::get_snapshot(TorrentHandleID uid) {
  std::shared_lock<std::shared_mutex> lock(_mtx);
  
  auto it = _torrents.find(uid);
  if (it != _torrents.end())
    return std::make_optional(it->second->snapshot());

  return std::nullopt;
}

std::optional<TorrentSnapshot> Furrent::remove_torrent(TorrentHandleID uid) {

  // Remove all tasks refering to the removed torrent 
  _global_queue.mutate([&](mt::ITask::Wrapper& wrapper) -> bool {

    // Mutate only if it is a TorrentTask
    auto task = dynamic_cast<mt::TorrentTask*>(wrapper.get());
    if (task == nullptr)
      return true;

    if (task->descriptor->uid == uid)
      return false;

    return true;
  });

  // Remove the torrent handle descriptor from furrent
  std::unique_lock<std::shared_mutex> lock(_mtx);
  auto it = _torrents.find(uid);
  if (it != _torrents.end()) {

    std::shared_ptr<TorrentHandle>& handle = it->second;
    TorrentSnapshot snapshot = handle->snapshot();

    // Write lock, all snapshot must be completed
    std::unique_lock<std::shared_mutex> write_lock(handle->mtx);
    handle->state.exchange(TorrentState::Stopped);
    _torrents.erase(it);

    return std::make_optional(snapshot);
  }

  return std::nullopt;
}

void Furrent::pause_torrent(TorrentHandleID uid) {
  _global_queue.mutate([&](mt::ITask::Wrapper& wrapper) -> bool {
    
    // Mutate only if it is a TorrentTask
    auto task = dynamic_cast<mt::TorrentTask*>(wrapper.get());
    if (task == nullptr)
      return true;

    if (task->descriptor->uid == uid)
      task->state = mt::TaskState::Paused;

    return true;
  });
}

void Furrent::resume_torrent(TorrentHandleID uid) {
  _global_queue.mutate([&](mt::ITask::Wrapper& wrapper) -> bool {

    // Mutate only if it is a TorrentTask
    auto task = dynamic_cast<mt::TorrentTask*>(wrapper.get());
    if (task == nullptr)
      return true;

    if (task->descriptor->uid == uid)
      task->state = mt::TaskState::Running;

    return true;
  });
}

}  // namespace fur
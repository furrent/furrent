#include <bencode/bencode_parser.hpp>
#include <fstream>
#include <furrent.hpp>
#include <iostream>
#include <log/logger.hpp>
#include <policy/policy.hpp>
#include <random>
#include <sstream>
#include <tasks/torrent.hpp>

namespace fur {

TorrentDescriptor::TorrentDescriptor(const std::string& filename)
    : filename{filename},
      pieces_downloaded{0},
      pieces_saved{0},
      announce_time{std::chrono::high_resolution_clock::time_point{}},
      split_output_spawned{false} {}

// This task is very expensive but it is executed one time every X minutes
bool TorrentDescriptor::regenerate_peers() {
  if (!torrent.has_value()) return false;

  // Default global logger
  auto logger = spdlog::get("custom");
  logger->info("Regenerating list of peers for {}, may take a few seconds",
               filename);

  auto response = peer::announce(*torrent);
  if (!response.valid()) return false;

  interval = response->interval;

  std::stringstream log_text;
  log_text << "Regenerated list of peers for " << filename << " (next interval "
           << interval << " s):\n";

  std::vector<Peer> new_peers;
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

  // If no peer is valid then the operation failed
  if (new_peers.empty()) return false;

  {
    std::unique_lock<std::shared_mutex> lock(mtx);
    downloaders.clear();
    downloaders = new_peers;
  }

  logger->info(log_text.str());
  return true;
}

bool TorrentDescriptor::download_finished() {
  std::shared_lock<std::shared_mutex> lock(mtx);
  const size_t total_pieces = torrent->length / torrent->piece_length;
  return pieces_downloaded == total_pieces;
}

Furrent::Furrent() {
  /// Local queues of all workers
  const size_t concurrency = std::thread::hardware_concurrency();
  _local_queues = new TaskSharingQueue[concurrency];

  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;

  /// This is the core of all workers
  _workers.launch(std::bind(&Furrent::thread_main, this, _1, _2, _3));
}

Furrent::~Furrent() {
  _global_queue.begin_skip_waiting();
  _workers.terminate();

  delete[] _local_queues;
}

void Furrent::thread_main(mt::Runner runner, WorkerState& state, size_t index) {
  // TODO: custom policy per thread etc...
  mt::PriorityPolicy task_policy;
  const size_t concurrency = _workers.get_worker_count();

  // Default global logger
  auto logger = spdlog::get("custom");

  // How many times we have to fail stealing to go to sleep
  const int STEALING_LIMIT = 100;
  // How many times the thread tried to steal without success
  int failed_stealing_count = 0;

  auto& local_queue = _local_queues[index];
  while (runner.alive()) {
    // First we check our own local queue
    auto local_work = local_queue.try_extract(task_policy);
    if (local_work.valid()) {
      // logger->debug("thread {:02d} is executing local work", index);
      (*local_work)->execute(local_queue);
      failed_stealing_count = 0;
    } else {
      // Then we check the global queue
      auto global_work = _global_queue.try_extract(task_policy);
      if (global_work.valid()) {
        // logger->debug("thread {:02d} is executing global work", index);
        (*global_work)->execute(_global_queue);
        failed_stealing_count = 0;
      } else {
        logger->info("thread {:02d} is waiting for work on global queue",
                     index);
        _global_queue.wait_work();
        failed_stealing_count = 0;
#if 0
                // If there is nothing then we steal from a random worker
                else {
                    size_t steal_idx = rand() % concurrency;
                    if (steal_idx == index) steal_idx = (steal_idx + 1) % concurrency;

                    auto steal_work = _local_queues[steal_idx].steal();
                    if (steal_work.valid()) {

                        //logger->info("thread {:02d} is executing stolen work from {:02d}", index, steal_idx);
                        (*steal_work)->execute(local_queue);
                        failed_stealing_count = 0;
                    }
                    else {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        failed_stealing_count += 1;
                    }
                }
#endif
      }
    }
  }
}

void Furrent::add_torrent(const std::string& filename) {
  /// Allocate descriptor for the new torrent
  _descriptors.emplace_back(filename);
  auto& descriptor = _descriptors.front();

  /// Begin loading task
  _global_queue.insert(std::make_unique<tasks::TorrentFileLoad>(descriptor));
}

const std::list<TorrentDescriptor>& Furrent::get_descriptors() const {
  return _descriptors;
}

}  // namespace fur
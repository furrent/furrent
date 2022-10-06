#include <chrono>
#include <furrent.hpp>
#include <tasks/torrent_task.hpp>

namespace fur::mt {

TorrentPeerRefresh::TorrentPeerRefresh(std::shared_ptr<TorrentHandle> descr)
: TorrentTask(descr) { }

TorrentPeerRefresh::TorrentPeerRefresh(SharedQueue<Wrapper>* spawn_queue, std::shared_ptr<TorrentHandle> descr)
: TorrentTask(spawn_queue, descr) { }

size_t TorrentPeerRefresh::priority() const {
  
  /*
  auto now = std::chrono::high_resolution_clock::now();
  auto old = descriptor->last_announce_time.load(std::memory_order_relaxed);

  // Priority is low when the interval time has not been exceeded
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - old);
  return (elapsed.count() > descriptor->announce_interval) ? mt::Priority::PRIORITY_HIGH
                                                           : mt::Priority::PRIORITY_NONE;
  */

  return mt::Priority::PRIORITY_HIGH;
}

void TorrentPeerRefresh::execute() {

  // Copy the state of the torrent handle in thread local memory 
  TorrentSnapshot snapshot = descriptor->snapshot();

  // If the torrent has been stopped we return, effectively removing this task
  if (descriptor->state.load() == TorrentState::Stopped) {
    logger->info("Removed refresh peers task of stopped torrent ({})", descriptor->uid);
    return;
  }

  // Add refresh peers only if there are still pieces to process
  /*
  if (snapshot.pieces_processed != snapshot.torrent.pieces_count) {

    // Very expensive operation but executed only very rarily
    while (!descriptor->regenerate_peers())
      std::this_thread::sleep_for(std::chrono::seconds(5));

    // Update the last announce time
    auto now = std::chrono::high_resolution_clock::now();
    descriptor->last_announce_time.exchange(now);

    // Insert same task for the future
    local_queue.insert(std::make_unique<TorrentPeerRefresh>(descriptor));
  }
  */
}

}  // namespace fur::tasks

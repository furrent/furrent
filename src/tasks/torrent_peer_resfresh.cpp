#include <tasks/torrent.hpp>

#include <chrono>

namespace fur::tasks {
    
TorrentPeerRefresh::TorrentPeerRefresh(TorrentDescriptor& descr)
: _descriptor{descr} { }

size_t TorrentPeerRefresh::priority() const {
  // Priority is low when the interval time has not been exceeded
  auto now = std::chrono::high_resolution_clock::now();
  auto old = _descriptor.announce_time;
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - old);
  return (elapsed.count() > _descriptor.interval) ? mt::PRIORITY_HIGH : mt::PRIORITY_LOW;
}

void TorrentPeerRefresh::execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) {

    // Very expensive operation but executed only very rarily
    while(!_descriptor.regenerate_peers())
      std::this_thread::sleep_for(std::chrono::seconds(5));

    // Add refresh peers only if there are still pieces to download
    if (!_descriptor.download_finished()) {
      auto now = std::chrono::high_resolution_clock::now();
      _descriptor.announce_time = now;
      local_queue.insert(std::make_unique<TorrentPeerRefresh>(_descriptor));
    }
}

} // namespace fur::task

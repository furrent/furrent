#include <tasks/torrent.hpp>

namespace fur::tasks {
    
TorrentPeerRefresh::TorrentPeerRefresh(TorrentDescriptor& descr)
: _descriptor{descr} { }

void TorrentPeerRefresh::execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) {
    // Very expensive operation but executed only very rarily
    while(!_descriptor.regenerate_peers())
        std::this_thread::sleep_for(std::chrono::seconds(5));
}

} // namespace fur::task

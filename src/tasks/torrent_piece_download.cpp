#include <tasks/torrent.hpp>

namespace fur::tasks {

TorrentPieceDownload::TorrentPieceDownload(TorrentDescriptor& desc, download::PieceDescriptor piece)
: _descriptor{desc}, _piece{piece} { }

void TorrentPieceDownload::execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) {

    // Default global logger
    auto logger = spdlog::get("custom");

    fur::torrent::TorrentFile torrent;
    fur::peer::Peer peer;

    {
        // Allows multiple readers
        _descriptor.mtx.lock_shared();

        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - _descriptor.announce_time);
        
        // If the elapsed time is greater than the interval time we should spawn a refresh-peers-task
        if (elapsed.count() > _descriptor.interval) {
            _descriptor.to_refresh = true;

            _descriptor.mtx.unlock_shared();
            _descriptor.mtx.lock();

            if (_descriptor.to_refresh) {
                logger->info("Spawning refresh peer task for torrent {}", _descriptor.filename);

                local_queue.insert(std::make_unique<TorrentPeerRefresh>(_descriptor));
                _descriptor.announce_time = now;
                _descriptor.to_refresh = false;
            }

            _descriptor.mtx.unlock();
            _descriptor.mtx.lock_shared();
        }

        const size_t dsize = _descriptor.downloaders.size();
        const size_t index = (_piece.index + _piece.attempts) % dsize;
        peer = _descriptor.downloaders[index];
        torrent = *_descriptor.torrent;

        _descriptor.mtx.unlock_shared();
    }

    auto download_begin = std::chrono::high_resolution_clock::now();
    download::downloader::Downloader d(*_descriptor.torrent, peer);
    auto result = d.try_download(_piece);

    // Download completed!
    if (result.valid()) {

        auto download_end = std::chrono::high_resolution_clock::now();
        auto download_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(download_end - download_begin);

        const size_t piece_length = torrent.piece_length;
        const size_t pieces_count = torrent.length / piece_length;

        auto downloaded = *result;
        logger->info("Download of piece {:5}/{} from {} COMPLETED ({} bytes, {} ms)", 
            _piece.index + 1, pieces_count, peer.address(), 
            downloaded.content.size(), download_elapsed.count());

        _descriptor.downloaded_pieces += 1;
        // TODO: Spawn write to file task
    }
    // Download failed!
    else {

        // Failed too many times
        //_descriptor.valid[peer_index] = false;

        /*
        logger->info("Download of piece {} of {} for {} from {} FAILED, retrying...", 
            _piece.index + 1, pieces_count, _descriptor.filename, peer.address());
        */

        // Retry operation by creating a new DownloadPieceTask
        _piece.attempts += 1;
        local_queue.insert(std::make_unique<TorrentPieceDownload>(
            _descriptor, _piece));
    }
}

} // namespace fur::task

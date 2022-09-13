#include <tasks/torrent.hpp>

#include <fstream>

#include <config.hpp>

namespace fur::tasks {

TorrentPieceDownload::TorrentPieceDownload(TorrentDescriptor& desc, download::PieceDescriptor piece)
: _descriptor{desc}, _piece{piece} { }

void TorrentPieceDownload::execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) {

    // Default global logger
    auto logger = spdlog::get("custom");

    fur::torrent::TorrentFile torrent;
    fur::peer::Peer peer;

    {
        // Allows multiple readers, extract peer information and torrent file
        std::shared_lock<std::shared_mutex> lock(_descriptor.mtx);

        const size_t count = _descriptor.downloaders.size();
        const size_t index = (_piece.index + _piece.attempts) % count;
        peer = _descriptor.downloaders[index];
        torrent = *_descriptor.torrent;
    }

    auto download_begin = std::chrono::high_resolution_clock::now();
    download::downloader::Downloader d(*_descriptor.torrent, peer);
    auto result = d.try_download(_piece);

    // Download completed!
    if (result.valid()) {

        auto download_end = std::chrono::high_resolution_clock::now();
        auto download_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            download_end - download_begin);

        const size_t piece_length = torrent.piece_length;
        const size_t pieces_count = torrent.length / piece_length;

        auto downloaded = *result;
        logger->info("Download of piece {:5}/{} from {} COMPLETED ({} bytes, {} ms)",
                     _piece.index + 1, pieces_count, peer.address(),
                     downloaded.content.size(), download_elapsed.count());

        _descriptor.downloaded_pieces += 1;

        auto write_begin = std::chrono::high_resolution_clock::now();
        {
            // Write bytes to memory
            std::unique_lock<std::shared_mutex> lock(_descriptor.mtx);

            // Create output file on system and open an handle
            auto& stream_ptr = _descriptor.torrent->stream_ptr;
            stream_ptr->seekp(_piece.offset, std::ios_base::beg);
            stream_ptr->write(reinterpret_cast<char*>(&result->content[0]), result->content.size());
        }
        auto write_end = std::chrono::high_resolution_clock::now();
        auto write_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(write_end - write_begin);
        logger->info("Writing of piece {}/{} to file {} COMPLETED ({} ms)",
                     _piece.index + 1, pieces_count, torrent.name, write_elapsed.count());
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

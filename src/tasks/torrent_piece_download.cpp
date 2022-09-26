#include <tasks/torrent.hpp>

#include <fstream>

#include <config.hpp>
#include <platform/io.hpp>

namespace fur::tasks {

TorrentPieceDownload::TorrentPieceDownload(TorrentDescriptor& desc, download::Piece piece)
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

        _descriptor.pieces_downloaded.fetch_add(1, std::memory_order_relaxed);

        auto downloaded = *result;
        logger->info("Download piece {:5}/{} from {} COMPLETED ({} bytes, {} ms)",
                     _piece.index + 1, torrent.pieces_count, peer.address(),
                     downloaded.content.size(), download_elapsed.count());

        auto write_begin = std::chrono::high_resolution_clock::now();
        {
            std::shared_lock<std::shared_mutex> lock(_descriptor.mtx);

            // Write every subpiece
            size_t piece_offset = 0;
            for(auto& subpiece: _piece.subpieces) {
                
                const auto& file = _descriptor.torrent->files[subpiece.file_index];
                const std::string filepath = config::DOWNLOAD_FOLDER + std::string("/") + file.filename();

                const std::vector<uint8_t> file_data(
                    downloaded.content.begin() + piece_offset, 
                    downloaded.content.begin() + piece_offset + subpiece.len
                );
                fur::platform::io::write_bytes(filepath, downloaded.content, subpiece.file_offset);
                piece_offset += subpiece.len;
            }

            // This must be true, otherwise there has been a corruption
            assert(downloaded.content.size() == piece_offset);
        }

        auto write_end = std::chrono::high_resolution_clock::now();
        auto write_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(write_end - write_begin);
        logger->info("Writing piece {}/{} to file {} COMPLETED ({} ms)",
                     _piece.index + 1, torrent.pieces_count, torrent.name, write_elapsed.count());
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

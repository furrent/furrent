#include <tasks/torrent_task.hpp>

#include <fstream>
#include <cassert>

#include <config.hpp>
#include <platform/io.hpp>
#include <furrent.hpp>

namespace fur::mt {

TorrentPieceDownload::TorrentPieceDownload(std::shared_ptr<TorrentHandle> descr, download::Piece piece)
: TorrentTask(descr), piece{piece} { }

TorrentPieceDownload::TorrentPieceDownload(SharedQueue<Wrapper>* spawn_queue, std::shared_ptr<TorrentHandle> descr, download::Piece piece)
: TorrentTask(spawn_queue, descr), piece{piece} { }

size_t TorrentPieceDownload::priority() const {

    /*
    auto state = descriptor->state.load(std::memory_order_relaxed);
    auto priority = descriptor->priority.load(std::memory_order_relaxed);

    // Force fast removal of stopped tasks
    if (state == TorrentState::Stopped) 
        return mt::Priority::PRIORITY_HIGH + priority;

    // Priority is normal by default but becomes none when the torrent is paused
    return (state == TorrentState::Downloading) ? mt::Priority::PRIORITY_LOW + priority
                                                : mt::Priority::PRIORITY_NONE;
    */

   return mt::Priority::PRIORITY_LOW;
}

void TorrentPieceDownload::execute() {

    // Copy the state of the torrent handle in thread local memory 
    TorrentSnapshot snapshot = descriptor->snapshot();

#if 1
    assert(snapshot.torrent.has_value());   // File must be loaded! 
    assert(snapshot.torrent->files.size() != 0);
    assert(snapshot.peers.size() != 0);
#endif

    // If the torrent has been stopped we return, effectively removing this task
    if (snapshot.state == TorrentState::Stopped) {
        logger->info("Removed task ({}) of stopped torrent ({})", 
            piece.index + 1, snapshot.uid);

        return;
    }

    // Find a suitable peer for the download
    const size_t peer_index = (piece.index + piece.attempts) % snapshot.peers.size();
    fur::peer::Peer peer = snapshot.peers[peer_index];

    // Download piece from peer
    auto download_begin = std::chrono::high_resolution_clock::now();
    download::downloader::Downloader d(*snapshot.torrent, peer);
    
    logger->info("Dowloading of piece {} for torrent[{}]...", piece.index, snapshot.uid);
    auto download_result = d.try_download(piece);
    if (download_result.valid()) {

        auto download_end = std::chrono::high_resolution_clock::now();
        auto download_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            download_end - download_begin);

        auto download = *download_result;
        descriptor->pieces_processed.fetch_add(1, std::memory_order_relaxed);
        
        logger->info("Dowloading of piece {} for torrent[{}]... COMPLETED ({} bytes, {} ms)", 
            piece.index, snapshot.uid, download.content.size(), download_elapsed.count());
        
#if 0

        auto write_begin = std::chrono::high_resolution_clock::now();
        {
            // The operating system will protect the writing on the files
            std::shared_lock<std::shared_mutex> lock(descriptor->mtx);

            // Write every subpiece
            size_t piece_offset = 0;
            for(int i = 0; i < piece.subpieces.size(); i++) {
                
                const auto& subpiece = piece.subpieces[i];
                const auto& file = descriptor->torrent->files[subpiece.file_index];
                const std::string filepath = config::DOWNLOAD_FOLDER + std::string("/") + file.filename();

                const std::vector<uint8_t> file_data(
                    downloaded.content.begin() + piece_offset, 
                    downloaded.content.begin() + piece_offset + subpiece.len
                );
                if (!fur::platform::io::write_bytes(filepath, downloaded.content, subpiece.file_offset).valid()) {
                    logger->error("Unable to write to file (piece: {}, subpiece: {}) of {}", 
                        piece.index, i, descriptor->uid);

                    // TODO: Corruption? Hard Exit
                }

                piece_offset += subpiece.len;
            }

            // This must be true, otherwise there has been a corruption
            assert(downloaded.content.size() == piece_offset);
        }

        auto write_end = std::chrono::high_resolution_clock::now();
        auto write_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(write_end - write_begin);
        logger->info("Writing piece {}/{} to file {} COMPLETED",
                     piece.index + 1, torrent.pieces_count, torrent.name);

        descriptor->pieces_saved.fetch_add(1, std::memory_order_relaxed);

#endif
    }
    // Download failed!
    else {

        piece.attempts += 1;

        /*
        logger->warn("Dowloading of piece {} for torrent[{}]... FAILED ({} attempts)", 
            piece.index, snapshot.torrent->name, piece.attempts);
        */

        // Retry operation by creating a new DownloadPieceTask
        spawn(std::make_unique<TorrentPieceDownload>(descriptor, piece));
    }
}

} // namespace fur::task

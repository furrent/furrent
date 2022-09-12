#include <tasks/torrent.hpp>

#include <bencode/bencode_parser.hpp>
#include <fstream>

namespace fur::tasks {
    
TorrentFileLoad::TorrentFileLoad(TorrentDescriptor& desc)
: _descriptor(desc) { }

void TorrentFileLoad::execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) {

    // Default global logger
    auto logger = spdlog::get("custom");

    std::ifstream file(_descriptor.filename);
    std::string content;

    if (file) {

        logger->info("Loading torrent file from {}", _descriptor.filename);

        std::ostringstream ss;
        ss << file.rdbuf();
        content = ss.str();

    } else {

        logger->error("Error loading torrent from {}", _descriptor.filename);

        // TODO: manage the exception
        /*
        throw std::invalid_argument(
            "fur::Furrent::add_torrent: invalid path or "
            "missing permission");
        */
       return;
    }

    // Create torrent_manager for the file
    logger->info("Parsing torrent file {}", _descriptor.filename);
    auto parser = fur::bencode::BencodeParser();
    auto b_tree = parser.decode(content);

    // From now on the descriptor is available to all
    _descriptor.torrent = std::make_optional<fur::torrent::TorrentFile>(*b_tree);
    while(!_descriptor.regenerate_peers())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const size_t piece_length = _descriptor.torrent->piece_length;
    const size_t pieces_count = _descriptor.torrent->length / piece_length;

    // Generate all downloading tasks
    logger->info("Generating {} pieces torrent file {}", pieces_count, _descriptor.filename);
    for(size_t index = 0; index < pieces_count; index++) {
        size_t offset = index * piece_length;
        
        download::PieceDescriptor piece { index, offset, 0 };
        local_queue.insert(std::make_unique<TorrentPieceDownload>(
            _descriptor, piece));
    }
}

} // namespace fur::task

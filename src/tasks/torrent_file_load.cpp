#include <bencode/bencode_parser.hpp>
#include <config.hpp>
#include <fstream>
#include <platform/io.hpp>
#include <tasks/torrent.hpp>

namespace fur::tasks {

TorrentFileLoad::TorrentFileLoad(TorrentDescriptor& desc) : _descriptor(desc) {}

void TorrentFileLoad::execute(
    mt::SharingQueue<mt::ITask::Wrapper>& local_queue) {
  // Default global logger
  auto logger = spdlog::get("custom");

  std::string content;
  std::ifstream file(_descriptor.filename);
  if (file.good()) {
    logger->info("Loading torrent file from {}", _descriptor.filename);

    std::ostringstream ss;
    ss << file.rdbuf();
    content = ss.str();

  } else {
    logger->error("Error loading torrent from {}", _descriptor.filename);
    file.close();
    return;
  }

  // Create torrent_manager for the file
  logger->info("Parsing torrent file {}", _descriptor.filename);
  auto parser = fur::bencode::BencodeParser();
  auto b_tree = parser.decode(content);

  if (!b_tree.valid()) {
    logger->error("Error parsing torrent file: {}",
                  bencode::error_to_string(b_tree.error()));
    return;
  }

  // From now on the descriptor is available to all
  _descriptor.torrent =
      std::make_optional<fur::torrent::TorrentFile>(*(*b_tree));
  while (!_descriptor.regenerate_peers())
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Create file of correct size
  std::string output_filename =
      config::DOWNLOAD_FOLDER + _descriptor.torrent->name;
  auto file_creation =
      fur::platform::io::touch(output_filename, _descriptor.torrent->length);
  if (!file_creation.valid()) {
    logger->error("Error creating file for torrent {}", _descriptor.filename);
    return;
  }

  // Open output stream for file
  _descriptor.torrent->stream_ptr =
      std::make_shared<std::ofstream>(output_filename);

  const size_t piece_length = _descriptor.torrent->piece_length;
  const size_t pieces_count = _descriptor.torrent->length / piece_length;

  // Generate all downloading tasks
  logger->info("Generating {} pieces torrent file {}", pieces_count,
               _descriptor.filename);
  for (size_t index = 0; index < pieces_count; index++) {
    size_t offset = index * piece_length;

    download::PieceDescriptor piece{index, offset, 0};
    local_queue.insert(
        std::make_unique<TorrentPieceDownload>(_descriptor, piece));
  }

  // Generate refresh peers task
  local_queue.insert(std::make_unique<TorrentPeerRefresh>(_descriptor));
  file.close();
}

}  // namespace fur::tasks

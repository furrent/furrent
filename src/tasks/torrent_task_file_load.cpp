#include <bencode/bencode_parser.hpp>
#include <config.hpp>
#include <fstream>
#include <platform/io.hpp>
#include <tasks/torrent_task.hpp>

namespace fur::tasks {

TorrentFileLoad::TorrentFileLoad(std::shared_ptr<TorrentHandle> desc) 
: TorrentTask(desc) {}

void TorrentFileLoad::execute(
    mt::SharingQueue<mt::ITask::Wrapper>& local_queue) {
      
  // ===================================================================
  // Loading torrent file

  descriptor->state.exchange(TorrentState::Loading);

  std::string content;
  std::ifstream file(descriptor->filename);
  if (file.good()) {
    logger->info("Loading torrent file from {}", descriptor->filename);

    std::ostringstream ss;
    ss << file.rdbuf();
    content = ss.str();

  } else {
    logger->error("Error loading torrent from {}", descriptor->filename);
    file.close();
    return;
  }

  // ===================================================================
  // Parsing torrent file

  descriptor->state.exchange(TorrentState::Parsing);

  // Create torrent_manager for the file
  logger->info("Parsing torrent file {}", descriptor->filename);
  auto parser = fur::bencode::BencodeParser();
  auto b_tree = parser.decode(content);

  if (!b_tree.valid()) {
    logger->error("Error parsing torrent file: {}",
                  bencode::error_to_string(b_tree.error()));
    return;
  }

  // ===================================================================
  // Generating peers

  descriptor->state.exchange(TorrentState::Indexing);

  {
    std::unique_lock<std::shared_mutex> lock(descriptor->mtx);

    // From now on the descriptor is available to all
    descriptor->torrent =
        std::make_optional<fur::torrent::TorrentFile>(*(*b_tree));
  }

  // Find trackers
#if 1
  while(!descriptor->regenerate_peers())
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif

  // ===================================================================
  // Creating output folders and files

  // Lock for read
  std::shared_lock<std::shared_mutex> lock(descriptor->mtx);

  // Create output files
  for(auto& file : descriptor->torrent->files) {
    auto filepath = fur::platform::io::create_subfolders(
      config::DOWNLOAD_FOLDER, file.filepath);
    
    if (!filepath.valid()) {
      // TODO
      break;
    }

    if (!fur::platform::io::touch(*filepath, file.length).valid()) {
      logger->error("Error creating file for torrent {}", descriptor->filename);
      return;
    } 
  }

  // ===================================================================
  // Creating download piece tasks 

  descriptor->state.exchange(TorrentState::Downloading);

  const size_t piece_length = descriptor->torrent->piece_length;
  const size_t pieces_count = descriptor->torrent->length / piece_length;

  // Generate all downloading tasks
  logger->info("Generating {} pieces torrent file {}", pieces_count,
               descriptor->filename);

  auto& torrent = *descriptor->torrent;

  size_t cur_file = 0;
  size_t cur_file_tot_size = torrent.files[cur_file].length;
  size_t cur_file_rem_size = cur_file_tot_size;

  // Iterate all pieces
  for (size_t index = 0; index < pieces_count; index++) {
    std::vector<download::Subpiece> subpieces;

    // Must create multiple subpieces
    if (cur_file_rem_size < piece_length) {

      // Keep incrementing file index until there is no
      // more space available in the piece
      size_t piece_rem_len = piece_length;
      while(piece_rem_len > 0) {

        // Can fill entire file in the piece
        if (cur_file_rem_size <= piece_rem_len) {

          size_t offset = cur_file_tot_size - cur_file_rem_size;
          subpieces.push_back({cur_file, offset, cur_file_rem_size});
          piece_rem_len -= cur_file_rem_size;
          
          // Next file
          cur_file += 1;
          cur_file_tot_size = torrent.files[cur_file].length;
          cur_file_rem_size = cur_file_tot_size;
        }
        // Cannot fill entire file in the piece
        else {

          size_t offset = cur_file_tot_size - cur_file_rem_size;
          subpieces.push_back({cur_file, offset, piece_rem_len});
          cur_file_rem_size -= piece_rem_len;
          piece_rem_len = 0;
        }
      }
    }
    // Single "piece/file" mapping
    else {

      size_t offset = cur_file_tot_size - cur_file_rem_size;
      subpieces.push_back(download::Subpiece{cur_file, offset, piece_length});
      cur_file_rem_size -= piece_length;  
    }

    // Create piece download task
    download::Piece piece{index, subpieces, 0};
    local_queue.insert(
      std::make_unique<TorrentPieceDownload>(descriptor, piece));
  }

  // Generate refresh peers task
  // BUG: Starvation using high_performance_clock
  //local_queue.insert(std::make_unique<TorrentPeerRefresh>(descriptor));
  file.close();
}

}  // namespace fur::tasks

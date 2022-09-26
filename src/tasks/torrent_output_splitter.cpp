#include <tasks/torrent.hpp>

#include <platform/io.hpp>
#include <config.hpp>

#include <string>
#include <fstream>

namespace fur::tasks {

TorrentOutputSplitter::TorrentOutputSplitter(TorrentDescriptor& descr)
    : _descriptor{descr} {}

void TorrentOutputSplitter::execute(
    mt::SharingQueue<mt::ITask::Wrapper>& local_queue) {

  // No need for the output stream now
  // TODO: Move elsewhere
  _descriptor.torrent->stream_ptr->close();

  std::string temp_filename =
    config::DOWNLOAD_FOLDER + _descriptor.torrent->name;



  auto& torrent = *_descriptor.torrent;
  if (torrent.files.size() == 0) {
    // Single file is easy
    // TODO: move to output folder from temp folder
  }
  else {
    // Multi file not so much

    size_t offset = 0;
    for(auto& file : torrent.files)
      auto filepath = fur::platform::io::create_subfolders(
        config::DOWNLOAD_FOLDER, file.filepath);

      if (!filepath.valid()) {
        // TODO
        break;
      }

      fur::platform::io::transfer_bytes(temp_filename, filepath.value(), offset, file.length);
      offset += file.length;
    }


  }

}  // namespace fur::tasks

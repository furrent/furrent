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

  std::string output_filename =
    config::DOWNLOAD_FOLDER + _descriptor.torrent->name;

  auto& torrent = *_descriptor.torrent;
  if (torrent.files.size() == 0) {
    // Single file is easy
    // TODO: move to output folder from temp folder
  }
  else {
    // Multi file not so much
    
    /*
    std::ifstream input_stream(input_filename, ifstream::binary);
    for (auto& file : torrent.files) {

      const size_t path_size = file.filepath.size();

      std::stringstream ss;
      for (int i = 0; i < path_size i++) {
        ss << file.filepath[i];
        if (i < path_size - 1)
          ss << "/";
        else if (i == path_size - 1)
          platform::io::create_subfolders(ss.str());
      } 

      std::ofstream output_stream(ss.str());
      */


    }


  }

}  // namespace fur::tasks

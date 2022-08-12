#pragma once

#include "string"
#include "vector"
#include "queue"
#include "peer.hpp"

/// @file torrent_manager.h
/// @brief TorrentManager class declaration.
/// Every torrent file is mapped to a TorrentManager object

/// @namespace fur::manager
/// @brief Namespace for the torrent manager.
/// Contains the TorrentManager, every torrent file is mapped to a
/// TorrentManager object.
namespace fur::manager {
/// @class TorrentManager
/// Used to store every sub-data of a torrent file because is divided into many
/// Result, after the entire download there are to combine all the Results into
/// a single file
struct Result{
  int index;
  std::vector<uint8_t> content;
};

struct Task{
  int index;
  fur::torrent::TorrentFile &torrent;
};

/// Every download is mapped to a TorrentManager object... TODO
class TorrentManager {
  private:
    unsigned int                  _priority;
    fur::torrent::TorrentFile     _torrent;
    std::queue<Task>              _task;      // TODO: replace queue
    std::queue<Result>            _result;    // TODO: replace queue
    std::vector<fur::peer::Peer>  _peers;
    torrent::TorrentFile          _torrent_file;

  public:
    TorrentManager(fur::torrent::TorrentFile torrent);
    ~TorrentManager() = default;

    // Function for creating the list of tasks for the download, every file is
    // divided into many parts that are then downloaded in parallel
    // \return The list of tasks for the download
    // std::vector<Task> get_tasks() const;

};

} // namespace fur::manager
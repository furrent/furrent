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

/// Used to store the data of a torrent file that is divided into many Results,
/// after the entire download there are to combine all the Results into a single
/// file
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
    std::queue<Task>              _task;
    std::queue<Result>            _result;
    std::vector<fur::peer::Peer>  _peers;
  public:
    TorrentManager(unsigned int priority,
                   fur::torrent::TorrentFile torrent,
                   std::queue<Task> task,
                   std::queue<Result> result,
                   std::vector<fur::peer::Peer> peers);
    ~TorrentManager();

};

} // namespace fur::manager
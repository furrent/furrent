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
  unsigned int index;
  std::vector<uint8_t> content;
};

struct Task{
  unsigned int index;
  fur::torrent::TorrentFile &torrent;
};

/// Every download is mapped to a TorrentManager object
class TorrentManager {
 private:
    /// The parsed .torrent file
    fur::torrent::TorrentFile     _torrent;
    /// List of tasks to be done for the download
    std::queue<Task>              _tasks;      // TODO: replace queue
    /// List of downloaded pieces that have to be combined into a single file
    std::queue<Result>            _result;    // TODO: replace queue
    /// List of peers to download the file from
    std::vector<fur::peer::Peer>  _peers;
    /// The announce interval is the time (in seconds) we're expected to re-announce
    int                           _announce_interval;
    /// Last time we announced ourselves to the tracker
    time_t                        _last_announce;
  public:
    /// Priority of the file
    unsigned int                  _priority;
    /// Constructor for the TorrentManager class
    explicit TorrentManager(fur::torrent::TorrentFile &torrent);
    /// Function to get the next task to be done
    Task pick_task();
    /// Update the list of peers to download the file from
    void update_peers();
    /// Function that returns true if we have to re-announce ourselves
    /// and refresh the list of peers
    [[nodiscard]] bool should_announce() const;



};

} // namespace fur::manager
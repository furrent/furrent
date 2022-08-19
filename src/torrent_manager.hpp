#pragma once

#include "string"
#include "vector"
#include "list"
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

/// Enumeration for the different states of a torrent
enum class TorrentState {
  /// The torrent is downloading
  Download,
  /// The torrent is paused
  Paused,
  /// Need to refreshing the list of peers
  Refresh,
  /// There are no more tasks to be done
  Finished,
  /// All tasks are done, the torrent is downloaded
  Downloaded
};

/// @brief Names of the different states of a torrent.
static const char* TorrentStateNames[] = {
  "Download",
  "Paused",
  "Refresh",
  "Finished",
  "Downloaded"
};


/// @class TorrentManager
/// Used to store every sub-data of a torrent file because is divided into many
/// Result, after the entire download there are to combine all the Results into
/// a single file
struct Result{
  unsigned int index;
  std::vector<uint8_t> content;
};

/// @class TorrentManager
/// Every torrent file has many pieces to download, each piece is a Task that
/// have to be done
struct Task{
  unsigned int index;
  fur::torrent::TorrentFile &torrent;
};

/// Every torrent to download is mapped to a TorrentManager object
class TorrentManager {
 private:
    /// The parsed .torrent file
    fur::torrent::TorrentFile     _torrent;
    /// List of tasks to be done for the download
    std::queue<Task>              _tasks;      // TODO: replace queue
    /// List of downloaded pieces that have to be combined into a single file
    std::list<Result>            _result;    // TODO: replace queue
    /// List of peers to download the file from
    std::vector<fur::peer::Peer>  _peers;
    /// The announce interval is the time (in seconds) we're expected to re-announce
    int                           _announce_interval{};
    /// Last time we announced ourselves to the tracker
    time_t                        _last_announce{};
    /// The number of tasks that we have to do
    unsigned int                  _num_tasks;
  public:
    /// Priority of the torrent
    unsigned short int            priority;
    /// Status of the torrent
    TorrentState                  state;
    /// Constructor for the TorrentManager class
    explicit TorrentManager(fur::torrent::TorrentFile &torrent);
    /// Function to get the next task to be done
    Task pick_task();
    /// Function to call when a task is done, it removes the task from the list
    /// of tasks to be done and adds the result to the list of results
    void task_done(const Result& r);
    /// Update the list of peers to download the file from
    void update_peers();
    /// Function that returns true if we have to re-announce ourselves
    /// and refresh the list of peers
    void should_announce();
    /// Debug function to print the status of the TorrentManager object
    void print_status() const;
};

} // namespace fur::manager
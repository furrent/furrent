#pragma once

#include "string"
#include "vector"
#include "list"
#include "queue"
#include <optional>

#include "peer.hpp"

#include <download/lender_pool.hpp>
#include <mt/channel.hpp>
#include <strategy/local.hpp>


/// Namespace for the torrent manager. Contains the TorrentManager, every
/// torrent file is mapped to a TorrentManager object.
namespace fur {
/// Used to store every sub-data of a torrent file because is divided into many
/// Result, after the entire download there are to combine all the Results into
/// a single file
struct Result{
  int index;
  std::vector<uint8_t> content;
};

/// TODO: Make this real
struct Socket {};

/// Every torrent file has many pieces to download, each piece is a Task that
/// have to be done
struct Task {
  int index;
  fur::torrent::TorrentFile &torrent;
};

typedef std::shared_ptr<Task> TaskRef;

/// Work item for the workers, used to download a piece of the torrent
struct Piece {
  TaskRef task;
  // TODO: add LenderPool
  //LenderPool<Socket>::Borrow socket;
};

/// Every torrent to download is mapped to a TorrentManager object
class TorrentManager {

    typedef std::unique_ptr<strategy::ILocalStrategy> MyStrategy;

 private:
    /// The parsed .torrent file
    fur::torrent::TorrentFile       _torrent;
    /// List of tasks to be done for the download
    std::list<TaskRef>            _tasks;      // TODO: replace queue
    /// List of peers to download the file from
    std::vector<fur::peer::Peer>    _peers;
    /// The announce interval is the time (in seconds) we're expected to
    /// re-announce
    int                             _announce_interval;
    /// Last time we announced ourselves to the tracker
    time_t                          _last_announce;
    /// Pool of reusable sockets
    LenderPool<Socket>              _lender_pool;
    /// Define a strategy for choosing the task
    MyStrategy _strategy;

  public:
    /// Priority of the torrent
    int                             priority;
    /// The number of tasks that we have to do
    int                             num_tasks;  // Only for calculate it one
                                                // time
    /// The number of tasks that we have done
    int                             num_done;   // The _result size can't
                                                // be use because it's size
                                                // decrease each time the file
                                                // is written to the disk
    /// List of downloaded pieces that have to be combined into a single file
    std::list<Result>               result;     // TODO: replace queue
    /// Constructor for the TorrentManager class
    /// TODO: Initialize sockets
    explicit TorrentManager(fur::torrent::TorrentFile& torrent);

    // LenderPool is not copyable
    TorrentManager(TorrentManager& other) = delete;
    TorrentManager& operator= (TorrentManager& other) = delete;
    TorrentManager(TorrentManager&& other) noexcept = delete;
    TorrentManager& operator= (TorrentManager&& other) noexcept = delete;

    /// Function to know if there are tasks to do
    [[nodiscard]] bool has_tasks() const;
    /// Function to get the next task to be done
    std::optional<TaskRef> pick_task();
    /// Function to call when a task is done, it removes the task from the list
    /// of tasks adds the result to the list of results
    void task_done(const Result& r);
    /// Function to call when a task is failed, it put back the task in the list
    void task_failed(const TaskRef& t);
    /// Update the list of peers to download the file from
    void update_peers();
    /// Function that put the state of the current object to Refresh if the time
    /// has passed the announce interval
    [[nodiscard]] bool should_announce() const;
    /// Debug function to print the status of the TorrentManager object
    void print_status() const;

    /// Set the stategy used to pick a piece for the workers
    void set_strategy(MyStrategy strategy);
    /// Returns the _lender_pool of the current object
    LenderPool<Socket>& get_lender_pool();
};

/// Type used to reference a Torrent Manager without owning it or moving it
using TorrentManagerRef = std::weak_ptr<TorrentManager>;

} // namespace fur
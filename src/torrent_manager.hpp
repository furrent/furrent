#pragma once

#include <string>
#include <list>
#include <optional>
#include <atomic>

#include <peer.hpp>
#include <download/lender_pool.hpp>

/// Namespace for the torrent manager. Contains the TorrentManager, every
/// torrent file is mapped to a TorrentManager object.
namespace fur {

#if 0

/// Used to store every sub-data of a torrent file because is divided into many
/// Result, after the entire download there are to combine all the Results into
/// a single file
struct PieceResult{
  int index;
  std::vector<uint8_t> content;
};

/// TODO: Make this real
struct Socket {};

/// Describes a piece to download
struct PieceDescriptor {

  /// Index of the piece in order inside the torrent
  int index;
  /// Size of the piece
  size_t size;
  /// Offset from the beginning of the file
  size_t offset;
  /// Torrent file containing this piece
  torrent::TorrentFile& torrent;

};

/// Contains all data required to try the download of a piece
struct PieceDownloader {

  /// Contains generic information about the piece
  PieceDescriptor descriptor;
  /// Manager for this piece, it could have been remove from the UI 
  //std::weak_ptr<TorrentManager> manager;
  /// Socket used for the comunication
  //LenderPool<Socker>::Borrow socker;
  
};

/// Every torrent to download is mapped to a TorrentManager object
class TorrentManager {
public:

    typedef std::unique_ptr<strategy::ILocalStrategy> Strategy;
    typedef typename strategy::ILocalStrategy::Result Result;

 private:
    /// The parsed .torrent file
    fur::torrent::TorrentFile _torrent;
    /// all Pieces remaining to be processed workers
    std::list<PieceDescriptor> _pieces;
    /// Strategy used to extract downloader from the descriptors
    Strategy _strategy;

    /// List of peers to download the file from
    std::vector<fur::peer::Peer> _peers;

    /// The announce interval is the time (in seconds) we're expected to
    /// re-announce
    int                             _announce_interval;
    /// Last time we announced ourselves to the tracker
    time_t                          _last_announce;
    /// Pool of reusable sockets
    download::lender_pool::LenderPool<Socket>              _lender_pool;

  public:
    /// Priority of the torrent
    int                             priority;
    /// The number of tasks that we have to do
    int                             num_tasks;  // Only for calculate it one
                                                // time
    /// The number of tasks that we have done
    std::atomic_int                 num_done;   // The _result size can't
                                                // be use because it's size
                                                // decrease each time the file
                                                // is written to the disk
    /// List of downloaded pieces that have to be combined into a single file
    std::list<PieceResult>               result;     // TODO: replace queue
    /// Constructor for the TorrentManager class
    /// TODO: Initialize sockets
    explicit TorrentManager(fur::torrent::TorrentFile& torrent);

    // ============================================================================
    // LenderPool is not copyable or movable because of LenderPool

    TorrentManager(TorrentManager& other) = delete;
    TorrentManager& operator= (TorrentManager& other) = delete;
    TorrentManager(TorrentManager&& other) noexcept = delete;
    TorrentManager& operator= (TorrentManager&& other) noexcept = delete;

    // ============================================================================

    /// Function to know if there are tasks to do
    [[nodiscard]] bool has_tasks() const;
    /// Function to get the next task to be done
    Result pick_piece();
    /// Function to call when a task is done, it removes the task from the list
    /// of tasks adds the result to the list of results
    void task_done(const PieceResult& r);
    /// Function to call when a task is failed, it put back the task in the list
    void task_failed(const PieceDescriptor& t);
    /// Update the list of peers to download the file from
    util::Result<bool> update_peers();
    /// Function that put the state of the current object to Refresh if the time
    /// has passed the announce interval
    [[nodiscard]] bool should_announce() const;
    /// Debug function to print the status of the TorrentManager object
    void print_status() const;
    /// Set the stategy used to pick a piece for the workers
    void set_strategy(Strategy strategy);
    /// Returns the _lender_pool of the current object
    download::lender_pool::LenderPool<Socket>& get_lender_pool();

    /// @return True if not all pieces have been downloaded
    bool unfinished();
};

/// Type used to reference a Torrent Manager without owning it or moving it
using TorrentManagerRef = std::reference_wrapper<TorrentManager>;

#endif

} // namespace fur
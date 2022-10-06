#pragma once

#include <download/downloader.hpp>
#include <download/lender_pool.hpp>
#include <mt/group.hpp>
#include <mt/sharing_queue.hpp>
#include <tasks/torrent_task.hpp>
#include <torrent.hpp>

#include <shared_mutex>
#include <unordered_map>

namespace fur {

enum class TorrentState {
  Loading,
  Parsing,
  Indexing,
  Downloading,
  Paused,
  Completed,
  Stopped,
  Error,
};

/// UID for torrent handles
using TorrentHandleID = unsigned int;

/// Minimal amount of data easily transfered between thread boundaries
/// rappresents a torrent state in a single point in time
struct TorrentSnapshot {
  
  /// Unique torrent identifier
  TorrentHandleID uid;
  /// Current state
  TorrentState state;
  /// Priority of the torrent
  size_t priority;
  /// Name of the .torrent file
  std::string filename;
  /// File descriptor if it is loaded
  std::optional<TorrentFile> torrent;
  /// All peers that are available at the moment
  std::vector<Peer> peers;
  /// Number of pieces that have been procesed
  size_t pieces_processed;
};

/// Atomic time point 
using atomic_time_point = std::atomic<std::chrono::high_resolution_clock::time_point>;

/// Rapprensents an mutable active torrent download
struct TorrentHandle
{
  // Protects internal state, allows multiple readers but only one writer
  mutable std::shared_mutex mtx;

  /// Unique ID for this torrent
  const TorrentHandleID uid;
  /// Name of the file where the torrent can be found
  const std::string filename;
  
  /// Parsed torrent file
  std::optional<torrent::TorrentFile> torrent;
  /// Peers' where to ask for the pieces
  std::vector<Peer> peers;

  /// Current state of the torrent
  std::atomic<TorrentState> state;
  /// Priority of the torrent
  std::atomic_uint32_t priority;

  // Time of last announce
  atomic_time_point last_announce_time;
  /// Interval to next update
  std::atomic_uint32_t announce_interval;

  /// Number of pieces downloaded and written to file
  std::atomic_uint32_t pieces_processed;

  /// Constructs a new torrent handle
  /// @param uid unique id of the torrent
  /// @param filename filename of the .torrent file
  TorrentHandle(TorrentHandleID uid, const std::string& filename);

  /// Regenerate list of peers
  bool regenerate_peers();

  // Generate a snapshot of the torrent in this moment
  TorrentSnapshot snapshot() const;
};

/// Main state of the program
class Furrent {
  
  typedef mt::SharedQueue<mt::ITask::Wrapper> TorrentTaskQueue;
  typedef std::unordered_map<TorrentHandleID, std::shared_ptr<TorrentHandle>> TorrentHandleMap; 

  /// State of the worker threads
  struct WorkerState {};

  /// Mutex protecting furrent state
  mutable std::shared_mutex _mtx;
  /// List of torrents to download
  //TorrentDescriptorList _descriptors;

  /// All created torrents
  TorrentHandleMap _torrents;

  /// Global work queue
  TorrentTaskQueue _global_queue;
  /// Pool managing worker threads
  mt::ThreadGroup<WorkerState> _workers;

  /// Incremental torrent descriptor next id
  TorrentHandleID descriptor_next_uid = 0;

 public:

  Furrent();
  virtual ~Furrent();

  /// Begin download of a torrent
  /// @param filename filename of the .torrent file
  /// @return the uid of the new torrent descriptor
  size_t add_torrent(const std::string& filename);

  /// @return all loaded descriptors
  std::vector<TorrentSnapshot> get_torrents_snapshot() const;

  /// Retrive the snapshot of a torrent descriptor 
  /// @param uid uid of the torrent
  /// @return snapshot of the torrent, if it exists
  std::optional<TorrentSnapshot> get_snapshot(TorrentHandleID uid);

  /// Removes a torrent descriptor and all of his tasks
  /// @param uid uid of the torrent to remove
  /// @return snapshot of the torrent an the moment of removal, if it exists
  std::optional<TorrentSnapshot> remove_torrent(TorrentHandleID uid);

  /// Pause the download of a torrent
  /// @param uid uid of the torrent to pause
  void pause_torrent(TorrentHandleID uid);

  /// Resume the download of a torrent, wakeup all threads
  /// @param uid uid of the torrent to resume
  void resume_torrent(TorrentHandleID uid);

  /// Callbacks from the UI
 public:
  /*
  bool callback_torrent_insert(const std::string &fine_name, const std::string &file_path);
  bool callback_torrent_remove(const TorrentSnapshot &torrent);
  bool callback_torrent_update(const TorrentSnapshot &torrent);
  bool callback_setting_update(const std::string &path);
  */

 private:
  /// Main function of all workers
  void thread_main(mt::Runner runner, WorkerState& state, size_t index);
};

}  // namespace fur
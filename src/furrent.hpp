#pragma once

#include <download/downloader.hpp>
#include <download/lender_pool.hpp>
#include <mt/group.hpp>
#include <mt/sharing_queue.hpp>
#include <shared_mutex>
#include <torrent.hpp>
#include <types.hpp>
#include <unordered_map>
#include <util/singleton.hpp>

namespace fur {

struct TorrentGuiData {
  TorrentID tid;
  TorrentState state;
  std::string filename;

  size_t pieces_processed;
  size_t pieces_count;
};

/// Contains usefull statistics retrived
/// during the processing of a piece
struct PieceTaskStats {
  /// True if the operation was successfull
  bool completed;
  /// Index of the used peer
  size_t used_peer;
};

/// Class responsible for processing a piece
class PieceTask {
  // Downloaded piece content
  std::optional<download::Downloaded> _data;

 public:
  /// Identifier of the owner torrent
  TorrentID tid;
  /// Piece to process
  Piece piece;
  /// .torrent descriptor
  TorrentFile descriptor;

 public:
  /// Constructs an empty temporary piece task
  explicit PieceTask();
  /// Constructs a new piece task
  PieceTask(TorrentID tid, Piece piece, const TorrentFile& descriptor);

  /// Process piece from downloading to saving
  /// @param peer peer to use for the download
  PieceTaskStats process(const peer::Peer& peer);

 private:
  /// Download from a peer
  bool download(const peer::Peer& peer);
  /// Save to file
  [[nodiscard]] bool save() const;
};

/// Main state of the program
/// NB: All added torrent handle descriptor will never be removed from memory!
class Furrent : public Singleton<Furrent> {
  /// State of the worker threads
  struct WorkerState {
    /// Total number of pieces processed
    size_t piece_processed = 0;
  };

  /// Pool managing worker threads
  mt::ThreadGroup<WorkerState> _workers;
  /// All pieces to process
  mt::SharedQueue<PieceTask> _tasks;

  /// Mutex protecting furrent state
  mutable std::shared_mutex _mtx;
  /// All torrent to manage, even those that have been stopped or have errors
  std::unordered_map<TorrentID, Torrent> _torrents;
  /// Incremental torrent descriptor next id
  TorrentID _descriptor_next_uid;

  /// Filepath of the folder containing all downloaded content
  std::string _download_folder;

 public:
  /// All possible Furrent errors
  enum class Error { GenericError, LoadingTorrentFailed };

  /// Errors for the furrent class
  template <typename R>
  using Result = util::Result<R, Error>;
  using Empty = util::Empty;

 public:
  Furrent();
  virtual ~Furrent();

  /// Set the download folder
  Result<Empty> set_download_folder(const std::string& folder);

  /// Begin download of a torrent
  /// @param filename filename of the .torrent file
  /// @return the id of the new torrent
  Result<TorrentID> add_torrent(const std::string& filename);

  /// Removes a torrent descriptor and all of his tasks
  /// @param uid uid of the torrent to remove
  void remove_torrent(TorrentID tid);

  /// Extract torrents stats
  std::optional<TorrentGuiData> get_gui_data(TorrentID tid) const;

  /*
  /// Pause the download of a torrent
  /// @param uid uid of the torrent to pause
  void pause_torrent(TorrentID uid);

  /// Resume the download of a torrent, wakeup all threads
  /// @param uid uid of the torrent to resume
  void resume_torrent(TorrentID uid);
  */

  /// Callbacks from the UI
 public:
  /*
  bool callback_torrent_insert(const std::string &fine_name, const std::string
  &file_path); bool callback_torrent_remove(const TorrentSnapshot &torrent);
  bool callback_torrent_update(const TorrentSnapshot &torrent);
  bool callback_setting_update(const std::string &path);
  */

 private:
  /// Main function of all workers
  void thread_main(mt::Runner runner, WorkerState& state, size_t index);

  /// Set torrent state to error and remove torrent
  void torrent_error(TorrentID tid);

  /// Prepare all folders and files for a torrent
  /// @return True if the operation was a success, false otherwise
  bool prepare_torrent_files(TorrentFile& descriptor);
};

}  // namespace fur

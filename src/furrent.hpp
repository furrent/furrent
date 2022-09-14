#pragma once

#include <download/downloader.hpp>
#include <download/lender_pool.hpp>
#include <mt/group.hpp>
#include <mt/sharing_queue.hpp>
#include <mt/task.hpp>
#include <shared_mutex>
#include <torrent.hpp>
#include <torrent_manager.hpp>

namespace fur {

struct TorrentDescriptor {
  // Protects internal state, allows multiple readers but only one writer
  std::shared_mutex mtx;

  /// Name of the file where the torrent can be found
  std::string filename;
  /// Parsed torrent file
  std::optional<torrent::TorrentFile> torrent;
  /// Peers' downloaders where to ask for the pieces
  std::vector<Peer> downloaders;

  // Time of first announce
  std::chrono::high_resolution_clock::time_point announce_time;
  /// Interval to next update
  size_t interval;
  /// Number of pieces downloaded
  std::atomic_uint32_t downloaded_pieces;
  std::atomic_bool to_refresh;

  explicit TorrentDescriptor(const std::string& filename);

  /// Regenerate list of peers
  bool regenerate_peers();
  /// True if there are no more pieces to download
  bool finished();
};

/// Main state of the program
class Furrent {
  typedef mt::SharingQueue<mt::ITask::Wrapper> TaskSharingQueue;

  /// State of the worker threads
  struct WorkerState {};

  /// List of torrents to download
  std::list<TorrentDescriptor> _descriptors;
  /// Global work queue
  TaskSharingQueue _global_queue;
  /// Local work queues array, this is a dynamic array
  /// becaus it is easier to manager allocation of noncopyable types
  /// and random access
  TaskSharingQueue* _local_queues;
  /// Pool managing worker threads
  mt::ThreadGroup<WorkerState> _workers;

 public:
  Furrent();
  virtual ~Furrent();

  /// Begin downloading a torrent
  void add_torrent(const std::string& filename);

  /// @return all loaded descriptors
  const std::list<TorrentDescriptor>& get_descriptors() const;

 private:
  /// Main function of all workers
  void thread_main(mt::Runner runner, WorkerState& state, size_t index);
};

}  // namespace fur
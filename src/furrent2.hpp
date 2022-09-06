#pragma once

#include <torrent_manager.hpp>
#include <torrent.hpp>
#include <mt/sharing_queue.hpp>
#include <mt/task.hpp>
#include <furrent.hpp>

namespace fur {

struct TorrentDescriptor {
    /// Name of the file where the torrent can be found
    std::string filename;
    /// Parsed torrent file
    std::optional<torrent::TorrentFile> torrent;
};

/// Load torrent from file and spawn all piece download tasks
class TorrentFileLoadTask : public mt::ITask {

    /// Reference to preallocated torrent descriptor in furrent
    TorrentDescriptor& _descriptor;

public:
    TorrentFileLoadTask(TorrentDescriptor& descriptor);
    void execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) override;
};

/// Download a piece of a torrent
class DownloadPieceTask : public mt::ITask {

    /// Reference to preallocated torrent descriptor in furrent
    TorrentDescriptor& _descriptor;

    size_t _index;
    size_t _offset;
    size_t _bytes;

public:
    DownloadPieceTask(TorrentDescriptor& descr, size_t index, size_t offset, size_t bytes);
    void execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) override;
};

/// Main state of the program  
class Furrent2 {

    typedef mt::SharingQueue<mt::ITask::Wrapper> TaskSharingQueue;

    /// State of the worker threads
    struct WorkerState { };

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

    Furrent2();
    virtual ~Furrent2();

    /// Begin downloading a torrent 
    void add_torrent(const std::string& filename);

    /// @return all loaded descriptors 
    const std::list<TorrentDescriptor>& get_descriptors() const;

private:
    /// Main function of all workers
    void thread_main(mt::Runner runner, WorkerState& state, size_t index);
};

} // namespace fur

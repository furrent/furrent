#pragma once

#include <mt/task.hpp>
#include <download/downloader.hpp>
#include <log/logger.hpp>

namespace fur {
    class TorrentHandle;
}

namespace fur::mt {

class TorrentTask : public ITask {
protected:
    /// Default logger for the tasks
    std::shared_ptr<spdlog::logger> logger;

public:
    /// Reference to preallocated torrent descriptor in furrent
    std::shared_ptr<TorrentHandle> descriptor;

public:
    TorrentTask(std::shared_ptr<TorrentHandle> descriptor);  
    TorrentTask(SharedQueue<Wrapper>* spawn_queue, std::shared_ptr<TorrentHandle> descriptor);
};

/// Load torrent from file and spawn all piece download tasks
class TorrentFileLoad : public TorrentTask {
public:
    TorrentFileLoad(std::shared_ptr<TorrentHandle> descriptor);
    TorrentFileLoad(SharedQueue<Wrapper>* spawn_queue, std::shared_ptr<TorrentHandle> descriptor);
    
    void execute() override;
    size_t priority() const override { return mt::Priority::PRIORITY_HIGH; }
};

/// Download a piece of a torrent
class TorrentPieceDownload : public TorrentTask {
public:
    /// Contains all information relative to the piece to download
    download::Piece piece;

public:
    TorrentPieceDownload(std::shared_ptr<TorrentHandle> descr, download::Piece piece);
    TorrentPieceDownload(SharedQueue<Wrapper>* spawn_queue, std::shared_ptr<TorrentHandle> descr, 
                         download::Piece piece);
    
    void execute() override;
    size_t priority() const override;
};

/// Refresh peers list of a torrent
class TorrentPeerRefresh : public TorrentTask {
public:
    TorrentPeerRefresh(std::shared_ptr<TorrentHandle> descr);
    TorrentPeerRefresh(SharedQueue<Wrapper>* spawn_queue, std::shared_ptr<TorrentHandle> descr);
    
    void execute() override;
    size_t priority() const override;
};

/// Policy for extracing highest priority tasks first
class PriorityPolicy : public policy::IPolicy<TorrentTask::Wrapper> {
 public:
  Iterator extract(Iterator begin, Iterator end) const override;
};

} // namespace fur::task

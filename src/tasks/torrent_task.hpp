#pragma once

#include <mt/task.hpp>
#include <furrent.hpp>
#include <download/downloader.hpp>
#include <log/logger.hpp>

namespace fur::tasks {

class TorrentTask : public mt::ITask {
protected:
    /// Default logger for the tasks
    std::shared_ptr<spdlog::logger> logger;

public:
    /// Reference to preallocated torrent descriptor in furrent
    const std::shared_ptr<TorrentHandle> descriptor;

public:
    /// Constructs a new torrent task
    /// @param descriptor shared pointer to torrent descriptor  
    explicit TorrentTask(std::shared_ptr<TorrentHandle> descriptor);
};

/// Load torrent from file and spawn all piece download tasks
class TorrentFileLoad : public TorrentTask {
public:
    explicit TorrentFileLoad(std::shared_ptr<TorrentHandle> descriptor);
    void execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) override;

    size_t priority() const override { return mt::Priority::PRIORITY_HIGH; }
};

/// Download a piece of a torrent
class TorrentPieceDownload : public TorrentTask {
public:
    /// Contains all information relative to the piece to download
    download::Piece piece;

public:
    TorrentPieceDownload(std::shared_ptr<TorrentHandle> descr, download::Piece piece);
    void execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) override;

    size_t priority() const override;
};

/// Refresh peers list of a torrent
class TorrentPeerRefresh : public TorrentTask {
public:
    explicit TorrentPeerRefresh(std::shared_ptr<TorrentHandle> descr);
    void execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) override;

    size_t priority() const override;
};

} // namespace fur::task

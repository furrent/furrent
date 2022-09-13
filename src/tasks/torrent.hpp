#pragma once

#include <mt/task.hpp>
#include <furrent.hpp>
#include <download/downloader.hpp>
#include <log/logger.hpp>

namespace fur::tasks {

/// Load torrent from file and spawn all piece download tasks
class TorrentFileLoad : public mt::ITask {

    /// Reference to preallocated torrent descriptor in furrent
    TorrentDescriptor& _descriptor;

public:
    explicit TorrentFileLoad(TorrentDescriptor& descriptor);
    void execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) override;

    size_t priority() const { return 2; }
};

/// Download a piece of a torrent
class TorrentPieceDownload : public mt::ITask {

    /// Reference to preallocated torrent descriptor in furrent
    TorrentDescriptor& _descriptor;
    /// Contains all information relative to the piece to download
    download::PieceDescriptor _piece;

public:
    TorrentPieceDownload(TorrentDescriptor& descr, download::PieceDescriptor piece);
    void execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) override;

    size_t priority() const { return 1; }
};

/// Refresh peers list of a torrent
class TorrentPeerRefresh : public mt::ITask {

    /// Reference to preallocated torrent descriptor in furrent
    TorrentDescriptor& _descriptor;

public:
    explicit TorrentPeerRefresh(TorrentDescriptor& descr);
    void execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) override;

    size_t priority() const override;
};

    
} // namespace fur::task

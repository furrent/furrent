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

    size_t priority() const override { return mt::PRIORITY_HIGH; }
};

/// Download a piece of a torrent
class TorrentPieceDownload : public mt::ITask {

    /// Reference to preallocated torrent descriptor in furrent
    TorrentDescriptor& _descriptor;
    /// Contains all information relative to the piece to download
    download::Piece _piece;

public:
    TorrentPieceDownload(TorrentDescriptor& descr, download::Piece piece);
    void execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) override;

    size_t priority() const override { return mt::PRIORITY_LOW; }
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

/// Split downloaded binary blob into the correct files described in the torrent file
class TorrentOutputSplitter : public mt::ITask {

    /// Reference to preallocated torrent descriptor in furrent
    TorrentDescriptor& _descriptor;

public:
    explicit TorrentOutputSplitter(TorrentDescriptor& descr);
    void execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) override;

    size_t priority() const override { return mt::PRIORITY_HIGH; }
};

} // namespace fur::task

#pragma once

#include "torrent_manager.hpp"
#include <mt/thread_pool.hpp>
#include <download/lender_pool.hpp>
#include <furrent.hpp>

namespace fur {

class Furrent {

    /// List of torrents to download
    std::shared_ptr<mt::VectorRouter<TorrentManager, Piece>> _downloads;
    mt::WorkerThreadPool<TorrentManager, Piece> _thread_pool;

  public:
    Furrent();
    ~Furrent() = default;
    /// Add torrent to the list of downloads creating a TorrentManager object
    /// for it
    void add_torrent(const std::string& path);
    /// Print the status of the downloads
    void print_status() const;
};

}
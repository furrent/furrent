#pragma once

#include "torrent_manager.hpp"
#include <mt/thread_pool.hpp>
#include <download/lender_pool.hpp>
#include <furrent.hpp>

namespace fur {

class Furrent {

    typedef mt::VectorRouter<TorrentManagerRef, Piece> MyVectorRouter;

    /// List of torrents to download
    std::list<std::shared_ptr<TorrentManager>> _downloads;
    /// Router used to orchestrate workers' work
    std::shared_ptr<MyVectorRouter> _router;
    /// Pool managing workers threads
    mt::WorkerThreadPool<TorrentManagerRef, Piece> _thread_pool;

  public:
    Furrent();
    /// Constructor for the tests
    Furrent(std::function<void(Piece&)>);
    ~Furrent() = default;
    /// Add torrent to the list of downloads creating a TorrentManager object
    /// for it
    void add_torrent(const std::string& path);
    /// Print the status of the downloads
    void print_status() const;
};

}
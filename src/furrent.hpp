#pragma once

#include "torrent_manager.hpp"
#include <mt/channel.hpp>
#include <mt/group.hpp>
#include <download/lender_pool.hpp>
#include <furrent.hpp>

namespace fur {

class Furrent {

    typedef std::unique_ptr<mt::IListStrategy<std::weak_ptr<TorrentManager>, Piece>> MyTorrentStrategy;

    /// Internal state/statistics of the workers 
    struct WorkerState { };

    /// List of torrents to download
    std::list<std::shared_ptr<TorrentManager>> _downloads;
    /// Channel used to transfer work to workers
    mt::StrategyChannel<std::weak_ptr<TorrentManager>, Piece> _work_channel;
    /// Pool managing worker threads
    mt::ThreadGroup<WorkerState> _workers;
    /// Strategy used to distribute torrents to threads as pieces
    MyTorrentStrategy _strategy;

  public:
    /// Real constructor of Furrent
    Furrent();
    /// Constructor for the tests
    Furrent(std::function<void(Piece&)>);
    
    ~Furrent();
    
    /// Add torrent to the list of downloads creating a TorrentManager object
    /// for it
    void add_torrent(const std::string& path);
    /// Print the status of the downloads
    void print_status() const;
};

}
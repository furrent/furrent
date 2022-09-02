#pragma once

#include "torrent_manager.hpp"
#include <mt/channel.hpp>
#include <mt/group.hpp>
#include <strategy/global.hpp>
#include <download/lender_pool.hpp>
#include <furrent.hpp>

namespace fur {

class Furrent {

    typedef std::unique_ptr<strategy::IGlobalStrategy> MyTorrentStrategy;

    /// Internal state/statistics of the workers 
    struct WorkerState {
      /// Count the total number of processed pieces
      int processed_pieces;
    };

    /// List of torrents to download
    std::list<TorrentManager> _downloads;
    /// Channel used to transfer work to workers
    mt::channel::StrategyChannel<TorrentManagerRef> _torrent_channel;
    /// Pool managing worker threads
    mt::ThreadGroup<WorkerState> _workers;
    /// Strategy used to distribute torrents to threads as pieces
    MyTorrentStrategy _strategy;

  public:
    /// Real constructor of Furrent
    Furrent();
    ~Furrent();
    
    /// Add torrent to the list of downloads creating a TorrentManager object
    /// for it
    void add_torrent(const std::string& path);
    /// Print the status of the downloads
    void print_status() const;

    int get_total_processed_pieces();
};

}
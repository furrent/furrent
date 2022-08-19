#pragma once

#include "torrent_manager.hpp"

namespace fur {

class Furrent {
  private:
    /// List of torrents to download
    std::vector<fur::manager::TorrentManager> _downloads;
    /// Index of the next last TorrentManager to pick (TODO)
    unsigned int _index;
  public:
    Furrent();
    ~Furrent() = default;
    /// Add torrent to the list of downloads creating a TorrentManager object
    /// for it
    void add_torrent(const std::string& path);
    /// Print the status of the downloads
    void print_status();
    /// Pick the next torrent to download, then inside the TorrentManager object
    /// pick the next task to be done
    manager::TorrentManager pick_torrent();

};

}
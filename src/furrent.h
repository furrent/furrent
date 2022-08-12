#pragma once

#include "torrent_manager.h"

namespace fur {

class Furrent {
  private:
    std::vector<fur::manager::TorrentManager> _downloads;
  public:
    Furrent();
    void add_torrent(const std::string& torrent_file);
    void print_status();

};

}
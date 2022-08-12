#pragma once

#include "torrent_manager.hpp"

namespace fur {

class Furrent {
  private:
    std::vector<fur::manager::TorrentManager> _downloads;
  public:
    Furrent();
    ~Furrent() = default;
    void add_torrent(const std::string& path);
    void print_status();

};

}
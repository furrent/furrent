#include "torrent_manager.hpp"

using namespace fur::manager;

TorrentManager::TorrentManager(fur::torrent::TorrentFile torrent)
    : _torrent(torrent),
      _priority(0){

}


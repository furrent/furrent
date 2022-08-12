#include "torrent_manager.h"

using namespace fur::manager;

TorrentManager::TorrentManager(unsigned int priority,
                               fur::torrent::TorrentFile torrent,
                               std::queue<Task> task,
                               std::queue<Result> result,
                               std::vector<fur::peer::Peer> peers)
    : _priority(priority),
      _torrent(torrent),
      _task(task),
      _result(result),
      _peers(peers)
{
}


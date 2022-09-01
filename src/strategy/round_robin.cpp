#include <strategy/global.hpp>

namespace fur::strategy {

template<>
std::unique_ptr<IGlobalStrategy> make_strategy_global<GlobalStrategyType::RoundRobin>() {
    return std::make_unique<RoundRobinStrategy>();
}

std::optional<Piece> RoundRobinStrategy::extract(std::list<TorrentManagerRef>& torrents) {
    return std::nullopt; // TODO
}

} // namespace fur::strategy

/*
    std::optional<Piece> result = std::nullopt;
    while (!torrents.empty()) {
      
      // Get first torrent available (Round Robin)
      TorrentManagerRef weak_torrent = torrents.front();
      torrents.pop_front();
    
      // If torrent was not removed from the downloads
      // select a new piece to download
      if (auto torrent = weak_torrent.lock()) {

        auto task = torrent->pick_task();
        if (task.has_value()) {
          // TODO add socket picker from LenderPool
          result = std::optional{ Piece{ *task }};
        }

        // If the torrent has other pieces to download add it to the torrents
        if (torrent->has_tasks())
          torrents.push_back(weak_torrent);
      }
    }

    return result;
*/
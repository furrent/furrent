#include <strategy/global.hpp>

namespace fur::strategy {

template<>
std::unique_ptr<IGlobalStrategy> make_strategy_global<GlobalStrategyType::RoundRobin>() {
    return std::make_unique<RoundRobinStrategy>();
}

std::optional<TorrentManagerWeakRef> RoundRobinStrategy::extract(std::list<TorrentManagerWeakRef>& torrents) {
    TorrentManagerWeakRef torrent_ref = torrents.front();
    torrents.pop_front();
    return { torrent_ref };
}

void RoundRobinStrategy::insert(TorrentManagerWeakRef torrent, std::list<TorrentManagerWeakRef>& torrents) {
    torrents.push_back(torrent);
}

} // namespace fur::strategy
#include <strategy/global.hpp>

namespace fur::strategy {

template<>
std::unique_ptr<IGlobalStrategy> make_strategy_global<GlobalStrategyType::SmallerFirst>() {
    return std::make_unique<SmallerFirstStrategy>();
}

std::optional<TorrentManagerWeakRef> SmallerFirstStrategy::extract(std::list<TorrentManagerWeakRef>& torrents) {
    return std::nullopt; // TODO
}

void SmallerFirstStrategy::insert(TorrentManagerWeakRef torrent, std::list<TorrentManagerWeakRef>& torrents) {
    torrents.push_front(torrent);
}

} // namespace fur::strategy

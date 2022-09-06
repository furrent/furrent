#include <strategy/global.hpp>

namespace fur::strategy {

template<>
std::unique_ptr<IGlobalStrategy> make_strategy_global<GlobalStrategyType::SmallerFirst>() {
    return std::make_unique<SmallerFirstStrategy>();
}

auto SmallerFirstStrategy::extract(std::list<TorrentManagerRef>& torrents) -> Result {
    if (torrents.empty()) return Result::ERROR(util::Error::StrategyEmpty);
    return Result::ERROR(util::Error::StrategyEmpty); // TODO
}

void SmallerFirstStrategy::insert(TorrentManagerRef torrent, std::list<TorrentManagerRef>& torrents) {
    torrents.push_front(torrent);
}

} // namespace fur::strategy

#include <strategy/global.hpp>

namespace fur::strategy {

template<>
std::unique_ptr<IGlobalStrategy> make_strategy_global<GlobalStrategyType::SmallerFirst>() {
    return std::make_unique<SmallerFirstStrategy>();
}

std::optional<Piece> SmallerFirstStrategy::extract(std::list<TorrentManagerRef>& torrents) {
    return std::nullopt; // TODO
}

} // namespace fur::strategy

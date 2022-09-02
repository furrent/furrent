#include <strategy/local.hpp>

#include <torrent_manager.hpp>

namespace fur::strategy {

template<>
std::unique_ptr<ILocalStrategy> make_strategy_local<LocalStrategyType::Streaming>(TorrentManager& torrent) {
    return std::make_unique<StreamingStrategy>(torrent);
}

std::optional<PieceDescriptor> StreamingStrategy::extract(std::list<PieceDescriptor>& descriptors) {
    return std::nullopt; // TODO
}

} // namespace fur::strategy

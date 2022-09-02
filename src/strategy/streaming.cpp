#include <strategy/local.hpp>

#include <torrent_manager.hpp>

namespace fur::strategy {

template<>
std::unique_ptr<ILocalStrategy> make_strategy_local<LocalStrategyType::Streaming>(TorrentManager& torrent) {
    return std::make_unique<StreamingStrategy>(torrent);
}

auto StreamingStrategy::extract(std::list<PieceDescriptor>& descriptors) -> Result {
    if (descriptors.empty()) return Result::error(StrategyError::Empty);

    PieceDescriptor descriptor = descriptors.front();
    descriptors.pop_front();
    return Result::ok(descriptor);
}

} // namespace fur::strategy

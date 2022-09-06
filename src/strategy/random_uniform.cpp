#include <strategy/local.hpp>

#include <torrent_manager.hpp>

namespace fur::strategy {

template<>
std::unique_ptr<ILocalStrategy> make_strategy_local<LocalStrategyType::RandomUniform>(TorrentManager& torrent) {
    return std::make_unique<RandomUniformStrategy>(torrent);
}

auto RandomUniformStrategy::extract(std::list<PieceDescriptor>& descriptors) -> Result {
    std::uniform_int_distribution<> distr(0, std::distance(descriptors.begin(), descriptors.end()) - 1);
    auto begin = descriptors.begin();
    std::advance(begin, distr(_rng));
    return Result::OK(std::move(*begin));
}

} // namespace fur::strategy

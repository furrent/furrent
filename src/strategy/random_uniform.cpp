#include <strategy/local.hpp>

#include <torrent_manager.hpp>

namespace fur::strategy {

template<>
std::unique_ptr<ILocalStrategy> make_strategy_local<LocalStrategyType::RandomUniform>(TorrentManager& torrent) {
    return std::make_unique<RandomUniformStrategy>(torrent);
}

std::optional<Piece> RandomUniformStrategy::extract(std::list<Task>& torrents) {
    std::uniform_int_distribution<> distr(0, std::distance(torrents.begin(), torrents.end()) - 1);
    
    auto result = torrents.begin();
    std::advance(result, distr(_rng));

    //Task task = *result;
    //auto socket = _torrent.get_lender_pool().get();
    return std::nullopt; // TODO
}

} // namespace fur::strategy

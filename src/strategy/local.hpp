/**
 * @file local.hpp
 * @author Filippo Ziche
 * @version 0.1
 * @brief Contains all types used to create local strategies and the default ones
 * @date 2022-09-01
 */

#pragma once

#include <memory>
#include <random>

#include <strategy/strategy.hpp>

namespace fur {

    // Forward declaration

    class TorrentManager;
    class Task; 
    class Piece; 
}

namespace fur::strategy {

/// Local strategies are used to choose pieces
using ILocalStrategy = IListStrategy<Task, Piece>;

/// All hardcoded strategies used to distribute torrents pieces to workers
enum class LocalStrategyType {
    /// Pieces are processed in order, allows for streaming
    Streaming,
    /// Pieces are processed at random
    RandomUniform
}; 

/// Factory method used to generate hardcoded strategies
template<LocalStrategyType LST>
std::unique_ptr<ILocalStrategy> make_strategy_local(TorrentManager&);

/// Local strategy need to access the torrent manager
class LocalStrategy : public ILocalStrategy {
protected:
    TorrentManager& _torrent;
public:
    LocalStrategy(TorrentManager& torrent)
        : _torrent{torrent} { }
};

// Strategy to pick a task from a list of TorrentManager, every TorrentManager
// has a local strategy to pick a task from its own list of tasks.
class StreamingStrategy : public LocalStrategy {
public:
    StreamingStrategy(TorrentManager& torrent)
        : LocalStrategy(torrent) { };

    std::optional<Piece> extract(std::list<Task>& torrents) override;
};

// Strategy to pick a task from a list of TorrentManager, every TorrentManager
// has a local strategy to pick a task from its own
class RandomUniformStrategy : public LocalStrategy {

    std::random_device _rd;
    std::mt19937 _rng;

public:
    RandomUniformStrategy(TorrentManager& torrent)
        : LocalStrategy(torrent) { };

    std::optional<Piece> extract(std::list<Task>& torrents) override;
};

}
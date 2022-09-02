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
    class PieceDescriptor; 
    class PieceDownloader; 
}

namespace fur::strategy {

/// Local strategies are used to choose pieces
using ILocalStrategy = IListStrategy<PieceDescriptor>;

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
    TorrentManager& _torrent;
public:
    LocalStrategy(TorrentManager& torrent)
        : _torrent{torrent} { }

    /// Usually it is enough to insert at the end
    void insert(PieceDescriptor, std::list<PieceDescriptor>&) override;
};

// Strategy to pick a task from a list of TorrentManager, every TorrentManager
// has a local strategy to pick a task from its own list of tasks.
class StreamingStrategy : public LocalStrategy {
public:
    StreamingStrategy(TorrentManager& torrent)
        : LocalStrategy(torrent) { };

    Result extract(std::list<PieceDescriptor>&) override;
};

// Strategy to pick a task from a list of TorrentManager, every TorrentManager
// has a local strategy to pick a task from its own 
class RandomUniformStrategy : public LocalStrategy {

    std::random_device _rd;
    std::mt19937 _rng;

public:
    RandomUniformStrategy(TorrentManager& torrent)
        : LocalStrategy(torrent) { };

    Result extract(std::list<PieceDescriptor>&) override;
};

}
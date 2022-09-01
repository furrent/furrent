/**
 * @file global.hpp
 * @author Filippo Ziche
 * @version 0.1
 * @brief Contains all types used to create global strategies and the default ones
 * @date 2022-09-01
 */

#pragma once

#include <memory>

#include <strategy/strategy.hpp>
#include <torrent_manager.hpp>

namespace fur::strategy {

/// Global strategies are used to distribute torrent pieces to workers
using IGlobalStrategy = IListStrategy<TorrentManagerRef, Piece>;

/// All hardcoded strategies used to distribute torrents pieces to workers
enum class GlobalStrategyType {
    /// Torrents are processed in order and then reinserted at the end of the queue
    RoundRobin,
    /// Torrent with the least number of pieces is processed first
    SmallerFirst
}; 

/// Factory method used to generate hardcoded strategies
template<GlobalStrategyType GST>
std::unique_ptr<IGlobalStrategy> make_strategy_global();

// Strategy to pick a task from a list of TorrentManager, every TorrentManager
// has a local strategy to pick a task from its own list of tasks.
class RoundRobinStrategy : public IGlobalStrategy {
public:
    std::optional<Piece> extract(std::list<TorrentManagerRef>& torrents) override;
};

// Strategy to pick a task from a list of TorrentManager, every TorrentManager
// has a local strategy to pick a task from its own
class SmallerFirstStrategy : public IGlobalStrategy {
public:
    std::optional<Piece> extract(std::list<TorrentManagerRef>& torrents) override;
};

}
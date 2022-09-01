/**
 * @file strategy.hpp
 * @author Filippo Ziche
 * @version 0.1
 * @brief Contains types shared between al strategies
 * @date 2022-09-01
 */

#pragma once

#include <list>
#include <optional>

namespace fur::strategy {

/// Allows the extraction of an element from a collection using
/// a user-defined logic
/// @tparam Served Type of the result of the extraction
/// @tparam Container Type of the collection containing items
template<typename Served, typename Container>
class IStrategy {
public:
    /// Implements custom extract logic
    /// Extraction can fail, in that case returns nullopt
    virtual std::optional<Served> extract(Container&) = 0;
};

/// Comodity type for strategy used for working on a list
template<typename Stored, typename Served>
using IListStrategy = IStrategy<Served, typename std::list<Stored>>;

/// Comodity type for strategy that don't transform data from Stored to Server
template<typename Served>
using IAutoListStrategy = IListStrategy<Served, Served>;

} // namespace fur::strategy
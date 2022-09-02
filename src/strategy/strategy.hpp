/**
 * @file strategy.hpp
 * @author Filippo Ziche
 * @version 0.1
 * @brief Contains types shared between al strategies. 
 *        Strategies are transformers used to extract work from
 *        StategyQueue in a thread-safe manner.
 * @date 2022-09-01
 */

#pragma once

#include <list>
#include <optional>

namespace fur::strategy {

/// Allows the extraction of an element from a collection using
/// a user-defined logic in a thread-safe manner
/// @tparam Served Type of the result of the extraction
/// @tparam Container Type of the collection containing items
template<typename T, typename Container>
class IStrategy {
public:
    /// Implements custom extract logic
    /// Extraction can fail, in that case returns nullopt
    virtual std::optional<T> extract(Container&) = 0;

    /// Implements custom insert logic
    virtual void insert(T item, Container&) = 0;
};

/// Comodity type for strategy used for working on a list
template<typename T>
using IListStrategy = IStrategy<T, typename std::list<T>>;


} // namespace fur::strategy
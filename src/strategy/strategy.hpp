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

#include <util/result.hpp>

namespace fur::strategy {

/// Possible errors that can occour when using a strategy 
enum class StrategyError {
    /// Occurs whene there was no element to extract
    Empty
};

/// Allows the extraction of an element from a collection using
/// a user-defined logic in a thread-safe manner
/// @tparam Served Type of the result of the extraction
/// @tparam Container Type of the collection containing items
template<typename T, typename Container>
class IStrategy {
public:

    /// Error type for this type
    typedef util::Result<T, StrategyError> Result;

    /// Implements custom extract logic
    /// Extraction can fail, in that case returns nullopt
    virtual Result extract(Container&) = 0;

    /// Implements custom insert logic
    virtual void insert(T item, Container&) = 0;
};

/// Comodity type for strategy used for working on a list
template<typename T>
using IListStrategy = IStrategy<T, typename std::list<T>>;


} // namespace fur::strategy
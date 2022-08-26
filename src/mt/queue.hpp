/**
 * @file queue.hpp
 * @author Filippo Ziche
 * @version 0.1
 * @date 2022-08-26
 */

#pragma once

#include <mutex>
#include <condition_variable>
#include <optional>
#include <list>

namespace fur::mt {

/// Allows the extraction of an element from a collection using
/// a user-defined logic
/// @tparam Served Type of the result of the extraction
/// @tparam Container Type of the collection containing items
template<typename Served, typename Container>
class IStrategy {
public:
    /// Implements custom extract logic
    virtual Served extract(Container&) = 0;
};

/// Comodity type for strategy used for working on a list
template<typename Stored, typename Served>
using IListStrategy = IStrategy<Served, typename std::list<Stored>>;

/// Data structure used to transfer work to threads without
/// racing conditions. Similar to a MPMC queue, but uses custom strategies
/// to choose the work-item to serve.
template<typename Stored, typename Served>
class StrategyQueue {

    /// Compatible strategy type
    typedef IListStrategy<Stored, Served> Strategy;

private:
    /// Mutex protecting collection and CVs
    std::mutex _work_mutex;
    /// Used to notify that a new item is available 
    std::condition_variable _work_available;
    /// Used to notify that at the moment there are no more items available 
    std::condition_variable _work_finished;
    /// List storing all work item to be served
    std::list<Stored> _work;
    /// True if work should be served
    bool _serving;

public:
    /// Construct a new empty queue
    StrategyQueue();
    /// Frees all waiting threads
    virtual ~StrategyQueue() = default;

    //===========================================================================
    // This object is not copyable and not movable because of mutex and CVs

    StrategyQueue(StrategyQueue&) = delete;
    StrategyQueue& operator= (StrategyQueue&) = delete;
    StrategyQueue(StrategyQueue&&) noexcept = delete;
    StrategyQueue& operator= (StrategyQueue&&) noexcept = delete;

    //===========================================================================

    /// Changes serving variable and notifies threads, used to return
    /// execution to the outside
    void set_serving(bool value);

    /// Inserts new item inside collection
    void insert(Stored item);
    /// Extracts an element from the collection using a strategy.
    /// If the collection is empty then waits for a new item.
    /// If the result is nullptr then we stopped serving
    std::optional<Served> extract(Strategy* strategy);

    /// Blocks execution, without spinlock, until there is no more work to do
    void wait_empty();

    /// Returns a reference to the internal list, this function is not thread-safe!
    const std::list<Stored>& get_work_list() const;
};

} // namespace fur::mt

#include <mt/queue.inl>
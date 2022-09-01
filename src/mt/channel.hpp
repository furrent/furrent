/**
 * @file channel.hpp
 * @author Filippo Ziche
 * @version 0.1
 * @date 2022-08-26
 */

#pragma once

#include <mutex>
#include <condition_variable>
#include <optional>
#include <list>

#include <strategy/strategy.hpp>

namespace fur::mt {

/// Data structure used to transfer work to threads without
/// racing conditions. Similar to a MPSC queue, but uses custom strategies
/// to choose the work-item to serve.
template<typename Stored, typename Served>
class StrategyChannel {

    // Served type must be copyable
    static_assert(
        std::is_copy_constructible<Served>::value,
        "In StrategyQueue the Served type must be copyable!"
    );

    /// Compatible strategy type
    typedef strategy::IListStrategy<Stored, Served> MyStrategy;

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
    StrategyChannel();
    /// Frees all waiting threads
    virtual ~StrategyChannel();

    //===========================================================================
    // This object is not copyable and not movable because of mutex and CVs

    StrategyChannel(StrategyChannel&) = delete;
    StrategyChannel& operator= (StrategyChannel&) = delete;
    StrategyChannel(StrategyChannel&&) noexcept = delete;
    StrategyChannel& operator= (StrategyChannel&&) noexcept = delete;

    //===========================================================================

    /// Changes serving variable and notifies threads, used to return
    /// execution to the outside
    void set_serving(bool value);

    /// Inserts new item inside collection
    /// @param item item to be inserted
    void insert(Stored item);

    /// Extracts an element from the collection using a strategy.
    /// If the collection is empty then waits for a new item.
    /// If the result is nullopt then we stopped serving during waiting
    /// @param stategy strategy implementation to be used
    std::optional<Served> extract(MyStrategy* strategy);

    /// Extracts an element from the collection using a strategy.
    /// If the collection is empty or we stopped serving then 
    /// returns a nullopt
    /// @param stategy strategy implementation to be used
    std::optional<Served> try_extract(MyStrategy* strategy);

    /// Generic function used to mutate the internal collection
    /// in a thread-safe matter
    /// @param mutation function used to mutate the collection, returns
    ///                 true if waiting threads should be wakeuped
    void mutate(std::function<bool(std::list<Stored>&)> mutation);

    /// Blocks execution, without spinlock, until there is no more work to do
    void wait_empty();

    /// Returns a reference to the internal list, this function is not thread-safe!
    const std::list<Stored>& get_work_list() const;
};

} // namespace fur::mt

#include <mt/channel.inl>
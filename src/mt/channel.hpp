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

#include <util/result.hpp>
#include <strategy/strategy.hpp>

namespace fur::mt::channel {

/// Possible error types
enum class StrategyChannelError 
{
    /// Occurs when worker was waiting for an item but the Channel
    /// stopped serving
    StoppedServing,
    /// Occurs when the strategy used returned nothing
    StrategyFailed,
    /// Occurs when we try to extract an item but the Channel was empty
    Empty
};

/// Data structure used to transfer work to threads without
/// racing conditions. Similar to a MPSC queue, but uses custom strategies
/// to choose the work-item to serve.
template<typename T>
class StrategyChannel {

    // Served type must be copyable
    static_assert(
        std::is_copy_constructible<T>::value,
        "In StrategyQueue the Served type must be copyable!"
    );

public:

    /// Compatible strategy type
    typedef strategy::IListStrategy<T> MyStrategy;
    /// Error type used
    typedef util::Result<T, StrategyChannelError> MyResult;

private:
    /// Mutex protecting collection and CVs
    std::mutex _work_mutex;
    /// Used to notify that a new item is available 
    std::condition_variable _work_available;
    /// Used to notify that at the moment there are no more items available 
    std::condition_variable _work_finished;
    /// List storing all work item to be served
    std::list<T> _work;
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

    /// Inserts new item inside collection using the strategy
    /// @param item item to be inserted
    /// @param strategy strategy implementation to be used
    void insert(T item, MyStrategy* strategy);

    /// Extracts an element from the collection using a strategy.
    /// If the collection is empty then waits for a new item.
    /// If the result is nullopt then we stopped serving during waiting
    /// @param stategy strategy implementation to be used
    [[nodiscard]] MyResult extract(MyStrategy* strategy);

    /// Extracts an element from the collection using a strategy.
    /// If the collection is empty or we stopped serving then 
    /// returns a nullopt
    /// @param stategy strategy implementation to be used
    [[nodiscard]] MyResult try_extract(MyStrategy* strategy);

    /// Generic function used to mutate the internal collection
    /// in a thread-safe matter
    /// @param mutation function used to mutate the collection, returns
    ///                 true if waiting threads should be wakeuped
    void mutate(std::function<bool(std::list<T>&)> mutation);

    /// Blocks execution, without spinlock, until there is no more work to do
    void wait_empty();

    /// Returns a reference to the internal list, this function is not thread-safe!
    const std::list<T>& get_work_list() const;
};

} // namespace fur::mt::channel

#include <mt/channel.inl>
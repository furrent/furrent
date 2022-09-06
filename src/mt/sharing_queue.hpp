/**
 * @file mtts.hpp
 * @author Filippo Ziche (filippo.ziche@gmail.com)
 * @brief Sharing queues allow the propagation of Tasks in order to
 *        maintain high utilization of threads
 * @version 0.1
 * @date 2022-09-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <mutex>
#include <condition_variable>
#include <optional>

#include <policy/queue.hpp>

namespace fur::mt {

template<typename Work>
class SharingQueue {

    /// Contains all work that can be executed
    policy::Queue<Work> _work;
    /// Protects internal state and CVs
    mutable std::mutex _mutex;
    /// Signal the insertion of a new element
    mutable std::condition_variable _new_work_available;
    /// Signal the dispatch of all available work
    mutable std::condition_variable _all_work_dispatched;
    /// True if waiting threads should be woken up
    bool _skip_waiting;

public:

    /// Error are the same of the queue
    typedef typename policy::Queue<Work>::Error Error;
    typedef util::Result<Work> Result;

public:
    SharingQueue();

    /// Tries to extract work from the queue
    /// @param policy policy used to extract the element
    [[nodiscard]] Result try_extract(const policy::IPolicy<Work>& policy);

    /// Insert new work in the internal list
    /// @param work Work to be inserted 
    void insert(Work&& work);

    /// Steal work from the queue
    [[nodiscard]] Result steal();

    /// Wait for a new item in the queue
    void wait_for_work() const;

    /// Wake up all waiting threads
    void begin_skip_waiting();
};

} // namespace fur::mt

#include <mt/sharing_queue.inl>
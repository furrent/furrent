/**
 * @file sharind_queue.hpp
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

#include <condition_variable>
#include <mutex>
#include <optional>
#include <functional>
#include <policy/queue.hpp>

namespace fur::mt {

template <typename Work>
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
  typedef util::Result<Work, Error> Result;

 public:
  SharingQueue();

  /// Tries to extract work from the queue
  /// @param policy policy used to extract the element
  [[nodiscard]] Result try_extract(const policy::IPolicy<Work>& policy);

  /// Insert new work in the internal list
  /// @param work Work to be inserted
  void insert(Work&& work);

  /// Removes all elements satisying the filter
  /// @param filter function used to remove elements, returns true 
  /// if the elements should be removed, false otherwise
  void remove(std::function<bool(Work&)> filter);

  /// Steal work from the queue
  [[nodiscard]] Result steal();

  /// Wake up all waiting threads
  void force_wakeup();

  /// Wait for a new item in the queue
  void wait_work() const;

  /// Wait for the queue to be empty
  void wait_empty() const;

  /// Wake up all waiting threads and begins
  /// skipping work
  void begin_skip_waiting();
};

}  // namespace fur::mt

#include <mt/sharing_queue.inl>
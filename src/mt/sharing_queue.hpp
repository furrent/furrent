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

/// Wrapper of policy::Queue to make it thread-safe
/// and add concurrency related functionalities
template <typename T>
class SharedQueue {

  /// Contains all work that can be executed
  policy::Queue<T> _work;

  /// Protects internal state and CVs
  mutable std::mutex _mutex;
  /// Signal the insertion of a new element
  mutable std::condition_variable _new_work_available;
  /// Signal the dispatch of all available work
  mutable std::condition_variable _all_work_dispatched;
  /// True if waiting threads should be woken up
  bool _skip_waiting;

 public:

  // Error are the same of the queue
  typedef typename policy::Queue<T>::Error Error;
  typedef util::Result<T, Error> Result;

  // Function used to mutate the internal collection
  typedef typename policy::Queue<T>::MutateFn MutateFn;

 public:
  SharedQueue();

  /// Tries to extract work from the queue
  /// @param policy policy used to extract the element
  [[nodiscard]] Result try_extract(const policy::IPolicy<T>& policy);

  /// Insert new work in the internal list
  /// @param work Work to be inserted
  void insert(T&& work);

  /// Mutates the internal list of items in a locked way
  /// then signals all sleeping threads
  void mutate(MutateFn mutation);

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
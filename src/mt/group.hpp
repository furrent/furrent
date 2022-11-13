/**
 * @file group.hpp
 * @author Filippo Ziche
 * @version 0.1
 * @date 2022-08-26
 */

#pragma once

#include <functional>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

namespace fur::mt {

/// Used to control threads execution
class Runner {
  /// Protects terminate status
  std::mutex& _mutex;
  /// True if the threads need to be joined
  bool& _should_terminate;

 public:
  Runner(std::mutex& mutex, bool& _should_terminate);

  /// True if threads should continue executing
  bool alive();
};

/// Manages the execution of a group of threads
template <typename State>
class ThreadGroup {
  /// Functions executed by the threads, as an argument
  /// each threads receives the runner, its own unique state ad its thread
  /// number
  using ThreadFn = std::function<void(Runner, State&, size_t)>;

  /// Contains all threads handles managed by the group
  std::vector<std::thread> _threads;
  /// Contains all threads states
  std::vector<State> _states;
  /// Function running on workers
  ThreadFn _thread_fn;
  /// Protects terminate status
  std::mutex _mutex;
  /// True if the threads need to be joined
  bool _should_terminate;

 public:
  /// Create disabled thread group
  ThreadGroup();
  /// Stops and joins all threads
  virtual ~ThreadGroup();

  //===========================================================================
  // This object is not copyable and not movable because of the mutex

  ThreadGroup(ThreadGroup&) = delete;
  ThreadGroup& operator=(ThreadGroup&) = delete;
  ThreadGroup(ThreadGroup&&) noexcept = delete;
  ThreadGroup& operator=(ThreadGroup&&) noexcept = delete;

  //===========================================================================

  /// Create threads and begin execution
  void launch(ThreadFn fn, int64_t max_worker_threads = 0);

  /// Terminate thread execution, this operation is irrecuperable
  void terminate();

  /// Obtain threads state
  std::vector<State>& get_states();

  /// Get total number of workers
  [[nodiscard]] int64_t get_worker_count() const;

 private:
  /// Workers thread main function
  void thread_main(int64_t index);
};

}  // namespace fur::mt

#include <mt/group.inl>
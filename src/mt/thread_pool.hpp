//
// Created by Z1ko on 28/07/2022.
//

#ifndef FURRENT_THREAD_POOL_HPP
#define FURRENT_THREAD_POOL_HPP

#include <condition_variable>
#include <thread>
#include <vector>
#include <functional>
#include <atomic>

namespace fur::mt {

/// Allows workers to find work to do using different strategies.
/// Only one thread is allowed to use it at any same time
template<typename T>
class router {
 public:
  virtual ~router() = default;
  virtual bool work_is_available() = 0;
  virtual T get_work() = 0;
};

template<typename T>
class worker_thread_pool {

  /// Vector containing all workers threads managed by the pool
  std::vector<std::thread> m_threads;
  /// Function executing in all workers
  std::function<void(T&)> m_worker_fn;
  /// Router used to find work for the workers.
  /// Only one thread is allowed to use it at any time.
  std::unique_ptr<router<T>> m_router;

  /// True if the workers have been signaled to shutdown
  bool m_should_terminate;

  /// CV used to wakeup threads if they are idle
  std::condition_variable m_work_available;
  /// Mutex protecting condition variable and router
  std::mutex m_router_mutex;

 public:

  /// @brief Construct a new pool of worker threads
  /// @param max_worker_threads maximum number of worker threads
  /// @param router used to find and balance work for the workers
  worker_thread_pool(std::unique_ptr<router<T>> router,
                     std::function<void(T&)> worker_fn,
                     size_t max_worker_threads = 0);

  /// Shutdown workers threads pool by notifying and joining them
  ~worker_thread_pool();

  //
  // This object should be non-copyable and non-movable
  //
  worker_thread_pool(const worker_thread_pool&) = delete;
  worker_thread_pool& operator=(const worker_thread_pool&) = delete;
  worker_thread_pool(worker_thread_pool&&) noexcept = delete;
  worker_thread_pool& operator=(worker_thread_pool&&) noexcept = delete;

  /// Returns True if the router still has work to distribute
  bool busy();

  /// Change workers' router
  void change_router(std::unique_ptr<router<T>> router);

 public:

  /// Get number of workers
  size_t get_workers_count() { return m_threads.size(); }

 private:

  /// Workers' main function, used to control their behaviour
  /// outside of user defined code
  void worker();

};

} // namespace fur::mt

#include <mt/thread_pool.inl>

#endif  // FURRENT_THREAD_POOL_HPP

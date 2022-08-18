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

template<typename T>
class DataRouter;

template<typename T>
class WorkerThreadPool {

  /// Vector containing all workers threads managed by the pool
  std::vector<std::thread> m_threads;
  /// Function executing in all workers
  std::function<void(T&)> m_worker_fn;
  /// Router used to find work for the workers.
  /// Only one thread is allowed to use it at any time.
  std::unique_ptr<DataRouter<T>> m_router;

  /// True if the workers have been signaled to shutdown
  bool m_should_terminate;

  /// CV used to wakeup threads if they are idle
  std::condition_variable m_work_available;
  /// Mutex protecting condition variables and router
  std::mutex m_mutex;

 public:

  /// @brief Construct a new pool of worker threads
  /// @param max_worker_threads maximum number of worker threads
  /// @param router used to find and balance work for the workers
  WorkerThreadPool(std::unique_ptr<DataRouter<T>> router,
                   std::function<void(T&)> worker_fn,
                   size_t max_worker_threads = 0);

  /// Shutdown workers threads pool by notifying and joining them
  ~WorkerThreadPool();

  //
  // This object should be non-copyable and non-movable
  //
  WorkerThreadPool(const WorkerThreadPool&) = delete;
  WorkerThreadPool& operator=(const WorkerThreadPool&) = delete;
  WorkerThreadPool(WorkerThreadPool&&) noexcept = delete;
  WorkerThreadPool& operator=(WorkerThreadPool&&) noexcept = delete;

  /// 

  /// Returns True if the router still has work to distribute in this exact moment
  /// TODO: Implement a state-per-thread structure to monitor workers real status
  bool busy();

  /// Change workers' data router
  void change_router(std::unique_ptr<DataRouter<T>> router);

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

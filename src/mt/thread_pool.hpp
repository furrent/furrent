//
// Created by Z1ko on 28/07/2022.
//

#pragma once

#include <thread>
#include <vector>
#include <functional>
#include <mutex>

#include <mt/router.hpp>

namespace fur::mt {

template<typename From, typename To>
class WorkerThreadPool {

  /// Vector containing all workers threads managed by the pool
  std::vector<std::thread> m_threads;
  /// Function executing in all workers
  std::function<void(To&)> m_worker_fn;
  /// Router used to distribute tasks
  std::shared_ptr<IVectorRouter<From, To>> m_task_router;

  /// Mutex protecting terminate status
  std::mutex m_mutex;
  /// True if threads need to be joined
  bool m_should_terminate;

 public:

  /// @brief Construct a new pool of worker threads
  /// @param max_worker_threads maximum number of worker threads
  /// @param router used to find and balance work for the workers
  /// @param worker_fn function of the worker applied to the router data
  WorkerThreadPool(std::shared_ptr<IVectorRouter<From, To>> router,
                   std::function<void(To&)> worker_fn,
                   size_t max_worker_threads = 0);

  /// Shutdown workers threads pool by notifying and joining them
  virtual ~WorkerThreadPool();

  //
  // This object should be non-copyable and non-movable
  //
  
  WorkerThreadPool(const WorkerThreadPool&) = delete;
  WorkerThreadPool& operator=(const WorkerThreadPool&) = delete;
  WorkerThreadPool(WorkerThreadPool&&) noexcept = delete;
  WorkerThreadPool& operator=(WorkerThreadPool&&) noexcept = delete;

 public:
  /// Get number of workers
  size_t get_workers_count() { return m_threads.size(); }

  /// Waits until there is no more work to do, this doesn't guarantee
  /// that the threads have finished working
  void busy();

 private:
  /// Workers' main function, used to control their behaviour
  /// outside of user defined code
  void worker();

};

} // namespace fur::mt

#include <mt/thread_pool.inl>
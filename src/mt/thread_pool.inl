//
// Created by Z1ko on 28/07/2022.
//

#include <mt/thread_pool.hpp>
#include <mt/router.hpp>

#include <iostream>
#include <functional>
#include <utility>

namespace fur::mt {

template<typename T, typename W>
void WorkerThreadPool<T, W>::worker() {

  m_mutex.lock();
  while (!m_should_terminate) {
    m_mutex.unlock();

    std::optional<W> task_opt = m_task_router->get_work();
    if (task_opt.has_value())
      m_worker_fn(task_opt.value());

    m_mutex.lock();
  }
  m_mutex.unlock();
}

template<typename T, typename W>
WorkerThreadPool<T, W>::WorkerThreadPool(IVectorRouter<T, W>* router, std::function<void(W&)> worker_fn, size_t max_worker_threads)
  : m_worker_fn{std::move(worker_fn)}, m_task_router(router), m_should_terminate{false} {

  size_t size = std::thread::hardware_concurrency();
  if (max_worker_threads != 0) size = std::min(size, max_worker_threads);

  for (int i = 0; i < (int)size; i++)
    m_threads.emplace_back(
        std::bind(&WorkerThreadPool::worker, this), i, std::ref(*this));

  // Resume serving workers if router was stopped
  // TODO: This is a side-effect, not so pretty...
  m_task_router->resume();
}

template<typename T, typename W>
WorkerThreadPool<T, W>::~WorkerThreadPool() {
  {
    // Changes terminate status
    std::scoped_lock<std::mutex> lock(m_mutex);
    //std::cout << "WorkerThreadPool is requesting workers to shutdown\n";
    m_should_terminate = true;
  }

  // Ask the router to stop serving the workers
  m_task_router->stop();

  for (auto& thread : m_threads)
    if (thread.joinable()) thread.join();

  //std::cout << "WorkerThreadPool has joined all threads\n";
}

template<typename T, typename W>
void WorkerThreadPool<T, W>::busy() {
  m_task_router->busy();
}

} // namespace fur::mt
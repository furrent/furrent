//
// Created by Z1ko on 28/07/2022.
//

#include <mt/thread_pool.hpp>

#include <iostream>
#include <functional>
#include <utility>

namespace fur::mt {

template<typename T>
void worker_thread_pool<T>::worker() {

  bool work_todo = false;
  T work;

  while (true) {
    {
      // Waits for an available torrent or a shutdown event
      std::unique_lock<std::mutex> lock(m_router_mutex);
      m_work_available.wait(lock, [&] {
        return m_should_terminate || m_router->work_is_available();
      });

      if (m_should_terminate)
        return;

      // work is an independent piece of data
      if (m_router->work_is_available()) {
        work = m_router->get_work();
        work_todo = true;
      }
    }

    if (work_todo) {
      m_worker_fn(work);
      work_todo = false;
    }
  }
}
template <typename T>
bool worker_thread_pool<T>::busy() {
  std::lock_guard<std::mutex> lk(m_router_mutex);
  return m_router->work_is_available();
}

template<typename T>
worker_thread_pool<T>::worker_thread_pool(
    std::unique_ptr<router<T>> router,
    std::function<void(T&)> worker_fn,
    size_t max_worker_threads)
    : m_worker_fn{std::move(worker_fn)},
      m_router{std::move(router)},
      m_should_terminate{false}
{
  size_t size = std::thread::hardware_concurrency();
  if (max_worker_threads != 0) size = std::min(size, max_worker_threads);

  for (int i = 0; i < (int)size; i++)
    m_threads.emplace_back(
        std::bind(&worker_thread_pool::worker, this), i, std::ref(*this));
}

template<typename T>
worker_thread_pool<T>::~worker_thread_pool() {
  {
    std::lock_guard<std::mutex> lk(m_router_mutex);
    m_should_terminate = true;
  }
  // Send shutdown request to all workers threads
  m_work_available.notify_all();

  for (auto& thread : m_threads)
    if (thread.joinable()) thread.join();
}

template <typename T>
void worker_thread_pool<T>::change_router(std::unique_ptr<router<T>> router) {
  std::lock_guard<std::mutex> lk(m_router_mutex);
  m_router = std::move(router);
}

} // namespace fur::mt
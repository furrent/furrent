
#include <mt/router.hpp>

#include <memory>
#include <optional>
#include <iostream>

namespace fur::mt {

template<typename T, typename W>
VectorRouter<T, W>::VectorRouter(IVectorRouterStrategy<T, W>* strategy)
: m_should_serve{true}, m_strategy{strategy} { }

template<typename T, typename W>
void VectorRouter<T, W>::insert(T&& item) {
  {
    std::scoped_lock<std::mutex> lock(m_mutex);
    m_work_items.push_back(item);
  }
  m_work_available.notify_one();
}

template<typename T, typename W>
std::optional<W> VectorRouter<T, W>::get_work() {
    
  std::unique_lock<std::mutex> lock(m_mutex);
  std::cout << std::this_thread::get_id() << " is inside critical region\n";

  // Wait for available work
  while(m_work_items.empty() && m_should_serve) {
    std::cout << std::this_thread::get_id() << " is waiting for work\n";
    m_work_available.wait(lock);
  }

  if (m_should_serve) {
    std::cout << std::this_thread::get_id() << " is doing work\n";
    return (*m_strategy)(m_work_items);
  }

  std::cout << std::this_thread::get_id() << " has skipped doing work\n";
  return std::nullopt;
}

template<typename T, typename W>
void VectorRouter<T, W>::stop() {
  {
    std::scoped_lock<std::mutex> lock(m_mutex);
    std::cout << "Router has stopped to serve workers\n";
    m_should_serve = false;
  }
  m_work_available.notify_all();
}

template<typename T, typename W>
void VectorRouter<T, W>::resume() {
  std::scoped_lock<std::mutex> lock(m_mutex);
  std::cout << "Router ha resumed to serve workers\n";
  m_should_serve = true;
}

template<typename T, typename W>
bool VectorRouter<T, W>::busy() {
  return size() != 0;
}

template<typename T, typename W>
size_t VectorRouter<T, W>::size() {
  std::scoped_lock<std::mutex> lock(m_mutex);
  return m_work_items.size();
}

} // namespace fur::mt
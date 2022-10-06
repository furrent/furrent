#include <mt/sharing_queue.hpp>

namespace fur::mt {

template <typename Work>
SharedQueue<Work>::SharedQueue() : _skip_waiting{false} {}

template <typename Work>
auto SharedQueue<Work>::try_extract(const policy::IPolicy<Work>& policy) -> Result {
  bool work_empty = false;

  _mutex.lock();
    auto result = _work.extract(policy);
  if (_work.size() == 0) work_empty = true;
  _mutex.unlock();

  if (work_empty) 
    _all_work_dispatched.notify_all();

  return result;
}

template <typename T>
void SharedQueue<T>::insert(T&& work) {
  {
    std::unique_lock<std::mutex> lock(_mutex);
    _work.insert(std::forward<T>(work));
  }
  _new_work_available.notify_all();
}

template <typename T>
void SharedQueue<T>::force_wakeup() {
  _new_work_available.notify_all();
}

template <typename T>
void SharedQueue<T>::wait_work() const {
  std::unique_lock<std::mutex> lock(_mutex);
  if (_skip_waiting || _work.size() != 0) return;

  _new_work_available.wait(
      lock, [this] { return _work.size() != 0 || _skip_waiting; });
}

template <typename T>
void SharedQueue<T>::wait_empty() const {
  std::unique_lock<std::mutex> lock(_mutex);
  if (_skip_waiting || _work.size() == 0) 
    return;

  _all_work_dispatched.wait(
      lock, [this] { return _work.size() == 0 || _skip_waiting; });
}

template<typename T>
void SharedQueue<T>::mutate(MutateFn mutation) {
  {
    std::unique_lock<std::mutex> lock(_mutex);
    _work.mutate(mutation);
  }
  force_wakeup();
}

template <typename T>
void SharedQueue<T>::begin_skip_waiting() {
  {
    std::unique_lock<std::mutex> lock(_mutex);
    _skip_waiting = true;
  }
  force_wakeup();
}

}  // namespace fur::mt
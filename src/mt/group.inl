#include <limits>
#include <mt/group.hpp>
#include <stdexcept>

namespace fur::mt {

template <typename State>
ThreadGroup<State>::ThreadGroup() : _should_terminate{false} {}

template <typename State>
ThreadGroup<State>::~ThreadGroup() {
  if (!_should_terminate) terminate();
}

template <typename State>
void ThreadGroup<State>::launch(ThreadFn fn, int64_t max_worker_threads) {
  if (max_worker_threads < 0) {
    throw std::invalid_argument("expected positive number of workers");
  }

  _thread_fn = fn;
  int64_t size = std::thread::hardware_concurrency();
  if (max_worker_threads != 0) size = std::min(size, max_worker_threads);

  _states.resize(size);
  for (int64_t i = 0; i < size; i++)
    _threads.emplace_back(std::bind(&ThreadGroup::thread_main, this, i), i,
                          std::ref(*this));
}

template <typename State>
void ThreadGroup<State>::thread_main(int64_t index) {
  State& state = _states.at(index);
  _thread_fn(Runner(_mutex, _should_terminate), state, index);
}

template <typename State>
void ThreadGroup<State>::terminate() {
  _mutex.lock();
  _should_terminate = true;
  _mutex.unlock();

  for (auto& thread : _threads) thread.join();
}

template <typename State>
std::vector<State>& ThreadGroup<State>::get_states() {
  return _states;
}

template <typename State>
int64_t ThreadGroup<State>::get_worker_count() const {
  return _states.size();
}

}  // namespace fur::mt
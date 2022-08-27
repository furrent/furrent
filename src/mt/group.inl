#include <mt/group.hpp>

namespace fur::mt {

template<typename State>
ThreadGroup<State>::ThreadGroup(ThreadFn fn, size_t max_worker_threads)
: _thread_fn{std::move(fn)}, _should_terminate{false} {

    size_t size = std::thread::hardware_concurrency();
    if (max_worker_threads != 0) size = std::min(size, max_worker_threads);

    for (size_t i = 0; i < size; i++)
        _threads.push_back(Thread{
            std::thread(std::bind(&ThreadGroup::thread, this, i), i, std::ref(*this)),
            State{} // Must be default constructible
        });
}

template<typename State>
ThreadGroup<State>::~ThreadGroup() {
    terminate();
    for (auto& thread : _threads)
        thread.handle.join(); 
}

template<typename State>
void ThreadGroup<State>::thread(size_t index) {
    Controller control(_mutex, _should_terminate);
    _thread_fn(control, _threads[index].state, index);
}

template<typename State>
void ThreadGroup<State>::terminate() {
    std::scoped_lock<std::mutex> lock(_mutex);
    _should_terminate = true;
}

template<typename State>
State& ThreadGroup<State>::get_thread_state(size_t thread) {
    return _threads[thread].state;
}

} // namespace fur::mt
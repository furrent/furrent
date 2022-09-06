#include <mt/sharing_queue.hpp>

namespace fur::mt {

template<typename Work>
SharingQueue<Work>::SharingQueue()
    : _skip_waiting{false} { }

template<typename Work>
auto SharingQueue<Work>::try_extract(const policy::IPolicy<Work>& policy) -> Result {
    
    bool work_empty = false;
    
    _mutex.lock();
    auto result = _work.extract(policy);
    if (_work.size() == 0)
        work_empty = true;
    _mutex.unlock();

    if (work_empty)
        _all_work_dispatched.notify_all();
        
    return result;
}

template<typename Work>
void SharingQueue<Work>::insert(Work&& work) {

    _mutex.lock();
    _work.insert(std::forward<Work>(work));
    _mutex.unlock();

    _new_work_available.notify_all();
}

template<typename Work>
auto SharingQueue<Work>::steal() -> Result {
    return try_extract(policy::FIFOPolicy<Work>{});
}

template<typename Work>
void SharingQueue<Work>::wait_for_work() const {

    std::unique_lock<std::mutex> lock(_mutex);
    if (_skip_waiting) return;

    _new_work_available.wait(lock, [this] {
        return _work.size() != 0 || _skip_waiting;
    });
}

template<typename Work>
void SharingQueue<Work>::begin_skip_waiting() {
    _mutex.lock();
    _skip_waiting = true;
    _mutex.unlock();

    _new_work_available.notify_all();
}

} // namespace fur::mt
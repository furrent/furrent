/**
 * @file channel.inl
 * @author Filippo Ziche
 * @version 0.1
 * @date 2022-08-26
 */

#include <mt/channel.hpp>

namespace fur::mt::channel {

template<typename T>
StrategyChannel<T>::StrategyChannel()
: _serving{true} { }

template<typename T>
StrategyChannel<T>::~StrategyChannel() {
    set_serving(false);
}

template<typename T>
void StrategyChannel<T>::insert(T item, MyStrategy* strategy) {
    {
        // Documentation suggests it's better to unlock the
        // mutex before notifing the CV

        std::scoped_lock<std::mutex> lock(_work_mutex);
        strategy->insert(item, _work);
    }
    _work_available.notify_one();
}

template<typename T>
auto StrategyChannel<T>::extract(MyStrategy* strategy) -> MyResult {

    // If the work list is empty then wait
    std::unique_lock<std::mutex> lock(_work_mutex);
    _work_available.wait(lock, [this]{
            return !_work.empty() || !_serving;
    });

    // We must exit because we are not serving anymore
    if (!_serving) 
        return MyResult::error(StrategyChannelError::StoppedServing);
        
    // Extracts a work-item
    std::optional<T> result = strategy->extract(_work);
    if (!result.has_value())
        return MyResult::error(StrategyChannelError::StrategyFailed);

    bool notify = _work.empty();

    // Documentation suggests it's better to unlock the
    // mutex before notifing the CV
    lock.unlock();
    if (notify)
        _work_finished.notify_all();
        
    return MyResult::ok(*result);
}

template<typename T>
auto StrategyChannel<T>::try_extract(MyStrategy* strategy) -> MyResult {

    // If the work list is empty then wait
    std::unique_lock<std::mutex> lock(_work_mutex);
    if (!_serving) 
        return MyResult::error(StrategyChannelError::StoppedServing);
    if (_work.empty())
        return MyResult::error(StrategyChannelError::Empty);

    // Extracts a work-item
    std::optional<T> result = strategy->extract(_work);
    if (!result.has_value())
        return MyResult::error(StrategyChannelError::StrategyFailed);

    bool notify = _work.empty();

    // Documentation suggests it's better to unlock the
    // mutex before notifing the CV
    lock.unlock();
    if (notify)
        _work_finished.notify_all();
        
    return MyResult::ok(*result);
}

template<typename T>
void StrategyChannel<T>::wait_empty() {
    
    std::unique_lock<std::mutex> lock(_work_mutex);
    if (_work.empty()) return;

    _work_finished.wait(lock, [this]{
        return _work.empty();
    });
}

template<typename T>
const std::list<T>& StrategyChannel<T>::get_work_list() const {
    return _work;
}

template<typename T>
void StrategyChannel<T>::set_serving(bool value) {
    {
        // Documentation suggests it's better to unlock the
        // mutex before notifing the CV

        std::scoped_lock<std::mutex> lock(_work_mutex);
        _serving = value;
    }
    _work_available.notify_all();
}

template<typename T>
void StrategyChannel<T>::mutate(std::function<bool(std::list<T>&)> mutation) {
    bool notify;
    {
        // Documentation suggests it's better to unlock the
        // mutex before notifing the CV

        std::scoped_lock<std::mutex> lock(_work_mutex);
        notify = mutation(_work);
    }

    if (notify)
        _work_available.notify_all();
}

} // namespace fur::mt::channel

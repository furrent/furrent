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
void StrategyChannel<T>::insert(T item, Strategy* strategy) {

    // No strategy was passed as an argument
    if (!strategy) return;

    {
        // Documentation suggests it's better to unlock the
        // mutex before notifing the CV

        std::scoped_lock<std::mutex> lock(_work_mutex);
        strategy->insert(item, _work);
    }
    _work_available.notify_one();
}

template<typename T>
auto StrategyChannel<T>::extract(Strategy* strategy) -> Result {

    typedef typename Strategy::Result StrategyResult;

    // No strategy was passed as an argument
    if (!strategy)
        return Result::ERROR(StrategyChannelError::StrategyFailed);

    // If the work list is empty then wait
    std::unique_lock<std::mutex> lock(_work_mutex);
    _work_available.wait(lock, [this]{
            return !_work.empty() || !_serving;
    });

    // We must exit because we are not serving anymore
    if (!_serving) 
        return Result::ERROR(StrategyChannelError::StoppedServing);
        
    // Extracts a work-item
    StrategyResult result = strategy->extract(_work);
    if (!result) {
        switch (result.error())
        {
        case strategy::StrategyError::Empty:
            return Result::ERROR(StrategyChannelError::Empty);
        default:
            return Result::ERROR(StrategyChannelError::StrategyFailed);
        }
    }

    bool notify = _work.empty();

    // Documentation suggests it's better to unlock the
    // mutex before notifing the CV
    lock.unlock();
    if (notify)
        _work_finished.notify_all();
        
    return Result::OK(std::move(*result));
}

template<typename T>
auto StrategyChannel<T>::try_extract(Strategy* strategy) -> Result {

    typedef typename Strategy::Result StrategyResult;

    // No strategy was passed as an argument
    if (!strategy)
        return Result::ERROR(StrategyChannelError::StrategyFailed);

    // If the work list is empty then wait
    std::unique_lock<std::mutex> lock(_work_mutex);
    if (!_serving) 
        return Result::ERROR(StrategyChannelError::StoppedServing);
    if (_work.empty())
        return Result::ERROR(StrategyChannelError::Empty);

    // Extracts a work-item
    StrategyResult result = strategy->extract(_work);
    if (!result) {
        switch (result.error() )
        {
        case strategy::StrategyError::Empty:
            return Result::ERROR(StrategyChannelError::Empty);
        }
        return Result::ERROR(StrategyChannelError::StrategyFailed);
    }

    bool notify = _work.empty();

    // Documentation suggests it's better to unlock the
    // mutex before notifing the CV
    lock.unlock();
    if (notify)
        _work_finished.notify_all();
        
    return Result::OK(*result);
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

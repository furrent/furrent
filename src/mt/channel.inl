/**
 * @file channel.inl
 * @author Filippo Ziche
 * @version 0.1
 * @date 2022-08-26
 */

#include <mt/channel.hpp>

namespace fur::mt {

template<typename Stored, typename Served>
StrategyChannel<Stored, Served>::StrategyChannel()
: _serving{true} { }

template<typename Stored, typename Served>
void StrategyChannel<Stored, Served>::insert(Stored item) {
    {
        // Documentation suggests it's better to unlock the
        // mutex before notifing the CV
        std::scoped_lock<std::mutex> lock(_work_mutex);
        _work.push_front(item);
    }
    _work_available.notify_one();
}

template<typename Stored, typename Served>
std::optional<Served> StrategyChannel<Stored, Served>::extract(Strategy* strategy) {
    
    // If the work list is empty then wait
    std::unique_lock<std::mutex> lock(_work_mutex);
    _work_available.wait(lock, [this]{
        return !_work.empty() || !_serving;
    });

    // We must exit because we are not serving anymore
    if (!_serving) return std::nullopt;

    // Extracts a work-item an notifies if there is no more work
    Served result = strategy->extract(_work);
    if (_work.empty()) _work_finished.notify_all();
    return { result };
}

template<typename Stored, typename Served>
void StrategyChannel<Stored, Served>::wait_empty() {
    
    std::unique_lock<std::mutex> lock(_work_mutex);
    if (_work.empty()) return;

    _work_finished.wait(lock, [this]{
        return _work.empty();
    });
}

template<typename Stored, typename Served>
const std::list<Stored>& StrategyChannel<Stored, Served>::get_work_list() const {
    return _work;
}

template<typename Stored, typename Served>
void StrategyChannel<Stored, Served>::set_serving(bool value) {
    {
        // Documentation suggests it's better to unlock the
        // mutex before notifing the CV
        std::scoped_lock<std::mutex> lock(_work_mutex);
        _serving = value;
    }
    _work_available.notify_all();
}

} // namespace fur::mt

#include <mt/task.hpp>

namespace fur::mt {
    
auto PriorityPolicy::extract(Iterator begin, Iterator end) const -> Iterator {
    
    size_t priority_best = 0;
    Iterator result = end;
    while(begin != end) {

        ITask::Wrapper& item = *begin;
        const size_t priority = item->priority();

        if (priority > priority_best) {
            priority_best = priority;
            result = begin;
        }

        ++begin;
    }
    return result;
}

} // namespace fur::mt

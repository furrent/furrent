#include <mt/task.hpp>

namespace fur::mt {

auto PriorityPolicy::extract(Iterator begin, Iterator end) const -> Iterator {
  size_t priority_best = 0;
  Iterator result = end;
  while (begin != end) {
    ITask::Wrapper& item = *begin;
    size_t priority = item->priority();
    
    // Sleeping tasks
    if (priority == mt::Priority::PRIORITY_NONE)
      continue;

    // Check if best
    if (priority > priority_best) {
      priority_best = priority;
      result = begin;
    }

    ++begin;
  }
  return result;
}

}  // namespace fur::mt

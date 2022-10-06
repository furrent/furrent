#include <tasks/torrent_task.hpp>
#include <furrent.hpp>

namespace fur::mt {
    
TorrentTask::TorrentTask(std::shared_ptr<TorrentHandle> descriptor)
: descriptor{descriptor} 
{
  // Get default custom logger
  logger = spdlog::get("custom");
}

TorrentTask::TorrentTask(SharedQueue<Wrapper>* spawn_queue, std::shared_ptr<TorrentHandle> descriptor)
: ITask(spawn_queue), descriptor{descriptor}
{
  // Get default custom logger
  logger = spdlog::get("custom");
}

auto PriorityPolicy::extract(Iterator begin, Iterator end) const -> Iterator {
  size_t priority_best = 0;
  Iterator result = end;
  while (begin != end) {

    // Consider only running tasks
    TorrentTask::Wrapper& item = *begin;
    if (item->state == mt::TaskState::Running) {

      size_t priority = item->priority();
      if (priority > priority_best) {
        priority_best = priority;
        result = begin;
      }  
    }
    
    ++begin;
  }
  return result;
}

} // namespace fur::tasks

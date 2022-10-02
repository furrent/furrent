#include <tasks/torrent_task.hpp>

namespace fur::tasks {
    
TorrentTask::TorrentTask(std::shared_ptr<TorrentHandle> descriptor)
: descriptor{descriptor}
{
    // Get default custom logger
    logger = spdlog::get("custom");
}

} // namespace fur::tasks

#include <furrent.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <random>

#include <bencode/bencode_parser.hpp>
#include <policy/policy.hpp>
#include <log/logger.hpp>

namespace fur {
    
TorrentFileLoadTask::TorrentFileLoadTask(TorrentDescriptor& desc)
: _descriptor(desc) { }

void TorrentFileLoadTask::execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) {

    // Default global logger
    auto logger = spdlog::get("custom");

    std::ifstream file(_descriptor.filename);
    std::string content;

    if (file) {

        logger->info("Loading torrent file from {}", _descriptor.filename);

        std::ostringstream ss;
        ss << file.rdbuf();
        content = ss.str();

    } else {

        logger->error("Error loading torrent from {}", _descriptor.filename);

        // TODO: manage the exception
        /*
        throw std::invalid_argument(
            "fur::Furrent::add_torrent: invalid path or "
            "missing permission");
        */
       return;
    }

    // Create torrent_manager for the file
    logger->info("Parsing torrent file {}", _descriptor.filename);
    auto parser = fur::bencode::BencodeParser();
    auto b_tree = parser.decode(content);

    // From now on the descriptor is available to all
    _descriptor.torrent = std::make_optional<fur::torrent::TorrentFile>(*b_tree);

    const size_t piece_length = _descriptor.torrent->piece_length;
    const size_t pieces_count = _descriptor.torrent->length / piece_length;


    // Generate all downloading tasks
    logger->info("Generating {} pieces torrent file {}", pieces_count, _descriptor.filename);
    for(size_t piece = 0; piece < pieces_count; piece++) {

        size_t offset = piece * piece_length;
        local_queue.insert(std::make_unique<DownloadPieceTask>(
            _descriptor, piece, offset, piece_length));
    }
}

DownloadPieceTask::DownloadPieceTask(TorrentDescriptor& desc, size_t index, size_t offset, size_t bytes)
: _descriptor{desc}, _index{index}, _offset{offset}, _bytes{bytes} { }

void DownloadPieceTask::execute(mt::SharingQueue<mt::ITask::Wrapper>& local_queue) {

    // Default global logger
    auto logger = spdlog::get("custom");
    //logger->info("Downloading piece {} of {} for torrent {}", 
    //    _index, _descriptor.torrent->piece_length, _descriptor.filename);
}

Furrent::Furrent() {

    /// Local queues of all workers
    const size_t concurrency = std::thread::hardware_concurrency();
    _local_queues = new TaskSharingQueue[concurrency];

    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    /// This is the core of all workers
    _workers.launch(
        std::bind(&Furrent::thread_main, this, _1, _2, _3)
    );
}


Furrent::~Furrent() {

    _global_queue.begin_skip_waiting();
    _workers.terminate();

    delete[] _local_queues;
}


void Furrent::thread_main(mt::Runner runner, WorkerState& state, size_t index) {

    // TODO: custom policy per thread etc...
    policy::LIFOPolicy<mt::ITask::Wrapper> task_policy;
    const size_t concurrency = _workers.get_worker_count();

    // Default global logger
    auto logger = spdlog::get("custom");

    // How many times we have to fail stealing to go to sleep
    const int STEALING_LIMIT = 5000;
    // How many times the thread tried to steal without success
    int failed_stealing_count = 0;

    auto& local_queue = _local_queues[index];
    while(runner.alive()) {

        // First we check our own local queue
        auto local_work = local_queue.try_extract(task_policy);
        if (local_work.valid()) {

            //logger->debug("thread {:02d} is executing local work", index);
            (*local_work)->execute(local_queue);
        }
        else {

            // Then we check the global queue
            auto global_work = _global_queue.try_extract(task_policy);
            if (global_work.valid()) {

                //logger->debug("thread {:02d} is executing global work", index);
                (*global_work)->execute(local_queue);
            }
            else {

                // If we tried to steal too many times then wait for work in global queue
                if (failed_stealing_count >= STEALING_LIMIT) {

                    logger->debug("thread {:02d} is waiting for work on global queue", index);
                    _global_queue.wait_work();
                    failed_stealing_count = 0;
                }
                // If there is nothing then we steal from a random worker
                else {
                    size_t steal_idx = rand() % concurrency;
                    if (steal_idx == index) steal_idx = (steal_idx + 1) % concurrency;

                    auto steal_work = _local_queues[steal_idx].steal();
                    if (steal_work.valid()) {

                        logger->debug("thread {:02d} is executing stolen work from {:02d}", index, steal_idx);
                        (*steal_work)->execute(local_queue);
                    }
                    else {
                        failed_stealing_count += 1;
                    }
                }
            }
        }
    }
}

void Furrent::add_torrent(const std::string& filename) {

    /// Allocate descriptor for the new torrent 
    _descriptors.push_front({ filename, std::nullopt });
    auto& descriptor = _descriptors.front();

    /// Begin loading task 
    _global_queue.insert(std::make_unique<TorrentFileLoadTask>(descriptor));
}

const std::list<TorrentDescriptor>& Furrent::get_descriptors() const {
    return _descriptors;
}

} // namespace fur

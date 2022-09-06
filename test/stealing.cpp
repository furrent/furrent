#include <catch2/catch.hpp>

#include <thread>
#include <iostream>
#include <atomic>

#include <mt/sharing_queue.hpp>
#include <policy/policy.hpp>

using namespace fur::policy;
using namespace fur::mt;

/*
TEST_CASE("[mt] Stealing simple") {

    // Queue owner can push work
    SharingQueue<int> owner_queue;
    std::thread owner([&] {
        owner_queue.insert(5);    
    });

    // Thief steals from queue owner
    int result_value = 0;
    std::thread thief([&] {
        for(;;) {
            auto result = owner_queue.steal();
            //if (!result && result.error() == SharingQueue<int>::Error::Empty)
            //    return;

            result_value = *result;
        }
    });

    owner.join();
    thief.join();

    REQUIRE(result_value == 5);
}

struct Task {
    int a = 0;

    void execute() { }
};

TEST_CASE("[mt] Stealing hierarchical") {

    const size_t THREAD_COUNT = 16;
    std::atomic_bool alive = true;

    // Test policy
    FIFOPolicy<Task> policy;

    // Contains all global work
    SharingQueue<Task> global_queue;

    // All local work queues
    std::array<SharingQueue<Task>, THREAD_COUNT> local_queues;

    std::vector<std::thread> workers;
    for (size_t idx = 0; idx < THREAD_COUNT; idx++)
        workers.emplace_back([&, idx] {
            auto& local_queue = local_queues[idx];
            while(alive) {

                // First we check our own local queue
                //std::cout << idx << " extracting from local queue...\n";
                auto local_work = local_queue.try_extract(policy);
                //std::cout << idx << " local extraction done\n";

                if (local_work) { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
                else {

                    // Then we check the global queue
                    //std::cout << idx << " extracting from global queue...\n";
                    auto global_work = global_queue.try_extract(policy);
                    //std::cout << idx << " global extraction done\n";
                    
                    if (global_work) { 

                        // Simulate the insertion of dependent tasks
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        for (int i = 0; i < 10; i++)
                            global_queue.insert({ 2 });

                    }
                    else {

                        // If there is nothing then we steal from a random thread
                        size_t steal_idx = rand() % THREAD_COUNT;
                        if (steal_idx == idx) steal_idx = (steal_idx + 1) % THREAD_COUNT; 

                        //std::cout << idx << " stealing from local queue of thread " << steal_idx << "...\n";
                        auto steal_work = local_queues[steal_idx].steal();
                        //std::cout << idx << " stealing done\n";

                        if (steal_work) { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
                        else {

                            // Otherwise we sleep on the global queue
                            //std::cout << idx << " waiting for global work...\n";
                            global_queue.wait_for_work();
                            //std::cout << idx << " waiting finished\n";
                        }
                    }
                }
            }
            //std::cout << idx << " exit!\n";
        });

    // Try to download 10 torrents each with 10000 pieces
    for(int i = 0; i < 1000; i++)
        global_queue.insert({ 1 });

    //std::cout << "GLOBAL EXIT\n";

    alive = false;
    global_queue.begin_skip_waiting();
    for(auto& worker : workers)
        worker.join();
}*/
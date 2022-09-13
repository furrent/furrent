#include <catch2/catch.hpp>

#include <thread>
#include <iostream>
#include <memory>
#include <atomic>

#include <mt/sharing_queue.hpp>
#include <policy/policy.hpp>

using namespace fur::policy;
using namespace fur::mt;

const size_t THREAD_COUNT = 16;

TEST_CASE("[mt] Stealing simple") {

  std::atomic_bool alive = true;
  int result_value;

  // Queue owner can push work
  SharingQueue<int> owner_queue;
  std::thread owner([&] {
    owner_queue.insert(5);
  });

  // Thief steals from queue owner
  std::thread thief([&] {
    while(alive) {
      auto result = owner_queue.steal();
      if (result.valid())
        result_value = *result;
      else
        std::this_thread::sleep_for(
            std::chrono::milliseconds(100));
    }
  });

  owner.join();

  owner_queue.wait_empty();
  alive = false;
  thief.join();

  REQUIRE(result_value == 5);
}

TEST_CASE("[mt] Stealing hierarchical") {

  std::atomic_bool alive = true;

  // Test policy
  FIFOPolicy<int> policy;

  // Contains all global work
  SharingQueue<int> global_queue;

  // All local work queues
  std::array<SharingQueue<int>, THREAD_COUNT> local_queues;

  std::vector<std::thread> workers;
  for (size_t idx = 0; idx < THREAD_COUNT; idx++)
    workers.emplace_back([&, idx] {
      auto& local_queue = local_queues[idx];
      while(alive) {

        // First we check our own local queue
        auto local_work = local_queue.try_extract(policy);
        if (local_work.valid()) {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else {

          // Then we check the global queue
          auto global_work = global_queue.try_extract(policy);
          if (global_work.valid()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            for (int i = 0; i < 10; i++)
              local_queue.insert({ 2 });
          }
          else {

            // If there is nothing then we steal from a random thread
            size_t steal_idx = rand() % THREAD_COUNT;
            if (steal_idx == idx) steal_idx = (steal_idx + 1) % THREAD_COUNT;

            auto steal_work = local_queues[steal_idx].steal();
            if (steal_work.valid()) {
              std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            else {

              // Otherwise we sleep on the global queue
              global_queue.wait_work();
            }
          }
        }
      }
    });

  // Try to download 10 torrents each with 10000 pieces
  for(int i = 0; i < 1000; i++)
    global_queue.insert({ 1 });

  std::this_thread::sleep_for(std::chrono::seconds(5));

  alive = false;
  global_queue.begin_skip_waiting();
  for(auto& worker : workers)
    worker.join();
}

class ITask {
 public:
  typedef std::unique_ptr<ITask> TaskPtr;

  /// Implements task custom logic
  virtual void operator()(SharingQueue<TaskPtr>& local_queue) = 0;
};

class TerminalTask : public ITask {
 public:
  void operator()(SharingQueue<TaskPtr>& local_queue) override {
    /* ... */
  }
};

class GeneratorTask : public ITask {
 public:
  void operator()(SharingQueue<TaskPtr>& local_queue) override {
    for(int i = 0; i < 10; i++) {
      local_queue.insert(std::make_unique<TerminalTask>());
    }
  }
};


TEST_CASE("[mt] Stealing hierarchical with generator tasks") {

  typedef std::unique_ptr<ITask> Task;
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

        if (local_work.valid()) {
          (*(*local_work))(local_queue);
        }
        else {

          // Then we check the global queue
          //std::cout << idx << " extracting from global queue...\n";
          auto global_work = global_queue.try_extract(policy);
          //std::cout << idx << " global extraction done\n";

          if (global_work.valid()) {
            (*(*global_work))(local_queue);
          }
          else {

            // If there is nothing then we steal from a random thread
            size_t steal_idx = rand() % THREAD_COUNT;
            if (steal_idx == idx) steal_idx = (steal_idx + 1) % THREAD_COUNT;

            //std::cout << idx << " stealing from local queue of thread " << steal_idx << "...\n";
            auto steal_work = local_queues[steal_idx].steal();
            //std::cout << idx << " stealing done\n";

            if (steal_work.valid()) {
              (*(*steal_work))(local_queue);
            }
            else {

              // Otherwise we sleep on the global queue
              global_queue.wait_work();
            }
          }
        }
      }
    });

  // Try to download 10 torrents each with 10000 pieces
  for(int i = 0; i < 1000; i++)
    global_queue.insert(std::make_unique<GeneratorTask>());

  std::this_thread::sleep_for(std::chrono::seconds(5));

  alive = false;
  global_queue.begin_skip_waiting();
  for(auto& worker : workers)
    worker.join();
}
#include <atomic>
#include <condition_variable>
#include <mt/sharing_queue.hpp>
#include <policy/policy.hpp>
#include <thread>

#include "catch2/catch.hpp"

struct Item {
  int value;
};

TEST_CASE("[Sharing queue][policy] Standard operations") {
  fur::mt::SharedQueue<Item> items;

  std::atomic_bool alive{true};
  std::atomic_int32_t counter{0};
  std::condition_variable CV;

  std::thread reader([&] {
    fur::policy::FIFOPolicy<Item> policy;
    items.wait_work();

    while (alive) {
      auto result = items.try_extract(policy);
      if (result.valid()) counter += result->value;

      CV.notify_one();
    }
    CV.notify_one();
  });

  const int64_t TOTAL_COUNT = 100;
  for (int i = 0; i < TOTAL_COUNT; i++) items.insert({1});

  items.wait_empty();

  {
    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    CV.wait(lock);
  }

  REQUIRE(counter == TOTAL_COUNT);

  alive.exchange(false);
  reader.join();
}

TEST_CASE("[Sharing queue][policy] Mutation") {
  fur::mt::SharedQueue<Item> items;

  const int64_t TOTAL_COUNT = 100;
  for (int i = 0; i < 100; i++) items.insert({0});

  std::thread mutator([&] {
    int counter = 0;
    items.mutate([&](Item& item) -> bool {
      item.value = 1;

      bool result = false;
      if (counter % 2 != 0) result = true;

      counter += 1;
      return result;
    });
  });

  mutator.join();

  int64_t count = 0;
  fur::policy::FIFOPolicy<Item> policy;
  for (int i = 0; i < TOTAL_COUNT / 2; i++) {
    auto result = items.try_extract(policy);
    REQUIRE(result.valid());
    count += result->value;
  }

  REQUIRE(count == TOTAL_COUNT / 2);
}
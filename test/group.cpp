#include <iostream>
#include <mt/group.hpp>
#include <mutex>
#include <vector>

#include "catch2/catch.hpp"

using namespace fur::mt;

struct ThreadState {
  int64_t value;
};

TEST_CASE("Thread group creation") {
  const int64_t THREADS_NUM = std::thread::hardware_concurrency();

  std::vector<bool> checked;
  checked.resize(THREADS_NUM);

  std::mutex mx;
  int64_t left_waiting = THREADS_NUM;

  {
    // Threads whould exists only inside this scope
    ThreadGroup<ThreadState> group{};
    group.launch(
        [&](Runner runner, ThreadState& state, int64_t index) {
          assert(runner.alive());
          state.value = 100;
          checked[index] = true;

          auto lock = std::unique_lock(mx);
          left_waiting--;
        },
        THREADS_NUM);

    while (true) {
      {
        auto lock = std::unique_lock(mx);
        if (left_waiting <= 0) break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    group.terminate();

    for (auto& state : group.get_states()) REQUIRE(state.value == 100);
  }

  bool valid = true;
  for (auto check : checked) valid &= check;

  REQUIRE(valid);
}
#include "catch2/catch.hpp"

#include <iostream>
#include <vector>

#include <mt/group.hpp>

using namespace fur::mt;

struct ThreadState { int value; };

TEST_CASE("Thread group creation") {

    const size_t THREADS_NUM = std::thread::hardware_concurrency();

    std::vector<bool> checked;
    checked.resize(THREADS_NUM);

    {
        // Threads whould exists only inside this scope
        ThreadGroup<ThreadState> group{};
        group.launch([&](Runner runner, ThreadState& state, size_t index) {
            while(runner.alive()) {
                state.value = 100;
                checked[index] = true;
            }
        }, THREADS_NUM);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        group.terminate();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        for(auto& state : group.get_states())
            REQUIRE(state.value == 100);
    }

    bool valid = true;
    for(auto check : checked)
        valid &= check;
    
    REQUIRE(valid);
}
#include "catch2/catch.hpp"

#include <iostream>
#include <vector>

#include <mt/group.hpp>

using namespace fur::mt;

struct ThreadState { int value; };

TEST_CASE("Thread group creation") {

    auto threads_num = std::thread::hardware_concurrency();
    std::vector<bool> checked(threads_num);

    {
        // Threads whould exists only inside this scope
        ThreadGroup<ThreadState> group{};
        group.launch([&](Runner runner, ThreadState& state, size_t index) {
            while(runner.alive()) {
                state.value = 100;
                checked[index] = true;
            }
        }, threads_num);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        group.terminate();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        for(size_t i = 0; i < threads_num; i++)
            REQUIRE(group.get_thread_state(i).value == 100);
    }

    bool valid = true;
    for (bool value : checked)
        valid &= value;
    
    REQUIRE(valid);
}
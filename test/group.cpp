#include "catch2/catch.hpp"

#include <iostream>

#include <mt/group.hpp>

using namespace fur::mt;

struct ThreadState { int value; };

TEST_CASE("Thread group creation") {

    const size_t THREADS_NUM = 4;
    bool checked[THREADS_NUM] = { false };

    {
        // Threads whould exists only inside this scope
        ThreadGroup<ThreadState> group([&](Controller controller, ThreadState& state, size_t index) {
            while(controller.alive()) {
                checked[index] = true;
                state.value = 100;
            }
        }, THREADS_NUM);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        group.terminate();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        for(size_t i = 0; i < THREADS_NUM; i++)
            REQUIRE(group.get_thread_state(i).value == 100);
    }

    bool valid = true;
    for (bool value : checked)
        valid &= value;
    
    REQUIRE(valid);
}
/**
 * @file channel.cpp
 * @author Filippo Ziche
 * @version 0.1
 * @date 2022-08-26
 */

#include "catch2/catch.hpp"

#include <thread>
#include <iostream>
#include <algorithm>

#include <mt/channel.hpp>
#include <mt/group.hpp>

using namespace fur::mt;

struct Stored { int val; };
struct Served { int val; };

class TestStrategy : public IListStrategy<Stored, Served> {
public:
    std::optional<Served> extract(std::list<Stored>& list) override {
        Served result = { list.front().val * 2 };
        list.pop_front();
        return result;
    }
};

struct ThreadState { };

TEST_CASE("Channel and Thread Group interop") {

    const int SIZE = 10000;

    StrategyChannel<Stored, Served> input{};
    StrategyChannel<Served, Served> output{};
    
    TestStrategy strategy;

    {
        for (int i = 0; i < SIZE; i++)
            input.insert({ 1 });

        ThreadGroup<ThreadState> group{};
        group.launch([&](Runner runner, ThreadState& state, size_t index) {
            while(runner.alive()) {
                auto served = input.extract(&strategy);
                if (served.has_value())
                    output.insert(*served);
            }
        });

        input.wait_empty();

        const auto& input_list = input.get_work_list();
        REQUIRE(input_list.empty());

        input.set_serving(false);
    }

    const auto& output_list = output.get_work_list();
    REQUIRE(output_list.size() == SIZE);

    int sum = 0;
    for (const Served& served : output_list)
        sum += served.val;

    REQUIRE(sum == SIZE * 2);
}
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
#include <strategy/strategy.hpp>

using namespace fur::mt;
using namespace fur::mt::channel;
using namespace fur::strategy;

struct Value { int val; };

class TestStrategy : public IListStrategy<Value> {
public:
    std::optional<Value> extract(std::list<Value>& list) override {
        Value result = { list.front().val * 2 };
        list.pop_front();
        return result;
    }

    void insert(Value item, std::list<Value>& list) override {
        list.push_back(item);
    }
};

struct ThreadState { };

TEST_CASE("Channel and Thread Group interop") {

    const int SIZE = 10000;

    StrategyChannel<Value> input{};
    StrategyChannel<Value> output{};
    
    TestStrategy strategy;
    {
        for (int i = 0; i < SIZE; i++)
            input.insert({ 1 }, &strategy);

        ThreadGroup<ThreadState> group{};
        group.launch([&](Runner runner, ThreadState& state, size_t index) {
            bool alive = true;
            while(runner.alive() && alive) {

                auto result = input.extract(&strategy);
                if (result.has_error())
                    switch (result.get_error())
                    {
                    case StrategyChannelError::StrategyFailed:
                    case StrategyChannelError::Empty:
                    case StrategyChannelError::StoppedServing:
                        alive = false;
                        continue;
                    }

                auto value = result.get_value();
                output.insert(value, &strategy);
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
    for (const Value& value : output_list)
        sum += value.val;

    REQUIRE(sum == SIZE * 2);
}
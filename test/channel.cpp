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

TEST_CASE("StrategyChannel base behaviour") {

    StrategyChannel<Value> input{};
    StrategyChannel<Value> output{};
    
    TestStrategy strategy;

    std::vector<std::thread> threads;
    for (size_t i = 0; i < std::thread::hardware_concurrency(); i++)
        threads.emplace_back([&]{
            
            bool alive = true;
            while(alive) {

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

    
    // Batch of work available after threads creation
    const int SIZE = 10000;
    for (int i = 0; i < SIZE; i++)
        input.insert({ 1 }, &strategy);

    input.wait_empty();
    
    const auto& input_list = input.get_work_list();
    REQUIRE(input_list.empty());

    input.set_serving(false);
    for (auto& thread : threads)
        thread.join();

    for(const auto& value : input_list)
        std::cout << value.val << "\n";

    const auto& output_list = output.get_work_list();
    REQUIRE(output_list.size() == SIZE);

    int sum = 0;
    for (const Value& value : output_list)
        sum += value.val;

    REQUIRE(sum == SIZE * 2);
}
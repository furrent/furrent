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
using namespace fur::strategy;

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

TEST_CASE("StrategyChannel base behaviour") {

    StrategyChannel<Stored, Served> input{};
    StrategyChannel<Served, Served> output{};
    
    TestStrategy strategy;

    std::vector<std::thread> threads;
    for (size_t i = 0; i < std::thread::hardware_concurrency(); i++)
        threads.emplace_back([&]{
            std::optional<Served> work_to_do;
            while(work_to_do = input.extract(&strategy))
                output.insert(*work_to_do);
        });

    
    // Batch of work available after threads creation
    const int SIZE = 10000;
    for (int i = 0; i < SIZE; i++)
        input.insert({ 1 });

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
    for (const Served& served : output_list)
        sum += served.val;

    REQUIRE(sum == SIZE * 2);
}
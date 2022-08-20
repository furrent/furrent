#include <mt/thread_pool.hpp>
#include <mt/router.hpp>

#include <random>
#include <iostream>
#include <optional>
#include <unordered_map>

#include "catch2/catch.hpp"
using namespace fur::mt;

// Data inside the router
struct From {
    int value;
};

// Data sent to workers
struct To {
    int value_processed;
};

// Choose at random an element in the collection of Foo transforming it into a Bar
class UniformRouterStrategy : public IVectorRouterStrategy<From, To> {
public:
    UniformRouterStrategy() {
        srand(time(nullptr));
    }

    std::optional<To> operator() (std::vector<From>& items) override {
        if (!items.empty()) {
        
            int index = rand() % items.size();
            From item = items.at(index);
            items.erase(items.begin() + index);

            To work{ item.value };
            return std::optional<To>{ work };
        }
        return std::nullopt;
    }
};

TEST_CASE("[mt] Correct uniform strategy behaviour") {

    UniformRouterStrategy strategy;
    std::vector<From> input = {
        { 1 }, { 2 }, { 3 },
        { 4 }, { 5 }, { 6 },
        { 7 }, { 8 }, { 9 }
    };

    std::vector<To> output;
    while(!input.empty()) {
        std::optional<To> selection = strategy(input);
        REQUIRE(selection);
        output.push_back(*selection);
    }

    REQUIRE(input.size()  == 0);
    REQUIRE(output.size() == 9);

    int sum = 0;
    for (auto& bar : output)
        sum += bar.value_processed;

    REQUIRE(sum == 45);
}

TEST_CASE("[mt] Worker Thread Pool") {

    const int THREADS_COUNT = 4;
    const int ITEMS_COUNT = 100;

    typedef VectorRouter<From, To> VectorRouter;
    typedef WorkerThreadPool<From, To> WorkerThreadPool;

    auto router = std::make_shared<VectorRouter>(
        std::make_unique<UniformRouterStrategy>());

    SECTION("Spin up and down")
    {
        // Only inside the scope there should be parallelism
        WorkerThreadPool pool(router, [&](To&) { });
    }

    SECTION("Busy behaviour")
    {
        // Lots of data to be worked on
        for(int i = 0; i < 1000; i++)
            router->insert({ 1 });

        WorkerThreadPool pool(router, [&](To&) { 
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        });

        pool.busy();
        REQUIRE(router->size() == 0);
    }

    SECTION("Simple uniform work behaviour")
    {
        std::unordered_map<std::thread::id, int> counter;
        WorkerThreadPool pool(router, [&](To& bar) { 
            
            auto tid = std::this_thread::get_id();
            if (auto result = counter.find(tid); result != counter.end())
                result->second += bar.value_processed;
            else 
                counter.insert({ tid, bar.value_processed });

        });

        // First work batch 
        for(int i = 0; i < 10000; i++)
            router->insert({ 1 });
        
        pool.busy();
        REQUIRE(router->size() == 0);
        
        // Second work batch
        for(int i = 0; i < 10000; i++)
            router->insert({ 1 });

        pool.busy();
        REQUIRE(router->size() == 0);

        SECTION("Work completition") 
        {
            int sum = 0;
            for (auto& elem : counter)
                sum += elem.second;

            REQUIRE(sum == 20000);
        }
    }
}
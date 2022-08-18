#include <mt/thread_pool.hpp>

#include "catch2/catch.hpp"
using namespace fur::mt;

// The custom data to be served to workers by the router
struct CustomData {
    int a, b, c;
};

// Work unit used in this test
const int WORK_COUNT = 100;

// Custom router used by the workers to find work
class CustomDataRouter : public DataRouter<CustomData> {

    int work_count = WORK_COUNT;

public:
    bool work_is_available() override {
        return work_count != 0;
    }

    CustomData get_work() override {
        CustomData res { work_count, work_count, work_count };
        work_count -= 1;
        return res;
    }
};

// Counter used by the test as for the correct result condition
int results[WORK_COUNT] = { 0 };

// Function used by the workers threads
void worker_fn(CustomData& data) {
    results[data.a - 1] = data.b;
}

TEST_CASE("[mt] Thread Pool Creation") {

    {
        WorkerThreadPool<CustomData> pool(
            std::make_unique<CustomDataRouter>(), 
            worker_fn, 
            4
        );

        while(pool.busy());
    }

    int counter = 0;
    for (int i = 0; i < WORK_COUNT; i++) {
        REQUIRE(results[i] != 0);
        counter += i;
    }

    REQUIRE(counter == WORK_COUNT * (WORK_COUNT - 1) / 2);
}
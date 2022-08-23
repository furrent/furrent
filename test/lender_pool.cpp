#include "download/lender_pool.hpp"

#include <thread>

#include "catch2/catch.hpp"

class NonCopyable {
 private:
  int value;

 public:
  explicit NonCopyable(int value) : value{value} {}

  // Successfully being able to call this function somewhat proves that you
  // got a reference to it from the LenderPool
  int proof_of_access() { return value; }

  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
  NonCopyable(NonCopyable&&) noexcept = default;
  NonCopyable& operator=(NonCopyable&&) noexcept = default;
};

TEST_CASE("[LenderPool] Basic") {
  LenderPool<NonCopyable> pool;
  pool.put(NonCopyable(27));

  auto t1 = std::thread([&] {
    auto borrow = pool.get();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(borrow->proof_of_access() == 27);
  });
  auto t2 = std::thread([&] {
    auto borrow = pool.get();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(borrow->proof_of_access() == 27);
  });

  t1.join();
  t2.join();
}

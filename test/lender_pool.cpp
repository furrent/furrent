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
    REQUIRE(borrow->proof_of_access() == 27);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(borrow->proof_of_access() == 27);
  });
  auto t2 = std::thread([&] {
    auto borrow = pool.get();
    REQUIRE(borrow->proof_of_access() == 27);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(borrow->proof_of_access() == 27);
  });

  t1.join();
  t2.join();
}

TEST_CASE("[LenderPool] Moving backing vector") {
  // This test case demonstrates that the destruction of a borrow keeps working
  // even after the contents of the pool are moved due to the backing vector
  // being reallocated.

  LenderPool<NonCopyable> pool;
  pool.put(NonCopyable(27));
  pool.put(NonCopyable(39));

  auto t1 = std::thread([&] {
    auto borrow = pool.get();
    REQUIRE(borrow->proof_of_access() == 27);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(borrow->proof_of_access() == 27);
  });
  auto t2 = std::thread([&] {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    auto borrow1 = pool.get();
    auto borrow2 = pool.get();
    REQUIRE(borrow1->proof_of_access() == 27);
    REQUIRE(borrow2->proof_of_access() == 39);
  });

  t1.join();
  t2.join();
}

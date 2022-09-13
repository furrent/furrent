#include <catch2/catch.hpp>
#include <policy/queue.hpp>

using namespace fur::policy;

class Movable {
 public:
  int value = 0;

  Movable(int x) : value{x} {}

  Movable(const Movable&) = delete;
  Movable& operator=(const Movable&) = delete;

  Movable(Movable&& o) {
    value = o.value;
    o.value = 0;
  }

  Movable& operator=(Movable&& o) {
    value = o.value;
    o.value = 0;
    return *this;
  }
};

TEST_CASE("Queue") {
  Queue<Movable> queue;

  // Accepts movable values
  Movable a = {5};
  queue.insert(std::move(a));

  FIFOPolicy<Movable> policy;
  auto present = queue.extract(policy);
  auto nothing = queue.extract(policy);
  REQUIRE(queue.size() == 0);

  REQUIRE(present.valid());
  REQUIRE(present->value == 5);

  REQUIRE(!nothing.valid());
  REQUIRE(nothing.error() == Queue<Movable>::Error::Empty);
}
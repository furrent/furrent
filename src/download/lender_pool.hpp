#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <vector>

template <typename T>
class LenderPool {
 private:
  struct Slot {
    explicit Slot(T inner) : inner{std::move(inner)}, is_borrowed{false} {}

    T inner;
    bool is_borrowed;
  };

 public:
  struct Borrow {
    T& operator*() { return inner; }
    T* operator->() { return &inner; }

    explicit Borrow(T& inner, std::function<void()> onRelease)
        : inner{inner}, onRelease{std::move(onRelease)} {}

    ~Borrow() { onRelease(); }

   private:
    T& inner;
    std::function<void()> onRelease;
  };

 private:
  std::condition_variable cv;
  std::mutex mx;
  std::vector<Slot> storage;

 public:



  void put(T inner) {
    std::unique_lock lock(mx);
    storage.push_back(Slot(std::move(inner)));
  }

  Borrow get() {
    std::unique_lock lock(mx);

    while (true) {
      for (auto& slot : storage) {
        if (!slot.is_borrowed) {
          slot.is_borrowed = true;
          return Borrow(slot.inner, [&] {
            std::unique_lock lock(mx);
            slot.is_borrowed = false;
            cv.notify_one();
          });
        }
      }

      // Too bad, there is no slot available to lend. Let's wait until something
      // happens.
      cv.wait(lock);
    }
  }
};

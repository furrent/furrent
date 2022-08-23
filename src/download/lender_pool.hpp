#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

template <typename T>
struct Slot {
  explicit Slot(T inner) : inner{std::move(inner)}, is_borrowed{false} {}

  T inner;
  bool is_borrowed;
};

template <typename T>
using SlotPtr = std::unique_ptr<Slot<T>>;

template <typename T>
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

template <typename T>
struct StratFirstAvailable {
  SlotPtr<T>* operator()(std::vector<SlotPtr<T>>& storage);
};

template <typename T, typename Strategy = StratFirstAvailable<T>>
class LenderPool {
 private:
  std::condition_variable cv;
  std::mutex mx;
  std::vector<SlotPtr<T>> storage;

 public:
  void put(T inner) {
    std::unique_lock lock(mx);
    storage.push_back(std::make_unique<Slot<T>>(std::move(inner)));
  }

  Borrow<T> get() {
    Strategy strategy;

    std::unique_lock lock(mx);

    while (true) {
      SlotPtr<T>* maybeSlot = strategy(storage);

      if (maybeSlot != nullptr) {
        auto& slot = *(*maybeSlot).get();
        slot.is_borrowed = true;
        return Borrow(slot.inner, [&] {
          std::unique_lock lock(mx);
          slot.is_borrowed = false;
          cv.notify_one();
        });
      }

      // Too bad, there is no slot available to lend. Let's wait until something
      // happens.
      cv.wait(lock);
    }
  }
};

template <typename T>
SlotPtr<T>* StratFirstAvailable<T>::operator()(
    std::vector<SlotPtr<T>>& storage) {
  for (auto& slotPtr : storage) {
    if (!slotPtr->is_borrowed) {
      return &slotPtr;
    }
  }
  return nullptr;
}

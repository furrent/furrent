#pragma once

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

namespace fur::download::lender_pool {
/// Used by the `LenderPool` to store a generic object of type `T` while
/// tracking whether or not it is currently borrowed.
template <typename T>
struct Slot {
  explicit Slot(T inner) : inner{std::move(inner)}, is_borrowed{false} {}

  T inner;
  bool is_borrowed;
};

/// Alias type an `std::unique_ptr`, for brevity.
template <typename T>
using SlotPtr = std::unique_ptr<Slot<T>>;

/// A handle that is given by the `LenderPool` to borrowers. It allows accessing
/// the underlying object. When destructed, it automatically notifies the pool
/// that the object can be used again.
/// \tparam T The type of the underlying object being borrowed
template <typename T>
struct Borrow {
  T& operator*() { return inner; }
  T* operator->() { return &inner; }

  explicit Borrow(T& inner, std::function<void()> onRelease)
      : inner{inner}, onRelease{std::move(onRelease)} {}

  ~Borrow() { onRelease(); }

 private:
  /// A reference to the underlying object. This is a reference because the
  /// actual objects are always owned by the `LenderPool`.
  T& inner;
  /// What to do when this `Borrow` goes out of scope. Deals with marking the
  /// underlying object as available-for-use in a thread safe way.
  std::function<void()> onRelease;
};

/// A simple strategy for picking an object when a borrow is requested to a
/// `LenderPool`.
template <typename T>
struct StratFirstAvailable {
  SlotPtr<T>* operator()(std::vector<SlotPtr<T>>& storage);
};

/// Collection of objects that can be borrowed, used and then returned. The pool
/// is owning the objects at all times, only references are given out.
/// \tparam T The type of the underlying object being lent out to borrowers
/// \tparam Strategy The strategy used to pick an object to lend out
template <typename T, typename Strategy = StratFirstAvailable<T>>
class LenderPool {
 private:
  /// Used to notify any thread sleeping on `get()` that an object has been
  /// returned or that a pool is no longer clearing.
  std::condition_variable cv;
  /// Protects the `storage` vector and also `draining`.
  std::mutex mx;
  std::vector<SlotPtr<T>> storage;

  /// Is the bool being drained?
  bool draining = false;

 public:
  /// Inserts a new object into the pool.
  ///
  /// Note that the `storage` object could be resize, thus moving the elements
  /// memory. This, however, causes no harm as all `Slot`s are pinned in memory
  /// by using `std::unique_ptr`.
  void put(T inner) {
    std::unique_lock lock(mx);
    if (draining) return;
    storage.push_back(std::make_unique<Slot<T>>(std::move(inner)));
    cv.notify_one();
  }

  /// Borrow an object from the pool. This is blocking if no suitable objects
  /// are available at the time of calling.
  Borrow<T> get() {
    // Create an instance of the picking strategy, which is a struct.
    Strategy strategy;

    std::unique_lock lock(mx);

    while (true) {
      if (!draining) {
        // We're using pointers here but for a good cause: the strategy cannot
        // always return a slot (think about the scenario in which all objects
        // are currently borrowed), so a `nullptr` value is used to represent this specific failure. `std::optional` cannot be used for reference types. But then: how is a `std::optional<T&>` any different from `T*`?
        SlotPtr<T>* maybeSlot = strategy(storage);

        if (maybeSlot != nullptr) {
          // This bad looking line is simply to convert to a `Slot<T>&`.
          auto& slot = *(*maybeSlot).get();
          slot.is_borrowed = true;
          return Borrow(slot.inner, [&] {
            std::unique_lock lock(mx);
            slot.is_borrowed = false;
            // This object has been released, notify any waiting thread.
            cv.notify_all();
          });
        }
      }

      // Too bad, there is no slot available to lend or the pool is draining.
      // Let's wait until something happens.
      cv.wait(lock);
    }
  }

  /// Block until the entire pool is drained
  void drain() {
    std::unique_lock lock(mx);
    draining = true;

    while (true) {
      for (int i = 0; i < static_cast<int>(storage.size()); i++) {
        if (!storage[i]->is_borrowed) {
          storage.erase(storage.begin() + i);
          i--;
        }
      }

      if (storage.empty()) break;
      cv.wait(lock);
    }

    draining = false;
    cv.notify_all();
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
}  // namespace fur::download::lender_pool

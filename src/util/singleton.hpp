#pragma once

namespace fur::util {

template <typename T>
class Singleton {
 public:
  /// Returns the instance of the singleton
  static T& instance();

 public:
  Singleton() = default;

  // Don't allow copy and move constructors/assignments!
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;
  Singleton(Singleton&&) noexcept = delete;
  Singleton& operator=(Singleton&&) noexcept = delete;
};

}  // namespace fur::util

#include <util/singleton.inl>
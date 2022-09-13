#include <policy/policy.hpp>

namespace fur::policy {

template <typename T>
auto LIFOPolicy<T>::extract(Iterator begin, Iterator end) const -> Iterator {
  return begin;
}

template <typename T>
auto FIFOPolicy<T>::extract(Iterator begin, Iterator end) const -> Iterator {
  return --end;
}

}  // namespace fur::policy
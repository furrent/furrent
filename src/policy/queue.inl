#include <policy/queue.hpp>

namespace fur::policy {

template <typename T>
void Queue<T>::insert(T&& item) {
  _items.push_back(std::forward<T>(item));
}


template <typename T>
std::list<T>& Queue<T>::items() {
  return _items;
}

template <typename T>
auto Queue<T>::extract(const IPolicy<T>& policy) -> Result {
  if (_items.empty()) return Result::ERROR(Error::Empty);

  auto it = policy.extract(_items.begin(), _items.end());
  if (it == _items.end()) return Result::ERROR(Error::PolicyFailure);

  auto result = std::move(*it);
  _items.erase(it);

  return Result::OK(std::move(result));
}

template<typename T>
void Queue<T>::mutate(MutateFn mutation) {
  auto it = _items.begin();
  while (it != _items.end()) {

    bool keep = mutation(*it);
    if (!keep) 
      it = _items.erase(it);
    else 
      ++it;
  }
}

template <typename T>
size_t Queue<T>::size() const {
  return _items.size();
}

}  // namespace fur::policy
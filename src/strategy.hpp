#include <vector>

/// Strategy for picking an item from a collection.
/// \tparam To The type of the picked item.
/// \tparam C The type of the collection.
template <typename To, typename C>
class IPickStrategy {
 public:
  virtual ~IPickStrategy() = default;
  /// Attempts to pick an item from the collection. Can return a null option
  /// to indicate an unsuccessful picking.
  /// \attention This function must be thread safe.
  virtual std::optional<To> operator()(C& collection) = 0;
};

/// Strategy for picking an item from a vector.
/// \tparam From The type of elements in the vector.
/// \tparam To The type of the picked item.
template <typename From, typename To>
using IVectorPickStrategy = IPickStrategy<To, std::vector<From>>;

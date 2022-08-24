#pragma once

#include <mt/router.hpp>
#include <torrent_manager.hpp>

namespace fur::strategy {

/// Uses a uniform probability distribution to choose the next work-item,
/// applies a custom transformation
template<typename From, typename To>
class UniformStrategy : public mt::IVectorStrategy<From, To> {

  const bool m_should_erase;

  public:
    explicit UniformStrategy(bool should_erase);
    std::optional<To> operator() (std::vector<From>& items) override;
    virtual std::optional<To> transform(From&) = 0;
};

/// Uses a uniform probability distribution to choose the next work-item,
/// doesn't apply a custom transformation
template<typename T>
class IdentityUniformStrategy : public UniformStrategy<T, T> {
 public:
  explicit IdentityUniformStrategy(bool should_erase)
      : UniformStrategy<T, T>(should_erase) { }

  std::optional<T> transform(T& other) { return { other }; }
};

} // namespace fur::strategy

#include <strategies/uniform.inl>

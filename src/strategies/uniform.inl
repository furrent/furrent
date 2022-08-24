//
// Created by nicof on 23/08/22.
//

#include <random>

namespace fur::strategy {

  template<typename From, typename To>
UniformStrategy<From, To>::UniformStrategy(bool should_erase)
    : m_should_erase{should_erase} {

    srand(time(0));
  }

  template<typename From, typename To>
  std::optional<To> UniformStrategy<From, To>::operator() (std::vector<From>& items){
    if (!items.empty()) {

      int index = rand() % items.size();
      std::optional<To> result = transform(items.at(index));
      if (result) {
        if (m_should_erase)
          items.erase(items.begin() + index);

        return std::optional<To>{ result };
      }
    }
    return std::nullopt;
  }

}
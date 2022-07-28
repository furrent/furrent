//
// Created by Z1ko on 28/07/2022.
//

#ifndef FURRENT_ROUTER_HPP
#define FURRENT_ROUTER_HPP

namespace fur::mt {

/// Allows workers to find work to do using different strategies.
/// Only one thread is allowed to use it at any same time.
/// \tparam T Type of data routed
template<typename T>
class router {
 public:
  virtual ~router() = default;

  /// Returns True if there is work to be done
  virtual bool work_is_available() = 0;
  /// Retrieve a piece of independent work
  virtual T get_work() = 0;
};

}

#endif  // FURRENT_ROUTER_HPP

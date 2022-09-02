/**
 * @file result.hpp
 * @author Filippo Ziche
 * @version 0.1
 * @date 2022-09-02
 */

#pragma once

#include <variant>

namespace fur::util {

/// @brief Rappresent a result of a function, and an error if the result is unavailable
/// @tparam R Type of the result
/// @tparam E Type of the error
template<typename R, typename E>
class Result {

    std::variant<R, E> _variant;

    /// @brief Create a new result from a variant
    Result(std::variant<R, E> variant);

public:
    /// @return True if there was an error 
    bool has_error() const;

    /// @return Result of the operation
    R get_value();

    /// @return Error occured during operation
    E get_error();

public:
    static Result ok(R result);
    static Result error(E error);

};

} // namespace fur::util

#include <util/result.inl>
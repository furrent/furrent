/**
 * @file result.hpp
 * @author Filippo Ziche
 * @version 0.1
 * @date 2022-09-02
 */

#pragma once

#include <variant>

namespace fur::util {

/// @brief Used to handle errors without using exceptions
/// @tparam R type of the result
/// @tparam E type of the error
template<typename R, typename E>
class Result {

    std::variant<R, E> _inner;

    Result(R&& result);
    Result(E&& error);

public:

    Result(const Result&) = delete;
    Result& operator=(const Result&) = delete;
    
    Result(Result&&) noexcept;
    Result& operator=(Result&&) noexcept;

    /// @return True if there is no error. 
    operator bool() const;
    /// @return Result if it is present, undefined behaviour otherwise
    R& operator *();

    /// @return Result if it is present, undefined behaviour otherwise
    R* operator ->();
    /// @return error that occurred 
    const E& error() const;

public:
    /// Creates an ok result
    static Result OK(R&& result);
    /// Creates an error result 
    static Result ERROR(E&& error);
};

} // namespace fur::util

#include <util/result.inl>
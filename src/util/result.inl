/**
 * @file result.inl
 * @author Filippo Ziche
 * @version 0.1
 * @date 2022-09-02
 */

#include <util/result.hpp>

namespace fur::util {

template<typename R, typename E>
Result<R, E>::Result(std::variant<R, E> variant)
: _variant{std::move(variant)} { }

template<typename R, typename E>
bool Result<R, E>::has_error() const {
    return std::holds_alternative<E>(_variant);
}

template<typename R, typename E>
R Result<R, E>::get_value() {
    return std::get<R>(_variant);
}

template<typename R, typename E>
E Result<R, E>::get_error() {
    return std::get<E>(_variant);
}

template<typename R, typename E>
Result<R, E> Result<R, E>::ok(R result) {
    return Result{std::variant<R, E>{result}};
}

template<typename R, typename E>
Result<R, E> Result<R, E>::error(E error) {
    return Result{std::variant<R, E>{error}};
}

} // namespace fur::util
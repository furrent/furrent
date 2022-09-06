/**
 * @file result.inl
 * @author Filippo Ziche
 * @version 0.1
 * @date 2022-09-02
 */

#include <util/result.hpp>

namespace fur::util {

template<typename R>
Result<R>::Result(R&& result)
: _inner{std::forward<R>(result)} { }

template<typename R>
Result<R>::Result(Error&& error)
: _inner{std::forward<Error>(error)} { }

template<typename R>
Result<R>::Result(Result&& o) noexcept {
    std::swap(_inner, o._inner);
}

template<typename R>
Result<R>& Result<R>::operator=(Result&& o) noexcept {
    std::swap(_inner, o._inner);
    return *this;
}

template<typename R>
auto Result<R>::OK(R&& result) -> Result {
    return Result(std::forward<R>(result));
}

template<typename R>
auto Result<R>::ERROR(Error&& error) -> Result {
    return Result(std::forward<Error>(error));
}

template<typename R>
Result<R>::operator bool() const {
    return std::holds_alternative<R>(_inner);
}

template<typename R>
R& Result<R>::operator *() {
    return std::get<R>(_inner);
}

template<typename R>
R* Result<R>::operator ->() {
    return &std::get<R>(_inner);
}

template<typename R>
const Error& Result<R>::error() const{
    return std::get<Error>(_inner);
}

} // namespace fur::util
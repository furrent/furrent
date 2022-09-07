/**
 * @file result.inl
 * @author Filippo Ziche
 * @version 0.1
 * @date 2022-09-02
 */

#include <util/result.hpp>

namespace fur::util {

template<typename R, typename E>
Result<R, E>::Result(R&& result)
: _inner{std::forward<R>(result)} { }

template<typename R, typename E>
Result<R, E>::Result(E&& error)
: _inner{std::forward<E>(error)} { }

template<typename R, typename E>
Result<R, E>::Result(Result&& o) noexcept {
    std::swap(_inner, o._inner);
}

template<typename R, typename E>
Result<R, E>& Result<R, E>::operator=(Result&& o) noexcept {
    std::swap(_inner, o._inner);
    return *this;
}

template<typename R, typename E>
auto Result<R, E>::OK(R&& result) -> Result {
    return Result(std::forward<R>(result));
}

template<typename R, typename E>
auto Result<R, E>::ERROR(E&& error) -> Result {
    return Result(std::forward<E>(error));
}

template<typename R, typename E>
bool Result<R, E>::valid() const {
    return std::holds_alternative<R>(_inner);
}

template<typename R, typename E>
R& Result<R, E>::operator *() {
    return std::get<R>(_inner);
}

template<typename R, typename E>
R* Result<R, E>::operator ->() {
    return &std::get<R>(_inner);
}

template<typename R, typename E>
const E& Result<R, E>::error() const {
    return std::get<E>(_inner);
}

} // namespace fur::util
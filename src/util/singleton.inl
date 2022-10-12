#include <util/singleton.hpp>

namespace fur::util {
    
template<typename T>
T& Singleton<T>::instance() {
    static T instance;
    return instance; 
}

} // namespace fur::util

/**
 * @file policy.hpp
 * @author Filippo Ziche (filippo.ziche@gmail.com)
 * @brief Policies are used to prioritize execution of tasks
 * @version 0.1
 * @date 2022-09-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <list>

namespace fur::policy {

template<typename T>
class IPolicy {
public:
    /// Iterator type used by the std::list type
    typedef typename std::list<T>::iterator Iterator;

    /// @brief implements the custom policy logic to extract an element from the list
    /// @param begin begin of the list
    /// @param end end of the list
    /// @return position of the element to extract
    virtual Iterator extract(Iterator begin, Iterator end) const = 0;
};

/// @brief Simple FIFO policy 
template<typename T>
class FIFOPolicy : public IPolicy<T> {
public:
    using typename IPolicy<T>::Iterator; 
    Iterator extract(Iterator begin, Iterator end) const override;
};

/// @brief Simple LIFO policy
template<typename T>
class LIFOPolicy : public IPolicy<T> {
public:
    using typename IPolicy<T>::Iterator;
    Iterator extract(Iterator begin, Iterator end) const override;
};

} // namespace fur::policy

#include <policy/policy.inl>
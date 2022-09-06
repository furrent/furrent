/**
 * @file queue.hpp
 * @author Filippo Ziche (filippo.ziche@gmail.com)
 * @version 0.1
 * @date 2022-09-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <optional>

#include <policy/policy.hpp>
#include <util/result.hpp>

namespace fur::policy {

/// @brief Special queue that allows the extraction of item using custom policies
/// @tparam T type of the stored items
template<typename T>
class Queue {

    /// Stored items
    std::list<T> _items;

public:
    /// Custom queue result type
    typedef util::Result<T> Result;

public:
    /// Insert a new element
    /// @param item element to be inserted
    void insert(T&& item);

    /// Tries to extract an element using a policy
    /// @param policy custom logic used to select the element
    /// @return nullopt it he policy failed, T otherwise
    [[nodiscard]] Result extract(const IPolicy<T>& policy);

    /// @return Number of items present
    size_t size() const;
};

} // namespace fur::policy

#include <policy/queue.inl>
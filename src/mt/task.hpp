/**
 * @file task.hpp
 * @author Filippo Ziche (filippo.ziche@gmail.com)
 * @brief Tasks are the basic unit of execution for the workers
 * @version 0.1
 * @date 2022-09-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <memory>

#include <mt/sharing_queue.hpp>
#include <policy/policy.hpp>

namespace fur::mt {

class ITask {
public:
    /// Standard wrapper type of the tasks
    typedef std::unique_ptr<ITask> Wrapper;

    /// Implements task custom logic
    /// @param queue queue where the task can append generated tasks
    virtual void execute(SharingQueue<Wrapper>& queue) = 0;

    /// @return priority of the task, used for scheduling 
    virtual size_t priority() const = 0;
};

/// Policy for extracing highest priority tasks first
class PriorityPolicy : public policy::IPolicy<ITask::Wrapper> {
public:
    Iterator extract(Iterator begin, Iterator end) const override;
};

} // namespace fur::mt
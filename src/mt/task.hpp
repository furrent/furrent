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

namespace fur::mt {

class ITask {
public:
    /// Standard wrapper type of the tasks
    typedef std::unique_ptr<ITask> TaskWrapper;

    /// Implements task custom logic
    /// @param queue queue where the task can append generated tasks
    virtual void operator()(SharingQueue<TaskWrapper>& queue) = 0;
};

} // namespace fur::mt
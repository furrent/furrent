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

/// All accepted piority levels:
/// levels from low to high are considered active
/// level none will not be considered
enum Priority : size_t {
  PRIORITY_HIGH = 30000,
  PRIORITY_MEDIUM = 20000,
  PRIORITY_LOW = 10000,
};

enum TaskState {
  /// Task can be executed
  Running,
  /// Task is present in queue but should not be executed
  Paused,
};

class ITask {
 public:
  /// Standard wrapper type of the tasks
  typedef std::unique_ptr<ITask> Wrapper;

  /// State of the current state
  TaskState state{};

 private:
  /// Queue where to spawn new tasks
  SharedQueue<Wrapper>* _spawn_queue{};

 public:
  ITask() = default;
  explicit ITask(SharedQueue<Wrapper>* spawn_queue);
  virtual ~ITask() = default;

  /// Implements task custom logic
  virtual void execute() = 0;

  /// @return priority of the task, used for scheduling
  [[nodiscard]] virtual size_t priority() const = 0;

 protected:
  /// Set the queue where to spawn new tasks
  void set_spawn_queue(SharedQueue<Wrapper>* spawn_queue);
  /// Spawn a new task on the queue where this task was extracted from
  void spawn(Wrapper&& task);
};

}  // namespace fur::mt
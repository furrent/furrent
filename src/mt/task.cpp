#include <mt/task.hpp>

namespace fur::mt {

ITask::ITask(mt::SharedQueue<Wrapper>* spawn_queue)
  : state{TaskState::Running}, _spawn_queue{spawn_queue} {} 

void ITask::set_spawn_queue(SharedQueue<Wrapper>* spawn_queue) {
  _spawn_queue = spawn_queue;
}

void ITask::spawn(Wrapper&& task) {
  task->set_spawn_queue(_spawn_queue);
  _spawn_queue->insert(std::move(task));
}

}  // namespace fur::mt

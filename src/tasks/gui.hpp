#pragma once

#include <furrent.hpp>
#include <mt/task.hpp>

namespace fur::tasks {

/// Task responsible for the rendering of the ui
class GuiTask : public mt::ITask {
 public:
  explicit GuiTask(Furrent& furrent);
};

}  // namespace fur::tasks

#include <mt/group.hpp>

namespace fur::mt {

Controller::Controller(std::mutex& mutex, bool& should_terminate)
: _mutex{mutex}, _should_terminate{should_terminate} { }

bool Controller::alive() {
    std::scoped_lock<std::mutex> lock(_mutex);
    return !_should_terminate;
}

} // namespace fur::mt
/**
 * @file group.hpp
 * @author Filippo Ziche
 * @version 0.1
 * @date 2022-08-26
 */

#pragma once

#include <mutex>
#include <vector>
#include <functional>
#include <thread>

namespace fur::mt {

/// Used to control threads execution
class Runner {
    
    /// Protects terminate status
    std::mutex& _mutex;
    /// True if the threads need to be joined
    bool& _should_terminate;
    
public:
    Runner(std::mutex& mutex, bool& _should_terminate);

    /// True if threads should continue executing
    bool alive();
};

/// Manages the execution of a group of threads
template<typename State>
class ThreadGroup {

    /// Functions executed by the threads, as an argument
    /// each threads receives the runner, its own unique state ad its thread number
    typedef std::function<void(Runner, State&, size_t)> ThreadFn;

    /// Contains all threads handles managed by the group
    std::vector<std::thread> _threads;
    /// Contains all threads states
    std::vector<State> _states;
    /// Function running on workers
    ThreadFn _thread_fn;
    /// Protects terminate status
    std::mutex _mutex;
    /// True if the threads need to be joined
    bool _should_terminate;

public:
    /// Create disabled thread group
    ThreadGroup();
    /// Stops and joins all threads
    virtual ~ThreadGroup();

    //===========================================================================
    // This object is not copyable and not movable because of the mutex

    ThreadGroup(ThreadGroup&) = delete;
    ThreadGroup& operator= (ThreadGroup&) = delete;
    ThreadGroup(ThreadGroup&&) noexcept = delete;
    ThreadGroup& operator= (ThreadGroup&&) noexcept = delete;

    //===========================================================================

    /// Create threads and begin execution
    void launch(ThreadFn fn, size_t max_worker_threads = 0);

    /// Terminate thread execution, this operation is irrecuperable
    void terminate();

    /// Obtain thread state
    State& get_thread_state(size_t thread);

    /// Get total number of workers
    size_t get_worker_count() const;

private:
    /// Workers thread main function
    void thread_main(size_t index);

};

} // namespace fur::mt

#include <mt/group.inl>
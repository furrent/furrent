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
class Controller {
    
    /// Protects terminate status
    std::mutex& _mutex;
    /// True if the threads need to be joined
    bool& _should_terminate;
    
public:
    Controller(std::mutex& mutex, bool& _should_terminate);

    /// True if threads should continue executing
    bool alive();
};

/// Manages the execution of a group of threads
template<typename State>
class ThreadGroup {

    /// Functions executed by the threads, as an argument
    /// each threads receives the controller, its own unique state ad its thread number
    typedef std::function<void(Controller, State&, size_t)> ThreadFn;

    /// Internal rappresentation of a thread
    struct Thread 
    {
        /// OS handle
        std::thread handle;
        /// User-defined state
        State state;
    };

    /// Contains all threads managed by the group
    std::vector<Thread> _threads;
    /// Function executing on all threads in a loop
    ThreadFn _thread_fn;
    /// Protects terminate status
    std::mutex _mutex;
    /// True if the threads need to be joined
    bool _should_terminate;

public:
    /// Starts the group of threads
    ThreadGroup(ThreadFn fn, size_t max_worker_threads = 0);
    /// Stops and joins all threads
    virtual ~ThreadGroup();

    //===========================================================================
    // This object is not copyable and not movable because of the mutex

    ThreadGroup(ThreadGroup&) = delete;
    ThreadGroup& operator= (ThreadGroup&) = delete;
    ThreadGroup(ThreadGroup&&) noexcept = delete;
    ThreadGroup& operator= (ThreadGroup&&) noexcept = delete;

    //===========================================================================

    /// Terminate thread execution, this operation is irrecuperable
    void terminate();

    /// Obtain thread state
    State& get_thread_state(size_t thread);

private:
    /// Threads main function
    void thread(size_t index);
};

} // namespace fur::mt

#include <mt/group.inl>
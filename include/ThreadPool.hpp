#ifndef _THREADPOOL
#define _THREADPOOL
#include "Task.hpp"
#include "ThreadSafeQueue.hpp"
#include <algorithm>
#include <atomic>
#include <functional>
#include <thread>
#include <vector>

namespace jz
{
enum class pool_state
{
    closed,
    running,
    paused,
    waitting
};

class thread_pool
{
public:
    using _Task       = std::function< void() >;
    using _Task_Queue = thread_safe_queue< _Task >;

public:
    thread_pool( const thread_pool& )            = delete;
    thread_pool( thread_pool&& )                 = delete;
    thread_pool& operator=( const thread_pool& ) = delete;
    thread_pool& operator=( thread_pool&& )      = delete;

public:
    explicit thread_pool( const size_t _Count = std::thread::hardware_concurrency() - 1 ) : _Thread_Count( _Count ) {
        _Threads.resize( _Count );
        create_threads( _Thread_Count );
        _Manager = std::thread( &thread_pool::manage_func, this );
        _Manager.detach();
    }

    ~thread_pool() {
        wait_util_done();
        destroy_pool();
    }

    void wait_util_done() {
        if ( is_closed() || is_paused() ) {
            return;
        }
        std::unique_lock< std::mutex > _Ulck( _Mutex );
        _State = pool_state::waitting;
        _Done_Cond.wait( _Ulck, [ this ]() {
            // std::cout << "wait_util_done" << std::endl;
            return _Tasks.empty();
        } );
        // std::cout << "done" << std::endl;
    }
    void destroy_pool() {
        _State = pool_state::closed;
        _Cond.notify_all();
        _Tasks.get_cond().notify_all();
        std::for_each( _Threads.begin(), _Threads.end(), []( std::thread& _Td ) {
            if ( _Td.joinable() ) {
                _Td.join();
            }
        } );
        // std::cout << "destroy ok" << std::endl;
    }

    template < typename _Fn, typename... Args >
    decltype( auto ) add_task( _Fn&& _Func, Args... args ) {
        using _Ret_Type = typename std::result_of< _Fn( Args... ) >::type;

        auto _F = std::bind( std::forward< _Fn >( _Func ), std::forward< Args >( args )... );
        auto _T = std::make_shared< std::packaged_task< _Ret_Type() > >( _F );

        typename _Task_Queue::value_type _TT( [ = ]() { ( *_T )(); } );
        _Tasks.push( std::move( _TT ) );
        _Cond.notify_one();
        return _T->get_future();
    }

    inline void resume() {
        _State = pool_state::running;
        _Cond.notify_all();
    }

    inline bool is_closed() const noexcept {
        return _State == pool_state::closed;
    }

    inline bool is_paused() const noexcept {
        return _State == pool_state::paused;
    }

    inline void pause() {
        _State = pool_state::paused;
    }

    inline bool is_waitting() const noexcept {
        return _State == pool_state::waitting;
    }

private:
    inline void create_threads( const size_t n ) {
        _State = pool_state::running;
        std::transform( _Threads.begin(), _Threads.end(), _Threads.begin(), [ this ]( std::thread& _Td ) { return std::thread( &thread_pool::work_func, this ); } );
    }

    void work_func() {
        for ( ;; ) {
            std::unique_lock< std::mutex > _Ulck( _Mutex );
            _Cond.wait( _Ulck, [ this ]() { return ( is_closed() || !_Tasks.empty() ) && !is_paused(); } );
            if ( is_closed() && _Tasks.empty() ) {
                // std::cout << "thread id :" << std::this_thread::get_id() << "break loop" << std::endl;
                break;
            }

            auto task = _Tasks.pop();
            _Ulck.unlock();
            task();

            if ( _State == pool_state::waitting && _Tasks.empty() ) {
                _Done_Cond.notify_all();
            }
        }
    }

    void manage_func() {
        for ( ;; ) {
            std::cout << "i am manager thread" << std::endl;
            std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
        }
    }

private:
    // safe queue that stores the tasks
    _Task_Queue _Tasks;

    // the number of threads
    size_t _Thread_Count;

    // store work-threads
    std::vector< std::thread > _Threads;

    // a thread that manages work-threads
    std::thread _Manager;

    // record the state of the thread_pool
    std::atomic< pool_state > _State{ pool_state::closed };

    std::condition_variable _Cond;

    std::condition_variable _Done_Cond;

    std::mutex _Mutex;
};
}  // namespace jz
#endif
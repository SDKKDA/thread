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
    using task       = std::function< void() >;
    using task_queue = thread_safe_queue< task >;

public:
    thread_pool( const thread_pool& )            = delete;
    thread_pool( thread_pool&& )                 = delete;
    thread_pool& operator=( const thread_pool& ) = delete;
    thread_pool& operator=( thread_pool&& )      = delete;

public:
    explicit thread_pool( const size_t threads_count = std::thread::hardware_concurrency() - 1 ) : threads_count_( threads_count ) {
        threads_.resize( threads_count );
        create_threads( threads_count_ );
    }

    ~thread_pool() {
        wait_util_done();
        destroy_pool();
    }

    void wait_util_done() {
        if ( is_closed() || is_paused() ) {
            return;
        }
        std::unique_lock< std::mutex > ul( mutex_ );
        state_ = pool_state::waitting;
        done_cond_.wait( ul, [ this ]() { return tasks_.empty(); } );
    }
    void destroy_pool() {
        cond_.notify_all();
        tasks_.get_cond().notify_all();
        std::for_each( threads_.begin(), threads_.end(), []( std::thread& t ) {
            if ( t.joinable() ) {
                t.join();
            }
        } );
    }

    template < typename Func, typename... Args >
    decltype( auto ) add_task( Func&& t, Args... args ) {
        using retType = typename std::result_of< Func( Args... ) >::type;

        auto f    = std::bind( std::forward< Func >( t ), std::forward< Args >( args )... );
        auto task = std::make_shared< std::packaged_task< retType() > >( f );

        typename task_queue::value_type tt( [ = ]() { ( *task )(); } );
        tasks_.push( std::move( tt ) );
        return task->get_future();
    }

    inline void run() {
        state_ = pool_state::running;
        cond_.notify_all();
    }

    inline bool is_closed() const noexcept {
        return state_ == pool_state::closed;
    }
    inline void close() {
        state_ = pool_state::closed;
    }

    inline bool is_paused() const noexcept {
        return state_ == pool_state::paused;
    }

    inline void pause() {
        state_ = pool_state::paused;
    }

    inline bool is_waitting() const noexcept {
        return state_ == pool_state::waitting;
    }

private:
    inline void create_threads( const size_t n ) {
        std::transform( threads_.begin(), threads_.end(), threads_.begin(), [ this ]( std::thread& t ) {
            t = std::thread( &thread_pool::work_func, this );
            return std::move( t );
        } );
    }

    void work_func() {
        for ( ;; ) {
            std::unique_lock< std::mutex > ul( mutex_ );
            cond_.wait( ul, [ this ]() { return !is_paused(); } );
            if ( is_closed() ) {
                break;
            }

            auto task = tasks_.pop();
            task();
            if ( state_ == pool_state::waitting && tasks_.empty() ) {
                done_cond_.notify_all();
            }
        }
    }

private:
    // safe queue that stores the tasks
    task_queue tasks_;

    // the number of threads
    size_t threads_count_;

    // store work-threads
    std::vector< std::thread > threads_;

    // a thread that manages work-threads
    std::thread manager_;

    // record the state of the thread_pool
    std::atomic< pool_state > state_{ pool_state::closed };

    std::condition_variable cond_;

    std::condition_variable done_cond_;

    std::mutex mutex_;
};
}  // namespace jz
#endif
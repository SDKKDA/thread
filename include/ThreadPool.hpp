#ifndef _THREADPOOL
#define _THREADPOOL
#include "Task.hpp"
#include "ThreadSafeQueue.hpp"
#include <algorithm>
#include <functional>
#include <thread>
#include <vector>

namespace jz
{
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
    explicit thread_pool( const size_t threads_count = std::thread::hardware_concurrency() - 1 ) : is_stop_( false ), threads_count_( threads_count ) {
        threads_.resize( threads_count );
    }
    ~thread_pool() {
        if ( !is_stop_ ) {
            stop();
        }
    }
    void start() {
        create_threads( threads_count_ );
    }
    void stop() {
        {
            std::unique_lock< std::mutex > ul( tasks_.get_lock() );
            is_stop_ = true;
        }
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

private:
    void create_threads( const size_t n ) {
        std::transform( threads_.begin(), threads_.end(), threads_.begin(), [ this ]( std::thread& t ) {
            t = std::thread( &thread_pool::work_func, this );
            return std::move( t );
        } );
    }

    void work_func() {
        while ( !is_stop_ ) {
            if ( !tasks_.empty() ) {
                auto tasks = tasks_.get_data();
                while ( !tasks.empty() ) {
                    auto t = tasks.front();
                    tasks.pop();
                    t();
                }
            }
        }
    }

private:
    bool       is_stop_;
    task_queue tasks_;
    size_t     threads_count_;

    std::vector< std::thread > threads_;
};
}  // namespace jz
#endif
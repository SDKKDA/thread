#ifndef _THREADPOOL
#define _THREADPOOL
#include "Task.hpp"
#include "ThreadSafeQueue.hpp"
#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <thread>
#include <vector>

namespace jz
{
template < typename Task_Queue >
class thread_pool
{
public:
    using task_queue = Task_Queue;

public:
    thread_pool( const thread_pool& )            = delete;
    thread_pool( thread_pool&& )                 = delete;
    thread_pool& operator=( const thread_pool& ) = delete;
    thread_pool& operator=( thread_pool&& )      = delete;

public:
    explicit thread_pool( const size_t threads_count = std::thread::hardware_concurrency() - 1 ) : is_stop_( false ), threads_count_( threads_count ) {}
    ~thread_pool() {
        {
            std::unique_lock< std::mutex > ul( tasks_.get_lock() );
            is_stop_ = true;
        }
        tasks_.notify_all();
        std::for_each( threads_.cbegin(), threads_.cend(), []( const std::shared_ptr< std::thread >& t ) {
            if ( t->joinable() ) {
                t->join();
            }
        } );
    }
    void start() {
        create_threads( threads_count_ );
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
        for ( size_t i = 0; i < n; ++i ) {
            auto t = std::make_shared< std::thread >( [ this ]() {
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
            } );
            threads_.emplace_back( std::move( t ) );
        }
    }

private:
    bool       is_stop_;
    task_queue tasks_;
    size_t     threads_count_;

    std::vector< std::shared_ptr< std::thread > > threads_;
};
}  // namespace jz
#endif
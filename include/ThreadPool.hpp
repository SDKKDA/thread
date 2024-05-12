#ifndef _THREADPOOL
#define _THREADPOOL
#include "ThreadSafeQueue.hpp"
#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace jz {
class thread_pool {
public:
    using task       = std::function< void() >;
    using task_queue = thread_safe_queue< task >;

public:
    thread_pool( const thread_pool& )            = delete;
    thread_pool( thread_pool&& )                 = delete;
    thread_pool& operator=( const thread_pool& ) = delete;
    thread_pool& operator=( thread_pool&& )      = delete;

public:
    explicit thread_pool( const size_t threads_count = std::thread::hardware_concurrency() - 1 ) : is_stop_( false ), threads_count_( threads_count ), threads( threads_count ) {}
    ~thread_pool() {
        is_stop_ = false;
        for ( auto& t : threads ) {
            if ( t->joinable() ) {
                t->join();
            }
        }
    }
    void start() {
        create_threads( threads_count_ );
    }
    void add_task( task&& t ) {
        tasks_.push( std::move( t ) );
    }

private:
    void create_threads( const size_t n ) {
        for ( size_t i = 0; i < n; ++i ) {
            auto t = std::make_shared< std::thread >( [ this ]() {
                while ( !is_stop_ ) {
                    auto task = tasks_.pop();
                    std::cout << "thread id :" << std::this_thread::get_id() << " run task" << std::endl;
                    task();
                }
            } );
            threads.emplace_back( std::move( t ) );
        }
    }

private:
    bool       is_stop_;
    task_queue tasks_;
    size_t     threads_count_;

    std::vector< std::shared_ptr< std::thread > > threads;
};
}  // namespace jz
#endif
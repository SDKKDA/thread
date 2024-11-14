#ifndef _THREADSAFEQUEUE
#define _THREADSAFEQUEUE
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>

namespace jz
{
template < class _Ty >
class thread_safe_queue
{
public:
    using value_type      = _Ty;
    using reference       = _Ty&;
    using const_reference = const _Ty&;
    using pointer         = _Ty*;
    using const_pointer   = const _Ty*;

public:
    thread_safe_queue() = default;
    thread_safe_queue( thread_safe_queue&& _Oth ) {
        std::lock_guard< std::mutex > _Lgd( _Oth._Mutex );
        std::swap( _Data, _Oth._Data );
    }
    thread_safe_queue& operator=( thread_safe_queue&& _Oth ) {
        if ( this == &_Oth ) {
            return *this;
        }
        std::lock_guard< std::mutex > _Lgd( _Mutex, _Oth._Mutex );
        std::swap( _Data, _Oth._Data );
    }
    void push( const value_type& _Val ) {
        {
            std::lock_guard< std::mutex > _Lgd( _Mutex );
            _Data.emplace( _Val );
        }
        // std::cout << "notify_one" << std::endl;

        _Cond.notify_one();
    }
    void push( value_type&& _Val ) {
        {
            std::lock_guard< std::mutex > _Lgd( _Mutex );
            _Data.emplace( std::move( _Val ) );
        }
        // std::cout << "notify_one" << std::endl;

        _Cond.notify_one();
    }
    value_type pop() {
        std::unique_lock< std::mutex > _Ulck( _Mutex );
        _Cond.wait( _Ulck, [ this ]() { return !_Data.empty(); } );

        auto _Val = _Data.front();
        _Data.pop();
        return _Val;
    }
    void pop( reference _Val ) {
        _Val = std::move( this->pop() );
    }

    bool empty() const {
        std::lock_guard< std::mutex > _Lgd( _Mutex );
        return _Data.empty();
    }
    size_t size() const {
        std::lock_guard< std::mutex > _Lgd( _Mutex );
        return _Data.size();
    }
    std::queue< value_type > get_data() {
        std::unique_lock< std::mutex > _Ulck( _Mutex );
        _Cond.wait( _Ulck, [ this ]() { return !_Data.empty(); } );
        std::queue< value_type > _New_Queue;
        _New_Queue.swap( _Data );
        return _New_Queue;
    }
    std::condition_variable& get_cond() {
        return _Cond;
    }
    std::mutex& get_lock() {
        return _Mutex;
    }
    ~thread_safe_queue() = default;

public:
    thread_safe_queue( const thread_safe_queue& )            = delete;
    thread_safe_queue& operator=( const thread_safe_queue& ) = delete;

private:
    mutable std::mutex              _Mutex;
    std::queue< value_type >        _Data;
    mutable std::condition_variable _Cond;
};
}  // namespace jz
#endif
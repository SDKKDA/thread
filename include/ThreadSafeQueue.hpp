#ifndef _THREADSAFEQUEUE
#define _THREADSAFEQUEUE
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>

namespace jz {
template < typename T >
class thread_safe_queue {

public:
    thread_safe_queue() = default;
    thread_safe_queue( thread_safe_queue&& other ) {
        std::lock_guard< std::mutex > lg( other.mutex_ );
        std::swap( data_, other.data_ );
    }
    thread_safe_queue& operator=( thread_safe_queue&& other ) {
        if ( this == &other ) {
            return *this;
        }
        std::lock_guard< std::mutex > lg( mutex_, other.mutex_ );
        std::swap( data_, other.data_ );
    }
    void push( const T& value ) {
        std::lock_guard< std::mutex > lg( mutex_ );
        data_.emplace( value );
        // std::cout << "notify_one" << std::endl;

        cond_.notify_one();
    }
    void push( T&& value ) {

        std::lock_guard< std::mutex > lg( mutex_ );
        data_.emplace( std::move( value ) );
        // std::cout << "notify_one" << std::endl;

        cond_.notify_one();
    }
    T pop() {
        std::unique_lock< std::mutex > ul( mutex_ );
        cond_.wait( ul, [ this ]() { return !data_.empty(); } );

        auto value = data_.front();
        data_.pop();
        return value;
    }
    void pop( T& value ) {
        value = std::move( pop() );
    }

    bool empty() const {
        std::lock_guard< std::mutex > lg( mutex_ );
        return data_.empty();
    }
    size_t size() const {
        std::lock_guard< std::mutex > lg( mutex_ );
        return data_.size();
    }
    std::queue< T > get_data() {
        std::unique_lock< std::mutex > ul( mutex_ );
        cond_.wait( ul, [ this ]() { return !data_.empty(); } );
        std::queue< T > new_queue;
        std::swap( new_queue, data_ );
        return new_queue;
    }
    ~thread_safe_queue() = default;

public:
    thread_safe_queue( const thread_safe_queue& )            = delete;
    thread_safe_queue& operator=( const thread_safe_queue& ) = delete;

private:
    mutable std::mutex              mutex_;
    std::queue< T >                 data_;
    mutable std::condition_variable cond_;
};
}  // namespace jz
#endif
#ifndef _THREADSAFEQUEUE
#define _THREADSAFEQUEUE
#include <mutex>
#include <queue>
namespace jz
{
template < typename T >
class thread_safe_queue
{
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
    ~thread_safe_queue() = default;

public:
    thread_safe_queue( const thread_safe_queue& )            = delete;
    thread_safe_queue& operator=( const thread_safe_queue& ) = delete;

private:
    mutable std::mutex mutex_;
    std::queue< T >    data_;
};
}  // namespace jz
#endif
#ifndef _TASKH
#define _TASKH
#include <functional>
#include <future>
#include <iostream>

namespace jz
{
class task
{
public:
    using func = std::function< void() >;

public:
    void operator()() {
        func_();
    }
    template < typename Func, typename... Args >
    task( Func&& f, Args... args ) {
        std::function< void() > ff = [ & ]() mutable { f( args... ); };
        func_                      = ff;
    }
    task( const task& other ) {
        func_ = other.func_;
    }
    task& operator=( const task& other ) {
        auto copy = other;
        std::swap( func_, copy.func_ );
        return *this;
    }
    task( task&& other ) {
        func_ = std::move( other );
    }
    task& operator=( task&& other ) {
        if ( &other == this ) {
            return *this;
        }
        std::swap( func_, other.func_ );
        return *this;
    }

private:
    func func_;
};
}  // namespace jz
#endif
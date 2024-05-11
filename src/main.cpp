#include "Singleton.hpp"
#include "ThreadSafeQueue.hpp"
#include <algorithm>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace jz;
thread_safe_queue< std::string > msg_queue;

int main( int argc, char** argv ) {
    std::thread t1( [ & ]() {
        while ( 1 ) {
            try {
                std::string msg( "push sring1" );
                msg_queue.push( msg );
                std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
                std::cout << "thread id: " << std::this_thread::get_id() << " put msg:" << msg << std::endl;
            } catch ( ... ) {
                std::cout << "catch exception" << std::endl;
            }
        }
    } );
    std::thread t2( [ & ]() {
        while ( 1 ) {
            if ( !msg_queue.empty() ) {
                std::cout << "thread id:" << std::this_thread::get_id() << " queue size:" << msg_queue.size() << std::endl;
                auto msg = msg_queue.pop();
                std::cout << "thread id:" << std::this_thread::get_id() << " get msg:" << msg << std::endl;
                std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
            }
        }
    } );
    std::thread t3( [ & ]() {
        while ( 1 ) {
            if ( !msg_queue.empty() ) {
                std::cout << "thread id:" << std::this_thread::get_id() << " queue size:" << msg_queue.size() << std::endl;
                auto msg = msg_queue.pop();
                std::cout << "thread id:" << std::this_thread::get_id() << " get msg:" << msg << std::endl;
                std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
            }
        }
    } );
    t1.join();
    t2.join();
    t3.join();
    while ( 1 ) {
    }
    return 0;
}
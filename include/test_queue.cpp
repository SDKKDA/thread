#include "ThreadSafeQueue.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
using namespace jz;
thread_safe_queue< std::string > msg_queue;
int                              main( int argc, char** argv ) {
    std::thread t1( [ & ]() {
        while ( 1 ) {
            try {
                std::string msg( "copy construct" );
                msg_queue.push( msg );
                // std::this_thread::sleep_for( std::chrono::milliseconds( 2000 ) );
                std::cout << "thread id: " << std::this_thread::get_id() << " put msg from push(const T&):" << msg << std::endl;
            }
            catch ( ... ) {
                std::cout << "catch exception" << std::endl;
            }
        }
    } );
    std::thread t2( [ & ]() {
        while ( 1 ) {
            if ( 1 ) {
                // std::cout << "thread id:" << std::this_thread::get_id() << " queue size:" << msg_queue.size() << std::endl;
                auto msg = msg_queue.pop();
                std::cout << "thread id:" << std::this_thread::get_id() << " get msg from pop():" << msg << std::endl;
                // std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
            }
        }
    } );
    std::thread t3( [ & ]() {
        while ( 1 ) {
            if ( 1 ) {
                // std::cout << "thread id:" << std::this_thread::get_id() << " queue size:" << msg_queue.size() << std::endl;
                std::string msg;
                msg_queue.pop( msg );
                std::cout << "thread id:" << std::this_thread::get_id() << " get msg from pop(T&):" << msg << std::endl;
                // std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
            }
        }
    } );
    std::thread t4( [ & ]() {
        while ( 1 ) {
            try {
                msg_queue.push( "move construct" );
                std::this_thread::sleep_for( std::chrono::milliseconds( 650 ) );
                std::cout << "thread id: " << std::this_thread::get_id() << " put msg from push(T&&)" << std::endl;
            }
            catch ( ... ) {
                std::cout << "catch exception" << std::endl;
            }
        }
    } );
    std::thread t5( [ & ]() {
        while ( 1 ) {
            try {
                auto data = msg_queue.get_data();
                std::cout << "-" << std::setw( 20 ) << std::endl;
                while ( !data.empty() ) {
                    std::cout << "thread id: " << std::this_thread::get_id() << " get data from get_data()" << data.front() << std::endl;
                    data.pop();
                }
                std::cout << "-" << std::setw( 20 ) << std::endl;
            }
            catch ( ... ) {
                std::cout << "catch exception" << std::endl;
            }
        }
    } );
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    while ( 1 ) {
    }
    return 0;
}
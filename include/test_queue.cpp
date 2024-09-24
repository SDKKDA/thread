#include "Task.hpp"
#include "ThreadSafeQueue.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace jz;
thread_safe_queue< task > msg_queue;

void test() {
    std::thread t1( [ & ]() {
        while ( 1 ) {
            try {
                std::string msg( "copy construct" );

                task t( []( std::string& str ) { std::cout << str << std::endl; }, msg );
                msg_queue.push( t );
                std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
            } catch ( ... ) {
                std::cout << "catch exception" << std::endl;
            }
        }
    } );
    std::thread t2( [ & ]() {
        while ( 1 ) {
            if ( 1 ) {
                // std::cout << "thread id:" << std::this_thread::get_id() << " queue size:" << msg_queue.size() << std::endl;
                auto tasks = msg_queue.get_data();
                std::cout << "get tasks from msg_queue: size = " << tasks.size() << std::endl;
                /*while ( !tasks.empty() ) {
                    auto task = tasks.front();
                    task();
                    tasks.pop();
                }*/
                std::this_thread::sleep_for( std::chrono::milliseconds( 2000 ) );
            }
        }
    } );

    t1.join();
    t2.join();
    while ( 1 ) {
    }
}
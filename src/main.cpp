#include "Singleton.hpp"
#include "ThreadPool.hpp"
#include "ThreadSafeQueue.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace jz;
int main() {
    jz::thread_pool pool;
    pool.start();
    while ( 1 ) {
        pool.add_task( []() { std::cout << "add tasks" << std::endl; } );
        std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
    }
    return 0;
}

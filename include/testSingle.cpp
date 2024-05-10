#include "Singleton.hpp"
#include <algorithm>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

namespace jz
{
std::vector< Singleton* > singletons;

std::mutex mt;
void       test() {
    std::vector< std::thread > threads( std::thread::hardware_concurrency() );

    size_t count = threads.size();
    for ( size_t i = 0; i < count; ++i ) {
        std::thread t( [ & ]() {
            for ( size_t i = 0; i < 1; ++i ) {
                Singleton* singleton = Singleton::getInstance();

                std::lock_guard< std::mutex > lg( mt );
                if ( singletons.empty() ) {
                    singletons.emplace_back( singleton );
                } else {
                    if ( singleton != singletons.front() ) {
                        singletons.emplace_back( singleton );
                    }
                }
            }
        } );
        threads.push_back( std::move( t ) );
    }
    for ( auto& t : threads ) {
        if ( t.joinable() ) {
            t.join();
        }
    }
    std::cout << "singleon numbers:" << singletons.size() << std::endl;
    for ( auto& it : singletons ) {
        std::cout << it << std::endl;
    }
}
}  // namespace jz
#pragma once
#include <iostream>
#include <mutex>
#include <thread>

namespace jz
{
class Singleton
{
public:
    static Singleton* getInstance() {
        std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
        std::lock_guard< std::mutex > lg( m_mutex );
        if ( m_singleton == nullptr ) {
            m_singleton = new Singleton();
        }
        return m_singleton;
    }

private:
    Singleton( const Singleton& )            = delete;
    Singleton& operator=( const Singleton& ) = delete;
    Singleton()                              = default;

private:
    static Singleton* m_singleton;

public:
    static std::mutex m_mutex;
};
Singleton* Singleton::m_singleton = nullptr;

std::mutex Singleton::m_mutex;
}  // namespace jz
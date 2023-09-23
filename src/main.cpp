#include <iostream>
#include <thread>
using namespace std;

struct A
{
    A() : m_data( 0 ) {}
    A( int val ) : m_data( val ) {}
    int m_data;
    ~A() {
        cout << "~A()" << endl;
    }
};
void func( const A& a ) {
    cout << "线程id:" << this_thread::get_id() << " " << a.m_data << endl;
}
int main( int argc, char** argv ) {
    thread t1{ func, A( 5 ) };
    t1.join();

    return 0;
}

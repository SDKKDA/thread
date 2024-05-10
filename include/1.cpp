#include <iostream>
#include <string>
int main( int argc, char** argv ) {
    std::string str = "335073900851,338973900846,325973900864,324673900920,333773900793,336373900865,337673900823,327273900843,328573900855,329873900868,";
    size_t      pos = 0;
    while ( ( pos = str.find( ",", 0 ) ) != std::string::npos ) {
        str.replace( pos, 1, "-1;" );
        pos += 1;
    }
    std::cout << str;
    return 0;
}
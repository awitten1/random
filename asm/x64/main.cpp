#include <iostream>


using namespace std;


extern "C" int SomeFunction();

int main() {
    cout << "result is " << SomeFunction() << std::endl;
    return 0;
}
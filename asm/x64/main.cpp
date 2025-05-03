#include <cstdint>
#include <iostream>


using namespace std;


extern "C" int SomeFunction();
extern "C" int RegTest();

int main() {
    cout << "result is " << SomeFunction() << std::endl;

    cout << "reg test " << RegTest() << endl;

    int i = -1;
    asm (
        "mov $56, %0"
        : "=r" (i)
    );

    cout << "i is " << i << std::endl;
    return 0;
}
#include <iostream>


using namespace std;


extern "C" int SomeFunction();

int main() {
    cout << "result is " << SomeFunction() << std::endl;

    int i = -1;
    asm (
        "mov $56, %0"
        : "=r" (i)
    );

    cout << "i is " << i << std::endl;
    return 0;
}
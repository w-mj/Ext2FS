#include <iostream>
#include "bits.h"


int main(void) {
    using namespace std;
    cout << (int)lowest_1(0) << endl;
    cout << (int)lowest_1(1) << endl;
    cout << (int)lowest_1(2) << endl;
    cout << (int)lowest_1(3) << endl;
    cout << (int)lowest_1((_u8)0xf0) << endl;
    cout << (int)lowest_0((_u8)0x07) << endl;
    
}
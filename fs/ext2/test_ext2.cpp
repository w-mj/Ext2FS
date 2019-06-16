#include <iostream>
#include "ext2_fs.h"
using namespace std;

int main(void) {
    cout << sizeof(EXT2::SuperBlock) << endl;
    cout << sizeof(EXT2::GroupDescriptor) << endl;
    cout << sizeof(EXT2::Inode) << endl;
}
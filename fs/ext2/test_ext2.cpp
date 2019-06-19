#include <iostream>
#include "ext2_fs.h"
#include "fs/vfs.h"
#include "dev/mock_disk.h"
using namespace std;

int main(void) {
    cout << sizeof(EXT2::SuperBlock) << endl;
    cout << sizeof(EXT2::GroupDescriptor) << endl;
    cout << sizeof(EXT2::Inode) << endl;

    Dev::MockDisk d;
    d.open("fs/ext2/fs");
    EXT2::EXT2_FS ext2(&d);

    for (auto& x: ext2.root->children) {
        cout << x->name << "  " << (int)x->type << "  " << x->inode_n << endl;
    }
}
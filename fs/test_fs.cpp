#include <iostream>
#include <map>
#include "ext2/ext2_fs.h"
#include "dev/block_dev.h"
#include "dev/mock_disk.h"

void ls(VFS::DEntry *cwd) {
    using namespace std;
    for (auto& x: cwd->children) {
        cout << x->name << "  ";
    }
    cout << endl;
}

int main(void) {
    using namespace std;

    Dev::BlockDevice *dev = new Dev::MockDisk();
    dev->open("fs/ext2/fs");
    VFS::FS *fs = new EXT2::EXT2_FS(dev);
    cout << "System up." << endl;
    cout << "File System at " << "fs/ext2/fs" << endl;
    string cmd;
    VFS::DEntry *cwd = nullptr;

    cout << "$ ";
    while (getline(cin, cmd)) {
        if (cmd == "mount") {
            fs->mount();
            cwd = fs->root;
            cout << "mount disk at /" << endl;
        } else if (fs->status == VFS::FS_unmounted) {
            cout << "Please mount fs first." << endl;
        } else if (cmd == "ls") {
            cwd->load_children();
            ls(cwd);
        }

        cout << "$ ";
    }
}
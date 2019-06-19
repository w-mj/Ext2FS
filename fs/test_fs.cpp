#include <iostream>
#include <vector>
#include <string>
#include "ext2/ext2_fs.h"
#include "dev/block_dev.h"
#include "dev/mock_disk.h"

std::vector<std::string> split(const std::string& s, char sp) {
    int i = 0, l = 0;
    std::vector<std::string> res;
    while (i < s.size()) {
        if (s[i] == sp) {
            if (i != l)
                res.push_back(s.substr(l, i - l));
            l = i + 1;
        }
        i++;
    }
    if (i != l)
        res.push_back(s.substr(l, i - l));
    return res;
}

void ls(VFS::DEntry *cwd, std::vector<std::string> cmdd) {
    using namespace std;
    cwd->load_children();
    for (auto& x: cwd->children) {
        if (cmdd.size() == 2 && cmdd[1] == "l")
            cout << x->name << "  " << x->inode_n << endl;
        else
            cout << x->name << "  ";
    }
    cout << endl;
}

VFS::DEntry *cd(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    using namespace std;
    cwd->load_children();
    list<VFS::DEntry*>::iterator it = cwd->children.begin();
    VFS::DEntry *ans = cwd;
    if (cmdd[1] == ".")
        ;
    else if (cmdd[1] == "..") {
        
        if (cwd->parent != nullptr)
            ans = cwd->parent;
    } else  {
        while(it != cwd->children.end()) {
            if (cmdd[1] == (*it)->name) {
                break;
            }
            it++;
        }
        if (it == cwd->children.end()) {
            cout << "No such directory " << cmdd[1] << endl;
        } else {
            if ((*it)->type != VFS::Directory) {
                cout << cmdd[1] << " is not a directory." << endl;
            } else {
                ans = *it;
            }
        }
    }
    // if (cwd != ans)
    //     delete cwd;
    return ans;
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

    cout << "[umount]$ ";
    while (getline(cin, cmd)) {
        vector<string> cmdd = split(cmd, ' ');
        if (cmd == "mount") {
            fs->mount();
            cwd = fs->root;
            cout << "mount disk at /" << endl;
        } else if (fs->status == VFS::FS_unmounted) {
            cout << "Please mount fs first." << endl;
        } else if (cmdd[0] == "ls") {
            ls(cwd, cmdd);
        } else if (cmdd[0] == "cd") {
            cwd = cd(cwd, cmdd);
        }
        
        if (cwd != nullptr)
            cout << cwd->name << "$ ";
        else
            cout << "[umount]$ ";
    }
}
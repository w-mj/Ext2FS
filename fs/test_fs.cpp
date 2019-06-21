#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include "ext2/ext2_fs.h"
#include "dev/block_dev.h"
#include "dev/mock_disk.h"
#include "delog/delog.h"

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
            cout << "(" << x->inode_n <<")"<< x->name << "  ";
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
            // _pos();
            // _sa((*it)->name.c_str(), (*it)->name.size());
            // _sa(cmdd[1].c_str(), cmdd[1].size());
            if (strcmp(cmdd[1].c_str(), (*it)->name.c_str()) == 0) {
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
    return ans;
}

void mkdir(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    cwd->mkdir(cmdd[1]);
}

void touch(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    cwd->create(cmdd[1]);
}

int main(void) {
    using namespace std;

    Dev::BlockDevice *dev = new Dev::MockDisk();
    dev->open("fs/ext2/fs");
    EXT2::EXT2_FS *ext2_fs = new EXT2::EXT2_FS(dev);
    VFS::FS *fs = ext2_fs;
    cout << "System up." << endl;
    cout << "File System at " << "fs/ext2/fs" << endl;
    string cmd;
    VFS::DEntry *cwd = nullptr;

    fs->mount();
    cwd = fs->root;

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
        } else if (cmdd[0] == "mkdir") {
            mkdir(cwd, cmdd);
        } else if (cmdd[0] == "exit") {
            break;
        } else if (cmdd[0] == "dump") {
            MM::Buf buf(1024);
            if (cmdd[1] == "i")
                ext2_fs->dev->read(buf, ext2_fs->inode_to_pos(stoi(cmdd[2])), 1024);
            else 
                ext2_fs->dev->read(buf, ext2_fs->block_to_pos(stoi(cmdd[2])), 1024);
            _sa(buf.data, 1024);
        } else if (cmdd[0] == "touch") {
            touch(cwd, cmdd);
        } else {
            cout << "Unknown command " << cmdd[0] << endl;
        }
        
        if (cwd != nullptr)
            cout << cwd->name << "$ ";
        else
            cout << "[umount]$ ";
    }
    dev->close();
}
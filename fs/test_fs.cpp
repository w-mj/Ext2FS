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

struct OP {
    const char *name;
    VFS::DEntry* (*op)(VFS::DEntry *, const std::vector<std::string>&);
};

VFS::DEntry *ls(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    using namespace std;
    cwd->load_children();
    for (auto& x: cwd->children) {
        if (cmdd.size() == 2 && cmdd[1] == "l")
            cout << x->name << "  " << x->inode_n << endl;
        else
            cout << "(" << x->inode_n <<")"<< x->name << "  ";
    }
    cout << endl;
    return cwd;
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
        ans = cwd->get_child(cmdd[1]);
        if (ans == nullptr) {
            cout << "No such directory " << cmdd[1] << endl;
            ans = cwd;
        } else if (ans->type != VFS::Directory) {
            cout << cmdd[1] << " is not a directory." << endl;
            ans = cwd;
        }
    }
    return ans;
}

VFS::DEntry *mkdir(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    cwd->mkdir(cmdd[1]);
    return cwd;
}

VFS::DEntry *touch(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    cwd->create(cmdd[1]);
    return cwd;
}

VFS::DEntry *cat(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    VFS::File *fe = cwd->open_file(cmdd[1]);
    if (fe == nullptr)
        std::cout << "No such file " << cmdd[1] << std::endl;
    else {
        _si(fe->size);
        char *buf = new char[fe->size];
        fe->read((_u8*)buf, fe->size);
        fwrite(buf, fe->size, 1, stdout);
        putchar('\n');
        fflush(stdout);
        delete[] buf;
        delete fe;
    }
    return cwd;
}

OP ops[] = {
    {"ls", ls},
    {"cd", cd},
    {"mkdir", mkdir},
    {"touch", touch},
    {"cat", cat},
    {"unknown", nullptr}
};

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
        } else if (cmdd[0] == "exit") {
            break;
        } else if (cmdd[0] == "dump") {
            MM::Buf buf(1024);
            if (cmdd[1] == "i")
                ext2_fs->dev->read(buf, ext2_fs->inode_to_pos(stoi(cmdd[2])), 1024);
            else 
                ext2_fs->dev->read(buf, ext2_fs->block_to_pos(stoi(cmdd[2])), 1024);
            _sa(buf.data, 1024);
        } else {
            int i = 0;
            while (ops[i].op != nullptr && strcmp(ops[i].name, cmdd[0].c_str()) != 0) {
                i++;
            }
            if (ops[i].op == nullptr)
                cout << "Unknown command " << cmdd[0] << endl;
            else
                cwd = ops[i].op(cwd, cmdd);
        }
        
        if (cwd != nullptr)
            cout << cwd->name << "$ ";
        else
            cout << "[umount]$ ";
    }
    dev->close();
}
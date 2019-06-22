#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include "ext2/ext2_fs.h"
#include "dev/block_dev.h"
#include "dev/mock_disk.h"
#include "delog/delog.h"
#include "stat.h"

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
        if (cmdd.size() == 2 && cmdd[1] == "-l") {
            x->inflate();
            switch (x->type) {
                case VFS::Directory:
                    cout << "d";
                    break;
                case VFS::RegularFile:
                    cout << "-";
                    break;
                default:
                    cout << "?";
                    break;
            }
            cout << previlege_to_str(x->inode->mode) << " " << x->inode->uid << " " << x->inode->gid;
            cout << " "  << x->inode->size << " " << x->inode->ctime << " " << x->name << endl;
        }
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
    VFS::File *fe = cwd->open(cmdd[1]);
    if (fe == nullptr)
        std::cout << "No such file " << cmdd[1] << std::endl;
    else if (fe->type != VFS::RegularFile) {
        std::cout << cmdd[1] << " is not a regular file." << std::endl;
        delete fe;
    } else {
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

// void rm_r(VFS::DEntry *cwd) {
//     _dbg_log("rm_r %d", cwd->inode_n);
//     cwd->load_children();
//     for (auto x: cwd->children) {
//         _dbg_log("%s %d, parent %d", x->name.c_str(), x->inode_n, cwd->parent->inode_n);
//         if (x->inode_n == cwd->inode_n || x->inode_n == cwd->parent->inode_n)
//             continue;  // 跳过. 和..
//         if (x->type == VFS::Directory) {
//             x->unlink_children();
//         } 
//         _pos();
//         _dbg_log("start unlink %d", x->inode_n);
//         cwd->unlink(x);  // 删除空目录和文件
//         _dbg_log("unlink %d finish", x->inode_n);
//     }
//     _dbg_log("rm_r finish");
// }

VFS::DEntry *rm(VFS::DEntry *cwd, const std::vector<std::string>& cmdd) {
    VFS::DEntry *to_del = cwd->get_child(cmdd[cmdd.size() - 1]);
    if (to_del == nullptr) {
        std::cout << "No such file " << cmdd[cmdd.size() - 1] << std::endl;
    } else {
        if (to_del->type == VFS::Directory && cmdd[1] != "-r") {
            std::cout << cmdd[cmdd.size() - 1] << " is a directory." << std::endl;
        } else {
            if (to_del->type == VFS::Directory)
                to_del->unlink_children();
            cwd->unlink(to_del);
        }
    }
    return cwd;
}

OP ops[] = {
    {"ls", ls},
    {"cd", cd},
    {"mkdir", mkdir},
    {"touch", touch},
    {"cat", cat},
    {"rm", rm},
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
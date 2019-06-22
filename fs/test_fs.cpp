#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <stack>
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
    VFS::DEntry* (*op)(VFS::DEntry *, VFS::NameI *, const std::vector<std::string>&);
};

static void ls_one(VFS::DEntry *x, bool l) {
    using namespace std;
    if (l) {
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
    } else {
        cout << "(" << x->inode_n <<")"<< x->name << "  ";
    }
}

VFS::DEntry *ls(VFS::DEntry *cwd, VFS::NameI *target, const std::vector<std::string>& cmdd) {
    using namespace std;
    auto t = cwd->get_child(target);
    if (t == nullptr) {
        cout << "No such file or directory. " << cmdd[cmdd.size() - 1] << endl;
        return cwd;
    }
    if (t->type == VFS::Directory) {
        t->load_children();
        for (auto& x: t->children) {
            ls_one(x, cmdd.size() == 2 && cmdd[1] == "-l");
        }
    } else  {
        ls_one(t, cmdd.size() == 2 && cmdd[1] == "-l");
    }
    cout << endl;

    return cwd;
}

VFS::DEntry *cd(VFS::DEntry *cwd, VFS::NameI *target, const std::vector<std::string>& cmdd) {
    using namespace std;
    auto ans = cwd->get_child(target);
    if (ans == nullptr) {
        cout << "No such directory " << cmdd[1] << endl;
        ans = cwd;
    } else if (ans->type != VFS::Directory) {
        cout << cmdd[1] << " is not a directory." << endl;
        ans = cwd;
    }
    return ans;
}

VFS::DEntry *mkdir(VFS::DEntry *cwd, VFS::NameI *target, const std::vector<std::string>& cmdd) {
    std::string name;
    VFS::DEntry *d = cwd->get_path(target, &name);

    if (d == nullptr) {
        std::cout << "No such directory " << name << std::endl;
    }else 
    d->mkdir(name);
    return cwd;
}

VFS::DEntry *touch(VFS::DEntry *cwd, VFS::NameI *target, const std::vector<std::string>& cmdd) {
    std::string name;
    VFS::DEntry *d = cwd->get_path(target, &name);

    if (d == nullptr) {
        std::cout << "No such directory " << name << std::endl;
    }else 
    d->create(name);
    return cwd;
}

VFS::DEntry *cat(VFS::DEntry *cwd, VFS::NameI *target, const std::vector<std::string>& cmdd) {
    auto ans = cwd->get_child(target);
    VFS::File *fe = ans->open();
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

VFS::DEntry *rm(VFS::DEntry *cwd, VFS::NameI *target, const std::vector<std::string>& cmdd) {
    VFS::DEntry *d;
    VFS::DEntry *to_del = cwd->get_child(target, &d);
    if (to_del == nullptr) {
        std::cout << "No such file " << cmdd[cmdd.size() - 1] << std::endl;
    } else {
        if (to_del->type == VFS::Directory && cmdd[1] != "-r") {
            std::cout << cmdd[cmdd.size() - 1] << " is a directory." << std::endl;
        } else {
            if (to_del->type == VFS::Directory)
                to_del->unlink_children();
            d->unlink(to_del);
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


VFS::NameI *parse_path(const std::string path) {
    using namespace VFS;
    NameI *ans = nullptr, *prev;
    if (path[0] == '/')
        ans = prev = new NameI("/");
    else
        ans = prev = new NameI(".");
    printf("%x %x\n", ans->prev, ans->next);
    std::vector<std::string> sp = split(path, '/');
    for (const auto& x: sp) {
        printf("++++++++++++++++++++++++++++++++++++\n");
        _ss(x.c_str());
        if (x.size() == 0)
            continue;
        if (x == ".")
            continue;
        if (x == "..") {
            if (prev->prev == nullptr) {
                goto error;
            }
            prev = prev->prev;
            prev->next->prev = nullptr;
            prev->next->next = nullptr;
            delete prev->next;
        } else if (ans == nullptr) {
            ans = new NameI(x);
            prev = ans;
        } else {
            prev = new NameI(x, prev);
        }
    }
    _pos();
    return ans;
error:
    delete ans;
    return nullptr;
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
            else {
                VFS::NameI *path = nullptr;
                if (cmdd.size() > 1) {
                    path = parse_path(cmdd[cmdd.size() - 1]);
                } else {
                    path = parse_path(".");
                }
                if (path == nullptr) {
                    cout << "Wrong path " << cmdd[cmdd.size() - 1] << endl;
                } else {
                    cwd = ops[i].op(cwd, path, cmdd);
                    delete path;
                }
            }
        }
        
        if (cwd != nullptr) {
            std::stack<std::string> st;
            VFS::DEntry *t = cwd;
            while (t != t->parent) {
                st.push(t->name);
                t = t->parent;
            }
            cout << "/";
            while (!st.empty()) {
                cout << st.top() << "/";
                st.pop();
            }
            cout << "$ ";
        } else
            cout << "[umount]$ ";
    }
    dev->close();
}
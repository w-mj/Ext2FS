#pragma once
#include "vfs.h"
#include <list>
#include <string>
#include <iostream>
#include <algorithm>

struct MountedFS {
    VFS::FS *fs = nullptr;
    std::string point;  // 挂载点
    MountedFS(VFS::FS *vfs, const std::string& p): fs(vfs), point(p) {}
};

class FSManager {
private:
    std::list<MountedFS> fs_list;
    VFS::DEntry *root = nullptr;
public:

    void mount(VFS::FS *vfs, const std::string& p) {
        for (auto &x: fs_list) {
            if (x.point == p) {
                std::cout << p << " is already mounted by another fs" << std::endl;
            }
        }
        fs_list.emplace_back(vfs, p);
        if (p == "/")
            root = vfs->root;
    }

    void umount(const std::string& p) {
        if (p == "/") {
            std::cout << "Cannot umount root." << std::endl;
            return;
        }
        auto res = std::find_if(fs_list.begin(), fs_list.end(), [&p](const MountedFS& x){x.point == p;});
        if (res == fs_list.end()) {
            std::cout << "No any filesystem mounted on " << p << std::endl;
            return;
        }
        fs_list.erase(res);
        delete res->fs;
    }
};
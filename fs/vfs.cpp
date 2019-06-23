#include "vfs.h"
#include "delog/delog.h"
#include "stat.h"
#include <string>
#include <sstream>
#include <stack>

using namespace VFS;

DEntry *DEntry::get_child(const std::string& name) {
    load_children();
    for (auto x: children) {
        if (x->name == name)
            return x;
    }
    return nullptr;
}

DEntry *DEntry::get_child(const NameI *namei, DEntry **path) {
    DEntry *ans = this;
    if (namei->name == "/") {
        ans=fs->root;
    }
    namei = namei->next;
    if (path != nullptr)
        *path = ans;
    while (namei != nullptr && ans != nullptr) {
        if (path != nullptr)
            *path = ans;
        ans = ans->get_child(namei->name);
        namei = namei->next;
    }
    _pos();
    return ans;
}


DEntry *DEntry::get_path(const NameI *namei, std::string* fname) {
    DEntry *ans = this;
    if (namei->name == "/") {
        ans=fs->root;
    }
    namei = namei->next;
    if (namei == nullptr) {
        return ans;
    }
    
    while (namei->next != nullptr && ans != nullptr) {
        if (namei->name == ".") {
            ;
        } else if (namei->name == "..") {
            ans = ans->parent;
        } else {
            ans = ans->get_child(namei->name);
        }
        namei = namei->next;
    }
    if (fname != nullptr) {
        (*fname) = namei->name;
    }
    return ans;
}

std::string DEntry::printed_path() {
    std::stack<std::string> st;
    std::stringstream ss;
    VFS::DEntry *t = this;
    while (t != t->parent) {
        st.push(t->name);
        t = t->parent;
    }
    while (!st.empty()) {
        ss << "/" << st.top();
        st.pop();
    }
    return ss.str();
}

File *DEntry::open(const std::string& name) {
    auto a = get_child(name);
    if (a == nullptr || a->type != RegularFile)
        return nullptr;
    return a->get_file();
}

File *DEntry::open() {
    return get_file();
}


NameI::NameI(const std::string name, NameI *p): name(name) {
    if (p != nullptr) {
        p->next = this;
    }
    prev = p;
    next = nullptr;
}

NameI::~NameI() {
    if (next != nullptr)
        next->prev = nullptr;
    if (prev != nullptr)
        prev->next = nullptr;
    delete next;
    delete prev;
}


void VFS::File::close() {
    delete this;  // 恶魔操作
}

_u8 VFS::mode_to_type(_u8 mode) {
    if(S_ISLNK(mode)) return VFS::SymbolLink;
    if(S_ISREG(mode)) return VFS::RegularFile;
    if(S_ISDIR(mode)) return VFS::Directory;
    if(S_ISCHR(mode)) return VFS::CharacterDevice;
    if(S_ISBLK(mode)) return VFS::BlockDevice;
    if(S_ISFIFO(mode)) return VFS::NamedPipe;
    if(S_ISSOCK(mode)) return VFS::SymbolLink;
    return Unknown;
}
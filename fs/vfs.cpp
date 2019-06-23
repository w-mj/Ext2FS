#include "vfs.h"
#include "delog/delog.h"
#include <string>

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
        ans = ans->get_child(namei->name);
        namei = namei->next;
    }
    if (fname != nullptr) {
        (*fname) = namei->name;
    }
    return ans;
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
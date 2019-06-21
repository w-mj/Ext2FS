#include "vfs.h"
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
#pragma once
#include "fs/vfs.h"
#include "ext2_fs.h"

namespace EXT2
{

    class EXT2_FS;
    
class EXT2_Inode: public VFS::Inode {
public:
    class iterator {
        int pos=0;
        EXT2_Inode* inode;
        iterator(EXT2_Inode* i, int p);
        iterator& operator++();
        iterator& operator++(int);
        bool operator==(const iterator& ano) const;
    };

    EXT2_Inode(EXT2_FS*, _u32, EXT2::Inode*);
    _u32 inode_n;
    EXT2::Inode* i;
    void print();

    ~EXT2_Inode();

};
} // namespace EXT2

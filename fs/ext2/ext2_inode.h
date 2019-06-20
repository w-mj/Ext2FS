#pragma once
#include "fs/vfs.h"
#include "ext2_fs.h"

namespace EXT2
{

    class EXT2_FS;
    
    class EXT2_Inode: public VFS::Inode {
    public:
        class iterator {
            int index = 1;
            EXT2_Inode* inode;
        public:
            iterator(EXT2_Inode* i);
            iterator& operator++();
            iterator& operator++(int);
            bool operator==(const iterator& ano) const;
            bool operator!=(const iterator& ano) const;
            int operator*() const ;

            friend class EXT2_Inode;
        };

        _u32 inode_n;
        EXT2::Inode* i;
        EXT2_FS *ext2_fs;

        EXT2_Inode(EXT2_FS*, _u32, EXT2::Inode*);
        void print();
        
        // 返回该文件的第b个字节所在块的块号
        _u32 byte_in_block(_u32 b);
        _u32 nth_block(_u32 n);

        iterator begin();
        iterator end();

        ~EXT2_Inode();

};
} // namespace EXT2

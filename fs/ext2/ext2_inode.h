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
            int block_index=0;

            // 块数计数器
            int index_cnt = 0;
            // buf0指向i->block
            // 三级分别页面缓存，各1k
            _u32 *block_buf[4] = {nullptr};
            // 每个缓存块的真实块号
            _u32 block_pos[4] = {0};
            // 在每个寻址级别下的最大可寻址块数
            // [0, 256+12, 256^2+256+12, 256^3+256^2+256+12]
            _u32 max_blocks[4];
            // 在每个缓存页面上的指针
            _u32 indexs[4] = {0};
            // 索引等级，可取值0 1 2 3
            int level = 0;
            // 每个间接索引块含的间接索引数
            _u32 sub_blocks_in_block[4];
            EXT2_Inode* inode;
        public:
            iterator(EXT2_Inode* i);
            ~iterator();
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

        void write_inode();

        iterator begin();
        iterator end();

        ~EXT2_Inode();

};
} // namespace EXT2

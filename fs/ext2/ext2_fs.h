#pragma once

#include "types.h"
#include "fs/vfs.h"
#include "dev/block_dev.h"
#include "ext2_disk_stru.h"
#include "ext2_inode.h"
#include "ext2_dentry.h"
namespace EXT2 {

    class EXT2_Inode;
    class EXT2_DEntry;

    class EXT2_FS: public VFS::FS {

        void printFS();
        void printInode(Inode*);
    public:
        int block_to_pos(int block);
        int inode_to_pos(int inode_n);

        SuperBlock* sb;
        std::list<GroupDescriptor*> gdt_list;

        _u32 block_size;
        EXT2_FS(Dev::BlockDevice* dev);
        void mount();
        ~EXT2_FS();
    };


}

#pragma once

#include "types.h"
#include "fs/vfs.h"
#include "dev/block_dev.h"
#include "ext2_disk_stru.h"
namespace EXT2 {


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

class EXT2_Inode: public VFS::Inode {
public:
    EXT2_Inode(EXT2_FS*, _u32 n, EXT2::Inode*);
    _u32 inode_n;
    EXT2::Inode* i;
    void print();

    ~EXT2_Inode();

};

class EXT2_DEntry: public VFS::DEntry {
private:
    EXT2_FS* ext2_fs;
public:
    EXT2_Inode* ext2_inode = nullptr;

    _u8 sync = 0;  // 为1时需要调用inflate从磁盘读取

    // 父文件夹可以与此项目有不同的文件系统
    EXT2_DEntry(EXT2_FS*, VFS::DEntry*, _u32, _u8, const std::string&);
    void inflate();
    void load_children();

    ~EXT2_DEntry();
};

}

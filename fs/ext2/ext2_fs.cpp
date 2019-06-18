#include "ext2_fs.h"
#include "mm/buf.h"
#include "delog/delog.h"
#include "stat.h"

#include <cstring>
#include <iostream>

#include <cmath>
using namespace EXT2;

EXT2_FS::EXT2_FS(Dev::BlockDevice* dev): FS(dev) {
    mount();
}

void EXT2_FS::printFS() {
    using namespace std;
    cout << endl;
    cout << "文件系统中Inode数量 " << sb->inodes_count << endl;
    cout << "以块为单位的文件系统大小 " << sb->blocks_count << endl;
    cout << "文件系统中保留块数目 " << sb->r_blocks_count << endl;
    cout << "文件系统中空闲块数目 " << sb->free_blocks_count << endl;
    cout << "文件系统中空闲Inode数量 " << sb->free_inodes_count << endl;
    cout << "第一个使用的块号 " << sb->first_data_block << endl;
    cout << "块大小 " << block_size << endl;
    cout << "逻辑片大小 " << sb->log_frag_size << endl;
    cout << "每组块数 " << sb->blocks_per_group << endl;
    cout << "每组碎片数 " << sb->frags_per_group << endl;
    cout << "每组Inode数 " << sb->inodes_per_group << endl;
    cout << "上次挂载时间 " << sb->mtime << endl;
    cout << "上次写入超级块的时间 " << sb->wtime << endl;
    cout << "未进行检查的挂载次数 " << sb->mnt_count << endl;
    cout << "必须执行检查前的最大挂载次数 " << sb->max_mnt_count << endl;
    cout << "Magic " << sb->magic << endl;
    cout << "文件系统的状态 " << sb->state << endl;

    cout << "组描述符表：" << endl;
    for (GroupDescriptor* gdt: gdt_list) {
        cout << "块位图块号 " << gdt->block_bitmap << endl;
        cout << "索引节点位图的块号 " << gdt->inode_bitmap << endl;
        cout << "第一个索引节点表块的块号 " << gdt->inode_table << endl;
        cout << "组中空闲块的个数 " << gdt->free_blocks_count << endl;
        cout << "组中空闲索引节点的个数 " << gdt->free_inodes_count << endl;
        cout << "组中目录的个数 " << gdt->used_dirs_count << endl;

    }
}

void EXT2_FS::printInode(Inode* i) {
    using namespace std;
    cout << "\nInode:" << endl;
    cout << "文件类型和访问权限 " << i->mode << mode_to_str(i->mode) << endl;
    cout << "拥有者 " << i->uid << endl;
    cout << "文件长度 " << i->size << endl;
    cout << "最后一次访问时间 " << i->atime << endl;
    cout << "索引节点最后改变的时间 " << i->ctime << endl;
    cout << "文件内容最后改变的时间 " << i->mtime << endl;
    cout << "文件删除的时间 " << i->dtime << endl;
    cout << "组id " << i->gid << endl;
    cout << "硬链接计数器 " << i->links_count << endl;
    cout << "文件的数据块数 " << i->blocks << endl;
    cout << "文件标志 " << i->flags << endl;
    cout << "文件版本 " << i->generation << endl;
    cout << "文件访问控制列表 " << i->file_acl << endl;
    cout << "目录访问控制列表 " << i->dir_acl << endl;
    cout << "片地址 " << i->faddr << endl;
    cout << "数据块指针 ";
    for (int j = 0; j < i->blocks;j++)
        cout << i->block[j] << " ";
    cout << endl;
}

int EXT2_FS::block_to_pos(int block) {
    _si(block);
    return (block - 1) * block_size + 1024;
}

void EXT2_FS::mount() {
    sb = new SuperBlock();
    dev->seek(1024);  // 跳过引导块
    MM::Buf sb_buf(1024);
    dev->read(sb_buf, 1024);  // 先读1k的超级块
    memmove(sb, sb_buf.data, 1024);

    _si(sb->log_block_size);
    _si(sb->blocks_count);
    _si(sb->blocks_per_group);

    // 1024 * (1 << sb->log_block_size) * 8;
    block_size = 1024 * (1 << sb->log_block_size);
    dev->seek(block_size + 1024);  // 指向GDT

    int group_cnt = 8 * sb->blocks_count / sb->blocks_per_group;
    for (int i = 0; i < group_cnt; i++) {
        GroupDescriptor* gtp = new GroupDescriptor();
        dev->read(sb_buf, sizeof(GroupDescriptor));
        memmove(gtp, sb_buf.data, sizeof(GroupDescriptor));
        gdt_list.push_back(gtp);
    }

    // 数据块位图
    dev->seek(block_to_pos(gdt_list.front()->inode_bitmap));
    dev->read(sb_buf, block_size);
    _sa(sb_buf.data, 1024);

    // 第一个索引节点 / 
    dev->seek(block_to_pos(gdt_list.front()->inode_table));
    dev->read(sb_buf, sizeof(Inode));
    dev->read(sb_buf, sizeof(Inode));
    Inode* root_inode = new Inode();
    memmove(root_inode, sb_buf.data, sizeof(Inode));
    // _sa(sb_buf.data, 1024);
    printFS();
    printInode(root_inode);
}

EXT2_FS::~EXT2_FS() {
    for (auto x: gdt_list) {
        delete x;
    }
    delete sb;
}

EXT2_Inode::EXT2_Inode(EXT2_FS *fs, EXT2::Inode *i): VFS::Inode(fs) {
    ino = 0;
    counter = 0;
    mode = i->mode;
    nlink = i->links_count;
    uid = i->uid;
    gid = i->gid;
    dev = fs->dev;
    size = i->size;
    atime = i->atime;
    mtime = i->mtime;
    ctime = i->ctime;
    blkbits = 0;
    version = i->generation;
    blocks = i->blocks;
    bytes = 0;
    sock = 1;
}
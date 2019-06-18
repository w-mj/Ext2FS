#include "ext2_fs.h"
#include "mm/buf.h"
#include "delog/delog.h"

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
    cout << "文件系统中总块数 " << sb->blocks_count << endl;
    cout << "文件系统中保留块数目 " << sb->r_blocks_count << endl;
    cout << "文件系统中空闲块数目 " << sb->free_blocks_count << endl;
    cout << "文件系统中空闲Inode数量 " << sb->free_inodes_count << endl;
    cout << "第一个数据块的位置 " << sb->first_data_block << endl;
    cout << "块大小 " << 1024 * pow(2, sb->log_block_size) << endl;
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

void EXT2_FS::mount() {
    sb = new SuperBlock();
    dev->seek(1024);  // 跳过引导块
    MM::Buf sb_buf(1024);
    dev->read(sb_buf, 1024);  // 先读1k的超级块
    memmove(sb, sb_buf.data, 1024);

    _si(sb->log_block_size);
    _si(sb->blocks_count);
    _si(sb->blocks_per_group);
    
    dev->seek(1024 * (1 << sb->log_block_size) + 1024);  // 指向GDT

    int group_cnt = sb->blocks_count / sb->blocks_per_group;
    for (int i = 0; i < group_cnt; i++) {
        GroupDescriptor* gtp = new GroupDescriptor();
        dev->read(sb_buf, sizeof(GroupDescriptor));
        memmove(gtp, sb_buf.data, 1024);
        gdt_list.push_back(gtp);
    }
    printFS();
}

EXT2_FS::~EXT2_FS() {
    for (auto x: gdt_list) {
        delete x;
    }
    delete sb;
}
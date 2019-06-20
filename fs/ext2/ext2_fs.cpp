#include "ext2_fs.h"
#include "mm/buf.h"
#include "delog/delog.h"
#include "stat.h"
#include "bits.h"

#include <cstring>
#include <iostream>

#include <cmath>
using namespace EXT2;

EXT2_FS::EXT2_FS(Dev::BlockDevice* dev): FS(dev) {
    // mount();
    status = VFS::FS_unmounted;
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
    for (EXT2_GD* gdt: gdt_list) {
        cout << "块位图块号 " << gdt->get_gd()->block_bitmap << endl;
        cout << "索引节点位图的块号 " << gdt->get_gd()->inode_bitmap << endl;
        cout << "第一个索引节点表块的块号 " << gdt->get_gd()->inode_table << endl;
        cout << "组中空闲块的个数 " << gdt->get_gd()->free_blocks_count << endl;
        cout << "组中空闲索引节点的个数 " << gdt->get_gd()->free_inodes_count << endl;
        cout << "组中目录的个数 " << gdt->get_gd()->used_dirs_count << endl;
    }
}


int EXT2_FS::block_to_pos(int block) {
    // _si(block);
    return (block - 1) * block_size + 1024;
}

int EXT2_FS::inode_to_pos(int inode_n) {
    int max_inodes_in_group = 8 * block_size;
    auto it = gdt_list.begin();
    while (it != gdt_list.end() && inode_n > max_inodes_in_group) {
        inode_n -= max_inodes_in_group;
        it++;
    }
    _error(it == gdt_list.end());
    _error(inode_n > max_inodes_in_group);

    return block_to_pos((*it)->get_gd()->inode_table) + inode_n * sizeof(Inode);
}

void EXT2_FS::mount() {
    sb = new SuperBlock();
    dev->seek(1024);  // 跳过引导块
    MM::Buf sb_buf(1024);
    dev->read(sb_buf, 1024);  // 先读1k的超级块
    memmove(sb, sb_buf.data, 1024);

    magic = sb->magic;
    _sx(magic);

    // _si(sb->log_block_size);
    // _si(sb->blocks_count);
    // _si(sb->blocks_per_group);

    // 1024 * (1 << sb->log_block_size) * 8;
    block_size = 1024 * (1 << sb->log_block_size);
    dev->seek(block_size + 1024);  // 指向GDT

    group_cnt = 8 * sb->blocks_count / sb->blocks_per_group;
    for (int i = 0; i < group_cnt; i++) {
        GroupDescriptor* gtp = new GroupDescriptor();
        dev->read(sb_buf, sizeof(GroupDescriptor));
        memmove(gtp, sb_buf.data, sizeof(GroupDescriptor));
        gdt_list.emplace_back(this, gtp, i);
    }

    // 数据块位图
    dev->seek(block_to_pos(gdt_list.front()->get_gd()->inode_bitmap));
    dev->read(sb_buf, block_size);
    // _sa(sb_buf.data, 1024);

    // 第一个索引节点 / 
    // dev->seek(block_to_pos(gdt_list.front()->inode_table));
    // dev->read(sb_buf, sizeof(Inode));
    // dev->read(sb_buf, sizeof(Inode));
    // Inode* root_inode = new Inode();
    // memmove(root_inode, sb_buf.data, sizeof(Inode));
    _u32 root_inode_pos = gdt_list.front()->get_gd()->inode_table;
    EXT2_DEntry* ext2_entry;
    ext2_entry = new EXT2_DEntry(this, nullptr, 1, VFS::Directory, "/");
    root = ext2_entry;
    // ext2_entry->inflate();
    // ext2_entry->load_children();
    
    // _sa(sb_buf.data, 1024);
    printFS();
    //printInode(ext2_entry->ext2_inode->i);
    status = VFS::FS_mounted;
}

_u32 EXT2_FS::alloc_inode() {
    for (EXT2_GD* gdd: gdt_list) {
        _u32 t = gdd->alloc_inode();
        if (t != 0) {
            _si(t);
            return t;
        }
    }
    return 0;
}

_u32 EXT2_FS::alloc_block() {
    for (EXT2_GD* gdd: gdt_list) {
        _u32 t = gdd->alloc_block();
        if (t != 0) {
            _si(t);
            return t;
        }
    }
    return 0;
}

void EXT2_FS::write_super() {
    _u32 super_pos = 1;
    MM::Buf buf(sizeof(SuperBlock));
    memcpy(buf.data, sb, sizeof(SuperBlock));
    for (auto x: gdt_list) {
        dev->seek(super_pos);
        dev->write(buf, sizeof(SuperBlock));
        super_pos += sb->blocks_per_group;
    }
}

void EXT2_FS::write_gdt() {
    _u32 super_pos = 2;
    MM::Buf buf(gdt_list.size() * sizeof(GroupDescriptor));
    _u32 s_pos = 0;
    for (auto x: gdt_list) {
        memcpy(buf.data + s_pos, x->get_gd(), sizeof(GroupDescriptor));
        s_pos += sizeof(GroupDescriptor);
        x->write_inode_bitmap();
        x->write_block_bitmap();
    }
    for (auto x: gdt_list) {
        dev->seek(super_pos);
        dev->write(buf, sizeof(SuperBlock));
        super_pos += sb->blocks_per_group;
    }
}


EXT2_FS::~EXT2_FS() {
    for (auto x: gdt_list) {
        delete x;
    }
    delete sb;
}

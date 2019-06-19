#include "ext2_fs.h"
#include "mm/buf.h"
#include "delog/delog.h"
#include "stat.h"

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
    for (GroupDescriptor* gdt: gdt_list) {
        cout << "块位图块号 " << gdt->block_bitmap << endl;
        cout << "索引节点位图的块号 " << gdt->inode_bitmap << endl;
        cout << "第一个索引节点表块的块号 " << gdt->inode_table << endl;
        cout << "组中空闲块的个数 " << gdt->free_blocks_count << endl;
        cout << "组中空闲索引节点的个数 " << gdt->free_inodes_count << endl;
        cout << "组中目录的个数 " << gdt->used_dirs_count << endl;

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

    return block_to_pos((*it)->inode_table) + inode_n * sizeof(Inode);
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
    // _sa(sb_buf.data, 1024);

    // 第一个索引节点 / 
    // dev->seek(block_to_pos(gdt_list.front()->inode_table));
    // dev->read(sb_buf, sizeof(Inode));
    // dev->read(sb_buf, sizeof(Inode));
    // Inode* root_inode = new Inode();
    // memmove(root_inode, sb_buf.data, sizeof(Inode));
    _u32 root_inode_pos = gdt_list.front()->inode_table;
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

EXT2_FS::~EXT2_FS() {
    for (auto x: gdt_list) {
        delete x;
    }
    delete sb;
}



EXT2_DEntry::EXT2_DEntry(EXT2_FS* fs1, VFS::DEntry* parent1, 
            _u32 inode_n1, _u8 type1, const std::string& name1) {
    fs = fs1;
    ext2_fs = fs1;
    parent = parent1;
    inode = nullptr;
    inode_n = inode_n1;
    type=type1;
    name = std::string(name1);
    sync = 1;
}

void EXT2_DEntry::load_children() {
    if (ext2_inode != nullptr && sync == 0)
        return ;
    inflate();
    _log_info("load children");

    _error(ext2_inode == nullptr);
    //_si(ext2_inode->size);

    Dev::BlockDevice* dev = ext2_fs->dev;
    MM::Buf buf(ext2_fs->block_size);

    DirEntry *temp_str = new DirEntry();
    for (auto x: children)
        delete x;
    children.clear();

    for (_u32 i = 0; i < ext2_inode->i->blocks; i++) {
        _u32 data_block_pos = ext2_fs->block_to_pos(ext2_inode->i->block[i]);
        dev->seek(data_block_pos);
        dev->read(buf, ext2_fs->block_size);
        // _sa(buf.data, ext2_fs->block_size);
        _u32 s_pos = 0;
        _u32 next_length = 12;
        while (true) {
            // _sa(buf.data + s_pos, 20);
            memmove(temp_str, buf.data + s_pos, sizeof(DirEntry));
            if (temp_str->rec_len == 0)
                break;
            EXT2_DEntry *sub = new EXT2_DEntry(ext2_fs, this, 
                    temp_str->inode - 1, temp_str->file_type, 
                    std::string((char*)temp_str->name, temp_str->name_len));
            children.push_back(sub);
            // std::cout << sub->name << " " << sub->inode_n << std::endl;
            s_pos += temp_str->rec_len;
        }
    }
    
}

void EXT2_DEntry::inflate() {
    if (inode != nullptr && sync != 1)
        return;
    _log_info("inflate");
    EXT2::Inode* disk_inode = new Inode();
    MM::Buf buf(sizeof(Inode));
    _u32 inode_pos = ext2_fs->inode_to_pos(inode_n);
    _si(inode_pos);
    fs->dev->seek(inode_pos);
    fs->dev->read(buf, sizeof(Inode));
    memcpy(disk_inode, buf.data, sizeof(Inode));
    ext2_inode = new EXT2_Inode(ext2_fs, inode_n, disk_inode);
    ext2_inode->print();
    inode = ext2_inode;
    sync = 0;
}


EXT2_DEntry::~EXT2_DEntry() {
    delete inode;
}


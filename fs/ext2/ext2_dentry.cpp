#include "ext2_dentry.h"
#include "ext2_file.h"
#include "delog/delog.h"
#include "mm/buf.h"
#include <cstring>
#include <ctime>

using namespace EXT2;
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
#include <iostream>
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
    // for (_u32 i = 0; i < ext2_inode->i->blocks; i++) {
    _sp(*ext2_inode);
    ext2_inode->print();
    for (int i: *ext2_inode) {
        _u32 data_block_pos = ext2_fs->block_to_pos(i);
        dev->read(buf, data_block_pos, ext2_fs->block_size);
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
    delete inode;
    EXT2::Inode* disk_inode = new Inode();
    MM::Buf buf(sizeof(Inode));
    _u32 inode_pos = ext2_fs->inode_to_pos(inode_n);
    _si(inode_pos);
    fs->dev->read(buf, inode_pos, sizeof(Inode));
    memcpy(disk_inode, buf.data, sizeof(Inode));
    ext2_inode = new EXT2_Inode(ext2_fs, inode_n, disk_inode);
    ext2_inode->print();
    inode = ext2_inode;
    sync = 0;
}

void EXT2_DEntry::mkdir(const std::string& new_name) {
    _u32 new_i_n = ext2_fs->alloc_inode();
    _u32 new_b_n = ext2_fs->alloc_block();
    load_children();

    // 构建新的Inode
    EXT2::Inode *new_disk_i = new EXT2::Inode();
    _sp(*new_disk_i);
    new_disk_i->mode = ext2_inode->mode;
    new_disk_i->uid = ext2_inode->uid;
    new_disk_i->size = ext2_fs->block_size;
    new_disk_i->atime = time(0);
    new_disk_i->mtime = time(0);
    new_disk_i->dtime = time(0);
    new_disk_i->gid = ext2_inode->gid;
    new_disk_i->links_count = 2;  // . 和父目录中的自己
    new_disk_i->blocks = 2;
    new_disk_i->flags = 0;
    new_disk_i->block[0] = new_b_n;
    EXT2_Inode *new_i = new EXT2_Inode(ext2_fs, new_i_n, new_disk_i);
    new_i->write_inode();  // 新Inode写入磁盘
    EXT2_DEntry *new_entry = new EXT2_DEntry(ext2_fs, this, new_i_n, VFS::Directory, new_name);
    new_entry->ext2_inode = new_i;
    // 加入到当前目录中
    children.push_back(new_entry);
    // 构建新的DirEntry
    DirEntry new_disk_entry[1];
    new_disk_entry->file_type = VFS::Directory;
    new_disk_entry->inode = new_i_n;
    strcpy((char*)new_disk_entry->name, new_name.c_str());
    new_disk_entry->name_len = new_name.size();
    _u32 expend_name_len = new_disk_entry->name_len;
    if (expend_name_len % 4 != 0)
        expend_name_len += 4 - expend_name_len % 4;
    new_disk_entry->rec_len = 8 + expend_name_len;
    // 子目录的DirEntry写入父目录的文件体中
    EXT2_File parent_body(this, this->ext2_inode);
    parent_body.write((_u8*)new_disk_entry, new_disk_entry->rec_len);
    // 在子目录的文件体中写入.和..
    EXT2_File child_body(new_entry, new_i);
    new_disk_entry->rec_len = 12;
    new_disk_entry->name_len = 1;
    new_disk_entry->name[0] = '.';
    new_disk_entry->name[1] = '\0';
    child_body.write((_u8*)new_disk_entry, new_disk_entry->rec_len);
    new_disk_entry->inode = this->inode_n;
    new_disk_entry->name_len = 2;
    new_disk_entry->name[1] = '.';
    new_disk_entry->name[2] = '\0';
    child_body.write((_u8*)new_disk_entry, new_disk_entry->rec_len);

    // ext2_fs->sb->
    // ext2_fs->write_gdt();
    // ext2_fs->write_super();
}


EXT2_DEntry::~EXT2_DEntry() {
    delete inode;
}


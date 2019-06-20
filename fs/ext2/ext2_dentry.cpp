#include "ext2_dentry.h"
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
    for (int i: *ext2_inode) {
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

void EXT2_DEntry::mkdir(std::string& new_name) {
    _u32 new_i_n = ext2_fs->alloc_inode();
    _u32 new_b_n = ext2_fs->alloc_block();
    load_children();

    EXT2::Inode *new_disk_i = new EXT2::Inode();
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
    EXT2_DEntry *new_entry = new EXT2_DEntry(ext2_fs, this, new_i_n, VFS::Directory, new_name);
    new_entry->ext2_inode = new_i;
    
    children.push_back(new_entry);

}


EXT2_DEntry::~EXT2_DEntry() {
    delete inode;
}


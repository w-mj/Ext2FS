#include "ext2_file.h"
#include <cstring>
#include "delog/delog.h"
using namespace EXT2;


EXT2_File::EXT2_File(EXT2_DEntry *d, EXT2_Inode *i) {
    ext2_dentry = d;
    ext2_inode = i;
    ext2_fs = i->ext2_fs;
    type = d->type;
    pos = 0;
    size = i->i->size;
    _dbg_log("打开文件");
    _si(size);
}

EXT2_File::EXT2_File(EXT2_DEntry *d) {
    if (d->ext2_inode == nullptr)
        d->inflate();
    ext2_dentry = d;
    ext2_inode = d->ext2_inode;
    ext2_fs = d->ext2_inode->ext2_fs;
    type = d->type;
    pos = 0;
    size = d->ext2_inode->i->size;
    _dbg_log("打开文件");
    _si(size);
}


_u32 EXT2_File::tell() {
    return pos;
}

_u32 EXT2_File::seek(int pos, int whence) {
    switch (whence) {
        case VFS::SEEK::CUR:
            this->pos += pos;
            break;
        case VFS::SEEK::END:
            this->pos = std::min(this->size + pos, this->size);
            break;
        case VFS::SEEK::SET:
            this->pos = pos;
            break;
        default:
            _error_s(whence, "Unknow whence");
    }
    return this->pos;
}

_u32 EXT2_File::read(_u8 *buf, _u32 len) {
    if (pos + len > size)
        return 0;
    _u32 read_len = 0, this_read_len;
    _u32 block_size = ext2_fs->block_size;  // 块大小
    _u32 byte_in_block = pos % block_size;  // 第一个字节在块中的位置
    _u32 nth_block = pos / block_size;  // 第一个字节在第n个块，pos以0开始计数，不需要加1
    auto it = ext2_inode->iter_at(nth_block);
    _u32 block_n = *it;  // 第一个字节的块号
    _si(block_n);
    _u32 pos_in_fs = ext2_fs->block_to_pos(block_n);  // 第一个块在文件系统中的位置

    MM::Buf buff(block_size);
    ext2_fs->dev->read(buff, pos_in_fs, block_size);  // 从第一个块读入数据，每次都读出一整块
    memcpy(buf, buff.data + byte_in_block, std::min(len, block_size - byte_in_block));
    read_len = std::min(len, block_size - byte_in_block);
    while (read_len < len && pos + read_len < size) {
        it++;  // 指向下一个块
        block_n = *it;  // 下一个块号
        pos_in_fs = ext2_fs->block_to_pos(block_n);  // 下一个块的位置
        this_read_len = ext2_fs->dev->read(buff, pos_in_fs, block_size);  // 读入
        memcpy(buf + read_len, buff.data, std::min(block_size, len - read_len));
        read_len += this_read_len;
    }
    pos += read_len;
    return read_len;
}

_u32 EXT2_File::write(_u8 *buf, _u32 len) {
    if (len + pos > size)
        resize(len + pos);
    ext2_inode->print();
    _u32 write_len = 0, this_write_len;
    _u32 block_size = ext2_fs->block_size;  // 块大小
    _u32 byte_in_block = pos % block_size;  // 第一个字节在块中的位置
    _u32 nth_block = pos / block_size;  // 第一个字节在第n个块，pos以0开始计数，不需要加1
    auto it = ext2_inode->iter_at(nth_block);
    _u32 block_n = *it;  // 第一个字节的块号
    _u32 pos_in_fs = ext2_fs->block_to_pos(block_n);  // 第一个块在文件系统中的位置
    //_si(pos + len);
    MM::Buf buff(block_size);
    this_write_len = std::min(block_size - byte_in_block, len);
    memcpy(buff.data, buf, this_write_len);
    ext2_fs->dev->write(buff, pos_in_fs + byte_in_block, this_write_len);  // 向第一个块写数据
    write_len = this_write_len;
    while (write_len < len) {
        it++;  // 指向下一个块
        block_n = *it;  // 下一个块号
        pos_in_fs = ext2_fs->block_to_pos(block_n);  // 下一个块的位置
        this_write_len = std::min(block_size, len - write_len);
        memcpy(buff.data, buf + write_len, this_write_len);
        ext2_fs->dev->write(buff, pos_in_fs, this_write_len);  // 写
        write_len += this_write_len;
    }
    pos += write_len;
    //_si(pos);
    if (pos > size)
        size = pos;
    return write_len;
}

void EXT2_File::resize(_u32 new_size) {
    EXT2_FS *fs = ext2_fs;
    _u32 current_block = ext2_inode->blocks;
    _u32 target_block = new_size / fs->block_size + (new_size % fs->block_size > 0);
    _dbg_log("resize from %d to %d", current_block, target_block);
    if (target_block == current_block) {
        return;
    }
    auto it = ext2_inode->end(); 
    if (target_block > current_block) {
        // 分配空间
        while (target_block != current_block) {
            *it;  // 利用解引用自动分配空间
            it++;
            current_block++;
        }
    } else {
        // 缩小空间
        while (target_block != current_block) {
            it--;
            _u32 to_release = *it;
            fs->release_block(to_release);
            current_block--;
        }
    }
    size = new_size;
    ext2_inode->size = size;
    ext2_inode->blocks = target_block;
    ext2_inode->write_inode();
    ext2_fs->write_gdt();
    ext2_fs->write_super();
}
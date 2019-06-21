#include "ext2_file.h"
#include <cstring>
#include "delog/delog.h"
using namespace EXT2;


EXT2_File::EXT2_File(EXT2_DEntry *d, EXT2_Inode *i) {
    ext2_dentry = d;
    ext2_inode = i;
    ext2_fs = i->ext2_fs;
    pos = 0;
    size = i->i->size;
    _dbg_log("打开文件");
    _si(size);
    // MM::Buf buf(size);
    // read(buf.data, size);
    // _sa(buf.data, size);
    // pos = 0;
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
    _u32 nth_block = pos / block_size + 1;  // 第一个字节在第n个块
    _u32 block_n = ext2_inode->nth_block(nth_block);  // 第一个字节的块号
    _u32 pos_in_fs = ext2_fs->block_to_pos(block_n);  // 第一个块在文件系统中的位置

    MM::Buf buff(block_size);
    read_len = ext2_fs->dev->read(buff, pos_in_fs + byte_in_block, std::min(block_size - byte_in_block, len));  // 从第一个块读入数据
    memcpy(buf, buff.data, read_len);
    while (read_len < len && pos + read_len < size) {
        nth_block++;  // 指向下一个块
        block_n = ext2_inode->nth_block(nth_block);  // 下一个块号
        pos_in_fs = ext2_fs->block_to_pos(block_n);  // 下一个块的位置
        this_read_len = ext2_fs->dev->read(buff, pos_in_fs, std::min(block_size, len - read_len));  // 读入
        memcpy(buf, buff.data + read_len, this_read_len);
        read_len += this_read_len;
    }
    pos += read_len;
    return read_len;
}

_u32 EXT2_File::write(_u8 *buf, _u32 len) {
    _u32 write_len = 0, this_write_len;
    _u32 block_size = ext2_fs->block_size;  // 块大小
    _u32 byte_in_block = pos % block_size;  // 第一个字节在块中的位置
    _u32 nth_block = pos / block_size + 1;  // 第一个字节在第n个块
    _u32 block_n = ext2_inode->nth_block(nth_block);  // 第一个字节的块号
    _u32 pos_in_fs = ext2_fs->block_to_pos(block_n);  // 第一个块在文件系统中的位置
    _si(pos + len);
    MM::Buf buff(block_size);
    this_write_len = std::min(block_size - byte_in_block, len);
    memcpy(buff.data, buf, this_write_len);
    ext2_fs->dev->write(buff, pos_in_fs + byte_in_block, this_write_len);  // 向第一个块写数据
    write_len = this_write_len;
    while (write_len < len) {
        nth_block++;  // 指向下一个块
        block_n = ext2_inode->nth_block(nth_block);  // 下一个块号
        pos_in_fs = ext2_fs->block_to_pos(block_n);  // 下一个块的位置
        this_write_len = std::min(block_size, len - write_len);
        memcpy(buff.data, buf + write_len, this_write_len);
        ext2_fs->dev->write(buff, pos_in_fs, this_write_len);  // 写
        write_len += this_write_len;
    }
    pos += write_len;
    _si(pos);
    if (pos > size)
        size = pos;
    return write_len;
}

void EXT2_File::resize(_u32 new_size) {
    if (size == new_size)
        return;
    if (ext2_inode->i->blocks >= 13)
        _error_s(ext2_inode->i->blocks, "not support big file yet");
    _u32 current_blocks = ext2_inode->i->blocks - 1;
    _u32 target_blocks = new_size / ext2_fs->block_size + ((new_size % ext2_fs->block_size) > 0);
    if (new_size < size) {
        while (target_blocks < current_blocks) {
            current_blocks--;
            ext2_fs->release_block(ext2_inode->i->block[current_blocks]);  // 循环释放当前块
        }
    } else {
        while (target_blocks > current_blocks) {
            ext2_inode->i->block[current_blocks] = ext2_fs->alloc_block();
            current_blocks++;
        }
    }
    ext2_inode->i->blocks = current_blocks + 1;
}
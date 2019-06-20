#include "ext2_group_descriptor.h"
#include "ext2_disk_stru.h"
#include "bits.h"
#include <cstring>
using namespace EXT2;


EXT2_GD::EXT2_GD(EXT2_FS *fs, GroupDescriptor* gd, _u32 n) {
    this->gd = gd;
    this->fs = fs;
    group_n = n;
}

EXT2_GD::~EXT2_GD() {
    delete gd;
    delete inode_bitmap;
    delete block_bitmap;
}

GroupDescriptor *EXT2_GD::get_gd() {
    return gd;
}

_u8 *EXT2_GD::get_inode_bitmap() {
    if (inode_bitmap != nullptr)
        return inode_bitmap;
    MM::Buf buf(fs->block_size);
    fs->dev->seek(fs->block_to_pos(gd->inode_bitmap));
    fs->dev->read(buf, fs->block_size);
    inode_bitmap = new _u8[fs->block_size];
    memcpy(inode_bitmap, buf.data, fs->block_size);
    return inode_bitmap;
}

_u8 *EXT2_GD::get_block_bitmap() {
    if (block_bitmap != nullptr)
        return block_bitmap;
    MM::Buf buf(fs->block_size);
    fs->dev->seek(fs->block_to_pos(gd->block_bitmap));
    fs->dev->read(buf, fs->block_size);
    block_bitmap = new _u8[fs->block_size];
    memcpy(block_bitmap, buf.data, fs->block_size);
    return block_bitmap;
}

_u32 EXT2_GD::alloc_inode() {
    if (gd->free_inodes_count == 0)
        return 0;
    get_inode_bitmap();
    for (int i = 0; i < fs->block_size; i++) {
        if (inode_bitmap[i] != (_u8)0xff) {
            int t = lowest_0(inode_bitmap[i]);
            inode_bitmap[i] |= _BITS_SIZE(t);
            gd->free_blocks_count--;         
            return t + i * 8;
        }
    }
    return 0;
}

_u32 EXT2_GD::alloc_block() {
    if (gd->free_blocks_count == 0)
        return 0;
    get_block_bitmap();
    for (int i = 0; i < fs->block_size; i++) {
        if (block_bitmap[i] != (_u8)0xff) {
            int t = lowest_0(block_bitmap[i]);
            block_bitmap[i] |= _BITS_SIZE(t);
            gd->free_blocks_count--;         
            return t + i * 8;
        }
    }
    return 0;
}

void EXT2_GD::write_inode_bitmap() {
    fs->dev->seek(fs->block_to_pos(gd->inode_bitmap));
    MM::Buf buf(fs->block_size);
    memcpy(buf.data, inode_bitmap, fs->block_size);
    fs->dev->write(buf, fs->block_size);
}

void EXT2_GD::write_block_bitmap() {
    fs->dev->seek(fs->block_to_pos(gd->block_bitmap));
    MM::Buf buf(fs->block_size);
    memcpy(buf.data, block_bitmap, fs->block_size);
    fs->dev->write(buf, fs->block_size);
}
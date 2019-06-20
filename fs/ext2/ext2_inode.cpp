#include "ext2_inode.h"
#include "stat.h"
#include "delog/delog.h"
#include "mm/buf.h"
#include <iostream>
#include <cstring>
using namespace EXT2;

void EXT2_Inode::print() {
    using namespace std;
    cout << "\nInode:  " << inode_n << endl;
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

EXT2_Inode::EXT2_Inode(EXT2_FS *fs, _u32 n, EXT2::Inode *i): VFS::Inode(fs) {
    this->i = i;
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
    inode_n = n;
    ext2_fs = fs;
}

_u32 EXT2_Inode::byte_in_block(_u32 b) {
    _u32 ans = nth_block(b / ext2_fs->block_size);

}

_u32 EXT2_Inode::nth_block(_u32 n) {
    if (n == 0 || (n - 1) * ext2_fs->block_size > i->size)
        return 0;
    n--;// n从1开始计数
    if (n < 12)
        return i->block[n];
    _u32 ans;
    _u32 blocks_in_block = ext2_fs->block_size / 4;
    MM::Buf buf(ext2_fs->block_size);
    if (n < 12 + blocks_in_block) {
        ext2_fs->dev->read(buf, ext2_fs->block_to_pos(i->block[12]), ext2_fs->block_size);
        n -= 12;
        memcpy(&ans, buf.data + n, 4);
        return ans;
    }
    if (n < 12 + blocks_in_block + blocks_in_block * blocks_in_block) {
        ext2_fs->dev->read(buf, ext2_fs->block_to_pos(i->block[13]), ext2_fs->block_size);
        n -= 12;  
        memcpy(&ans, buf.data + (n / blocks_in_block), 4);
        ext2_fs->dev->read(buf, ext2_fs->block_to_pos(ans), ext2_fs->block_size); 
        memcpy(&ans, buf.data + (n % blocks_in_block), 4);
        return ans;
    } else {
        ext2_fs->dev->read(buf, ext2_fs->block_to_pos(i->block[14]), ext2_fs->block_size);
        n -= 12;  
        memcpy(&ans, buf.data + (n / (blocks_in_block * blocks_in_block)), 4);
        ext2_fs->dev->read(buf, ext2_fs->block_to_pos(ans), ext2_fs->block_size); 
        memcpy(&ans, buf.data + (n / blocks_in_block), 4);
        ext2_fs->dev->read(buf, ext2_fs->block_to_pos(ans), ext2_fs->block_size); 
        memcpy(&ans, buf.data + (n % blocks_in_block), 4);
        return ans;
    }
    return 0;
}


EXT2_Inode::iterator EXT2_Inode::begin() {
    iterator it(this);
    return it;
}

EXT2_Inode::iterator EXT2_Inode::end() {
    iterator it(this);
    it.index = i->blocks - 1;
    return it;
}

EXT2_Inode::~EXT2_Inode() {
    delete i;
}

EXT2_Inode::iterator::iterator(EXT2_Inode* i) {
    inode = i;
}

EXT2_Inode::iterator& EXT2_Inode::iterator::operator++(int) {
    index++;
    return *this;
}

EXT2_Inode::iterator& EXT2_Inode::iterator::operator++() {
    index++;
    return *this;
}

bool
EXT2_Inode::iterator::operator==(const iterator& ano) const {
    _error(inode != ano.inode);
    return index == ano.index;
}

bool
EXT2_Inode::iterator::operator!=(const iterator& ano) const {
    _error(inode != ano.inode);
    return index != ano.index;
}


int EXT2_Inode::iterator::operator*() const {
    return inode->nth_block(index);
}
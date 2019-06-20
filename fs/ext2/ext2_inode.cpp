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

EXT2_Inode::iterator EXT2_Inode::begin() {
    iterator it(this);
    return it;
}

EXT2_Inode::iterator EXT2_Inode::end() {
    iterator it(this);
    it.pos_0 = i->blocks;
    _u32 t = i->blocks;
    if (t <= 12) {
        return it;
    } 
    _u32 cnt_in_data_block = ext2_fs->block_size / 4;
    t -= 12;
    if (t < cnt_in_data_block ) {
        it.pos_1 = t;
        return it;
    }
    t -= cnt_in_data_block;
    if (t < cnt_in_data_block * cnt_in_data_block) {
        it.pos_1 = t / cnt_in_data_block;
        it.pos_2 = t % cnt_in_data_block;
        return it;
    }
    t -= cnt_in_data_block * cnt_in_data_block;
    it.pos_1 = t / (cnt_in_data_block * cnt_in_data_block);
    it.pos_2 = t / cnt_in_data_block;
    it.pos_3 = t % cnt_in_data_block;
    return it;
}

EXT2_Inode::~EXT2_Inode() {
    delete i;
}

EXT2_Inode::iterator::iterator(EXT2_Inode* i) {
    inode = i;
}

EXT2_Inode::iterator& EXT2_Inode::iterator::operator++(int) {
    return operator++();
}

EXT2_Inode::iterator& EXT2_Inode::iterator::operator++() {
    _u32 cnt_in_data_block = inode->ext2_fs->block_size / 4;
    if (pos_0 < 12) {
        // 直接寻址
        pos_0++;
    } else if (pos_0 == 12) {
        // 一次间址
        if (pos_1 < cnt_in_data_block) {
            pos_1++;
        } else {
            pos_1 = 0;
            pos_2++;
        }
    } else if (pos_0 == 13) {
        // 两次间址
        if (pos_2 < cnt_in_data_block) {
            pos_2++;
        } else if (pos_1 < cnt_in_data_block) {
            pos_2 = 0;
            pos_1++;
        } else {
            pos_2 = 0;
            pos_1 = 0;
            pos_0++;
        }
    } else if (pos_0 == 14) {
        // 三次间址
        if (pos_3 < cnt_in_data_block) {
            pos_3++;
        } else if (pos_2 < cnt_in_data_block) {
            pos_3 = 0;
            pos_2++;
        } else if (pos_1 < cnt_in_data_block) {
            pos_3 = 0;
            pos_2 = 0;
            pos_1++;
        } else {
            pos_3 = 0;
            pos_2 = 0;
            pos_1 = 0;
            pos_0++;
        }
    }
    return *this;
}

bool
EXT2_Inode::iterator::operator==(const iterator& ano) const {
    _error(inode != ano.inode);
    return 
        pos_0 == ano.pos_0 &&
        pos_1 == ano.pos_1 &&
        pos_2 == ano.pos_2 &&
        pos_3 == ano.pos_3;
}

bool
EXT2_Inode::iterator::operator!=(const iterator& ano) const {
    return !operator==(ano);
}


int EXT2_Inode::iterator::operator*() const {
    if (pos_0 < 12)
        return inode->i->block[pos_0];

    MM::Buf buf(inode->ext2_fs->block_size);
    EXT2_FS* fs = inode->ext2_fs;
    _u32 ans;

    fs->dev->seek(fs->block_to_pos(inode->i->block[pos_0]));
    fs->dev->read(buf, fs->block_size);
    memmove(&ans, buf.data + pos_1 * 4, 4);
    if (pos_0 == 12) {
        return ans;
    }

    fs->dev->seek(fs->block_to_pos(ans));
    fs->dev->read(buf, fs->block_size);
    memmove(&ans, buf.data + pos_2 * 4, 4);
    if (pos_0 == 13) {
        return ans;
    }

    fs->dev->seek(fs->block_to_pos(ans));
    fs->dev->read(buf, fs->block_size);
    memmove(&ans, buf.data + pos_3 * 4, 4);
    if (pos_0 == 14) {
        return ans;
    }
    return pos_0;
}
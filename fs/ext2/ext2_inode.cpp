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
    cout << "文件的数据块数 " << blocks << endl;
    cout << "文件标志 " << i->flags << endl;
    cout << "文件版本 " << i->generation << endl;
    cout << "文件访问控制列表 " << i->file_acl << endl;
    cout << "目录访问控制列表 " << i->dir_acl << endl;
    cout << "片地址 " << i->faddr << endl;
    cout << "数据块指针 ";
    for (int j = 0; j < blocks;j++)
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
    blocks = i->blocks / (ext2_fs->block_size / 512);
}

_u32 EXT2_Inode::byte_in_block(_u32 b) {
    _u32 ans = nth_block(b / ext2_fs->block_size);
    return ans;
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

void EXT2_Inode::write_inode() {
    _dbg_log("inode %d, size %d", inode_n, i->size);
    _u32 inode_pos = ext2_fs->inode_to_pos(inode_n);
    MM::Buf buf(sizeof(EXT2::Inode));
    memcpy(buf.data, i, sizeof(EXT2::Inode));
    // _sa(buf.data, sizeof(EXT2::Inode));
    ext2_fs->dev->write(buf, inode_pos, sizeof(EXT2::Inode));
    // memset(buf.data, 0, sizeof(EXT2::Inode));
    // _sa(buf.data, sizeof(EXT2::Inode));
    // ext2_fs->dev->read(buf, inode_pos, sizeof(EXT2::Inode));
    // _sa(buf.data, sizeof(EXT2::Inode));
    
}


EXT2_Inode::iterator EXT2_Inode::begin() {
    iterator it(this);
    _si(it.index_cnt);
    return it;
}

EXT2_Inode::iterator EXT2_Inode::end() {
    iterator it(this);
    it.index_cnt = blocks;  // 最大值
    _si(it.index_cnt);
    return it;
}

EXT2_Inode::~EXT2_Inode() {
    delete i;
}

// iterator below

EXT2_Inode::iterator::iterator(EXT2_Inode* i) {
    inode = i;
    block_buf[0] = i->i->block;
    sub_blocks_in_block[0] = 12;
    sub_blocks_in_block[1] = i->ext2_fs->block_size / 4;
    sub_blocks_in_block[2] = sub_blocks_in_block[1];
    sub_blocks_in_block[3] = sub_blocks_in_block[2];
    _u32 indexs_per_block[4] = {
        12,
        sub_blocks_in_block[1]
    };
    indexs_per_block[2] = indexs_per_block[1] * indexs_per_block[1];
    indexs_per_block[3] = indexs_per_block[2] * indexs_per_block[1];
    max_blocks[0] = indexs_per_block[0];
    max_blocks[1] = max_blocks[0] + indexs_per_block[1];
    max_blocks[2] = max_blocks[1] + indexs_per_block[2];
    max_blocks[3] = max_blocks[2] + indexs_per_block[3];
}

EXT2_Inode::iterator::~iterator() {
    delete block_buf[1];
    delete block_buf[2];
    delete block_buf[3];
}


const EXT2_Inode::iterator& EXT2_Inode::iterator::operator++(int) {
    return ++(*this);
}

EXT2_Inode::iterator& EXT2_Inode::iterator::operator++() {
    index++;  // [0, 12)

    index_cnt++;
    indexs[level]++;
    _si(indexs[level]);
    if (indexs[level] >= sub_blocks_in_block[level]) {
        // 小升级! 
        int t = level;
        while (t > 0 && indexs[t] >= sub_blocks_in_block[level]) {
            // 重设指针
            indexs[t - 1]++;
            indexs[t] = 0;
            t--;
        }
        if (t == 0) {
            level++;  // 大升级！
            _error(level == 4);
        }
        // 从t+1级开始重新设置页面
        // t级所指向的位置就是t+1级的地址
        EXT2_FS *fs = inode->ext2_fs;
        while (t < level) {
            _u32 new_block_pos = block_buf[t][indexs[t]];
            MM::Buf buf(fs->block_size);
            // TODO: Dirty save
            block_pos[t + 1] = new_block_pos;
            fs->dev->read(buf, fs->block_to_pos(new_block_pos), fs->block_size);
            if (block_buf[t + 1] == nullptr)
                block_buf[t + 1] = new _u32[sub_blocks_in_block[level]];
            memcpy(block_buf[t + 1], buf.data, sub_blocks_in_block[t]);
            t++;
        }
    }
    return *this;
}

bool
EXT2_Inode::iterator::operator==(const iterator& ano) const {
    _error(inode != ano.inode);
    return !((*this) == ano);
}

bool
EXT2_Inode::iterator::operator!=(const iterator& ano) const {
    _error(inode != ano.inode);
    return index_cnt != ano.index_cnt;
}


int EXT2_Inode::iterator::operator*() const {
    int ans = block_buf[level][indexs[level]];
    _si(ans);
    return ans;
    // _pos();
    // fflush(stdout);
    // int ans = inode->nth_block(index);
    // // std::cout <<ans << std::endl;
    return ans;
}
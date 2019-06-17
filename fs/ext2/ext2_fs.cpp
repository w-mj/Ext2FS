#include "ext2_fs.h"
#include "mm/buf.h"
#include <cstring>
#include <iostream>

#include <cmath>
using namespace EXT2;

EXT2_FS::EXT2_FS(Dev::BlockDevice* dev): FS(dev) {
    mount();
}

static void printSuperBlock(const SuperBlock& sb) {
    std::cout << "文件系统中Inode数量" << sb.inodes_count << std::endl;
    std::cout << "文件系统中总块数" << sb.blocks_count << std::endl;
    std::cout << "文件系统中保留块数目" << sb.r_blocks_count << std::endl;
    std::cout << "文件系统中空闲块数目" << sb.free_blocks_count << std::endl;
    std::cout << "文件系统中空闲Inode数量" << sb.free_inodes_count << std::endl;
    std::cout << "第一个数据块的位置" << sb.first_data_block << std::endl;
    std::cout << "块大小" << 1024 * pow(2, sb.log_block_size) << std::endl;
    std::cout << "逻辑片大小" << sb.log_frag_size << std::endl;
    std::cout << "每组块数" << sb.blocks_per_group << std::endl;
    std::cout << "每组碎片数" << sb.frags_per_group << std::endl;
    std::cout << "每组Inode数" << sb.inodes_per_group << std::endl;
    std::cout << "上次挂载时间" << sb.mtime << std::endl;
    std::cout << "上次写入超级块的时间" << sb.wtime << std::endl;
    std::cout << "未进行检查的挂载次数" << sb.mnt_count << std::endl;
    std::cout << "必须执行检查前的最大挂载次数" << sb.max_mnt_count << std::endl;
    std::cout << "Magic" << sb.magic << std::endl;
    std::cout << "文件系统的状态" << sb.state << std::endl;

}

void EXT2_FS::mount() {
    SuperBlock* sb = new SuperBlock();
    dev->seek(1024);
    MM::Buf sb_buf(1024);
    dev->read(sb_buf, 1024);
    memmove(sb, sb_buf.data, 1024);
    printSuperBlock(*sb);
}
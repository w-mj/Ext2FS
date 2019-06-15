#ifndef _INODE_H_
#define _INODE_H_
#include "types.h"

constexpr _u16 EXT2_VALID_FS = 0x01;  // 文件系统状态定义，没有正常卸载
constexpr _u16 EXT2_ERROR_FS = 0x02;  // 文件系统状态定义，内核代码检测到错误
/**
 * ext2 文件系统的超级块定义
 */
struct ext2_super_block {
    _u32 s_inodes_count;  // 文件系统中Inode数量
    _u32 s_blocks_count;  // 文件系统中总块数
    _u32 s_r_blocks_count;  // 文件系统中保留块数目
    _u32 s_free_blocks_count;  // 文件系统中空闲块数目
    _u32 s_free_inodes_count;  // 文件系统中空心Inode数量
    _u32 s_first_data_block;  // 第一个数据块的位置，通常当数据块大小为1k时为1，否则为0
    _u32 s_log_block_size;  // 逻辑块数量
    _i32 s_log_frag_size;  // 逻辑碎片大小
    _u32 s_blocks_per_group;  // 每组块数
    _u32 s_frags_per_group;  // 每组Fragment数？
    _u32 s_inodes_per_group;  // 每组Inode数
    _u32 s_mtime;  // 上次挂载时间，为1970-1-1到现在的秒数
    _u32 s_wtime;  // 上次写入超级块的时间
    _u16 s_mnt_count;  // 未进行检查的挂载次数
    _i16 s_max_mnt_count;  // 必须执行检查前的最大挂载次数（可为-1）
    _u16 s_magic;  // Magic signature 用于表示文件系统的版本，一般为0xEF53
    _u16 s_state;  // 文件系统的状态，包括EXT2_VALID_FS和EXT2_ERROR_FS的或值。
    _u16 s_errors;   // 表明出现错误时要执行的操作
    _u16 s_pad;  // 未使用
    _u32 s_lastcheck;  // 上一次执行检查的时间
    _u32 s_checkinterval; // 两次执行检查的间隔时间
    _u32 s_creator_os;
    _u32 s_rev_level;
    _u16 s_def_resuid;
    _u16 s_def_resgid;

    _u32 s_first_ino;         /* First non-reserved inode */
    _u16 s_inode_size;         /* size of inode structure */
    _u16 s_block_group_nr;     /* block group # of this superblock */
    _u32 s_feature_compat;     /* compatible feature set */
    _u32 s_feature_incompat;     /* incompatible feature set */
    _u32 s_feature_ro_compat;     /* readonly-compatible feature set */
    _u8 s_uuid[16];        /* 128-bit uuid for volume */

    _u8 s_volume_name[16];     /* volume name */
    _u8 s_last_mounted[64];     /* directory where last mounted */

	_u32 algorithm_used;

    _u8 s_prealloc_blocks;    /* Nr of blocks to try to preallocate*/
    _u8 s_prealloc_dir_blocks;    /* Nr to preallocate for dirs */
    _u16 s_padding1;

    _u32 s_reserved[204];    /* Padding to the end of the block */
};

#endif
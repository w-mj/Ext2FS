#pragma once
#include "types.h"
#include "dev/block_dev.h"
#include "defines.h"
#include "semaphore.h"

#include <list>
#include <string>

namespace VFS
{
    class File;

    class Directory {
    public:
        std::list<File*> files;
        std::list<Directory*> directorys;
        
    };


    class FS {
    public:
        Dev::BlockDevice* dev;  // 设备
        _u32 flag;  // 安装标志
        _u32 magic;  // 文件系统标志
        
        Directory* root = nullptr;  // 根目录
        _i32 count;  // 引用计数器

        void** xattr;  // 指向扩展属性的指针
        FS(Dev::BlockDevice* dev): dev(dev) {}
    };
    
    class File {
        _u32 ino;  // 索引节点号
        _i32 counter;  // 引用计数器
        _u16 mode; // 文件类型与访问权限
        _u32 nlink;  // 硬链接数目
        _u32 uid;  // 拥有者
        _u32 gid;  // 组
        Dev::BlockDevice* dev;  // 设备
        _u64 size;  // 文件字节
        _u32 atime;  // 访问时间
        _u32 mtime;  // 写文件时间
        _u32 ctime;  // 修改索引节点时间
        _u32 blkbits;  // 块位数
        _u32 version;  // 版本号（每次使用后递增）
        _u32 blocks;  // 文件的块数
        _u16 bytes;  // 文件最后一个块的字节数
        _u8 sock;  // 如果文件是一个socket则为0
        int lock;  // 自旋锁

        FS* fs;
    public:
        File(FS* fs1):fs(fs1), dev(fs1->dev) {}
        
        void create(Directory*, int mode);  // 为与目录项相关联的普通文件创建一个索引节点

    };
    
}; // namespace VFS

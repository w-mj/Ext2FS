#pragma once
#include "types.h"
#include "dev/block_dev.h"
#include "defines.h"
#include "semaphore.h"

#include <list>
#include <string>

namespace VFS
{

    enum FileType {
        Unknown=0,
        RegularFile,
        Directory,
        CharacterDevice,
        BlockDevice,
        NamedPipe,
        Socket,
        SymbolLink
    };

    class Inode;
    class FS;

    /**
     * 一个目录项
     */
    class DEntry {
    public:
        FS* fs;
        _u32 inode_n;  // inode编号，懒加载
        Inode* inode = nullptr;
        DEntry* parent;
        _u8 type;

        std::string name;

        std::list<DEntry*> children;

        virtual void inflate()=0;
        virtual void load_children()=0;
    };

    enum {
        FS_unmounted = 0,
        FS_mounted
    };

    class FS {
    public:
        Dev::BlockDevice* dev;  // 设备
        // _u32 flag;  // 安装标志
        _u32 magic;  // 文件系统标志

        _u8 status; // 状态， 0为未挂载，1为已挂载
        
        DEntry* root = nullptr;  // 根目录
        _i32 count;  // 引用计数器

        void** xattr;  // 指向扩展属性的指针
        FS(Dev::BlockDevice* dev): dev(dev) {}

        virtual void mount() = 0;
    };
    
    class Inode {
    public:
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
        Inode(FS* fs1):fs(fs1), dev(fs1->dev) {}
        
        void create(DEntry*, int mode);  // 为与目录项相关联的普通文件创建一个索引节点

    };
    
}; // namespace VFS

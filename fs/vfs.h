#pragma once
#include "types.h"
#include "dev/block_dev.h"
#include "defines.h"
#include "semaphore.h"

#include <list>
#include <string>

namespace VFS
{
    class Denty {

    };

    class Inode;

    class FS {
    public:
        Dev::BlockDevice* dev;  // 设备
        _u32 blocksize;  // 块大小
        _u32 old_block_size;  // 基本块设备驱动程序中提到的以字节为单位的块大小
        _u8 blocksize_bits;  // 以位修改的块大小
        _u8 dirt;  // 脏标志
        _u64 maxbytes;  // 文件的最长长度
        _u32 flag;  // 安装标志
        _u32 magic;  // 文件系统标志
        
        Denty* root = nullptr;  // 根目录
        Sem::RW_Semaphore umount;  // 卸载用的信号量
        Sem::Semaphore lock;  // 超级块信号量
        _i32 count;  // 引用计数器
        _i32 syncing;  // 表示超级块的索引节点进行同步
        _i32 need_sync_fs;  // 对超级块已安装的文件系统进行同步的标志
        // Sem::Atomic active;  // 次级块引用计数器
        // _p security;  // 指向超级块安全数据结构的指针
        void** xattr;  // 指向扩展属性的指针
        std::list<Inode> inodes;  // 指向索引节点的链表
        std::list<Inode> dirty_inodes;  // 改进型索引节点的链表？
        std::list<Inode> io_inodes;  // 等待被写入磁盘的链表
        // std::list<void*> anon;  // 用于处理远程网络文件系统的匿名目录项的链表
        std::list<Inode> files;  // 文件项链表
        // std::list<void*> instances;  // 用于给定文件系统类型的超级块对象链表的指针
        // void* dquot;  // 磁盘限额描述符
        // _i32 frozen;  // 冻结文件系统时的使用标志（强制置于一致状态）
        // std::list<void*> wait_unfrozen;  // 进程挂起的等待队列
        // std::string id;  // 包含超级块的设备名称
        // void* fs_info;  // 指向特定文件系统的指针
        _u32 time_gran;  // 时间戳的粒度（纳秒级）

        FS(Dev::BlockDevice* dev): dev(dev) {}

        // virtual Inode* alloc_ionde();  // 分配一个Inode
        // virtual void destroy_inode(Inode*);  // 删除一个Inode
        // virtual void read_inode(Inode*);  // 填充一个Inode
        // virtual void dirty_inode(Inode*);  // 当Inode标记为“脏”时调用，Ext3等用其更新文件系统日志
        // virtual _i32 write_inode(Inode*, _i32);  // 更新一个索引节点，flag表示是否应该同步
        // virtual void put_inode(Inode*);  // 释放一个Inode（减少引用计数）
        // virtual void drop_inode(Inode*);  // 在最后一个用户释放索引节点时调用
        // virtual void delete_inode(Inode*);  // 真正删除一个Inode

        // virtual void put_super();  // 释放超级块对象 
        // virtual void write_super();  // 用指定对象的内容更新文件系统的超级块
    };
    
    class Inode {
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
        
        void create(Denty*, int mode);  // 为与目录项相关联的普通文件创建一个索引节点

    };
    
}; // namespace VFS

#pragma once
#include "ext2_fs.h"
#include "ext2_inode.h"

namespace EXT2
{
    class EXT2_FS;
    class EXT2_Inode;
    class EXT2_File;

    class EXT2_DEntry: public VFS::DEntry {
    private:
        EXT2_FS* ext2_fs;

        void write_children();
        VFS::File *get_file();
    public:
        EXT2_Inode* ext2_inode = nullptr;

        _u8 sync = 0;  // 为1时需要调用inflate从磁盘读取

        // 父文件夹可以与此项目有不同的文件系统
        EXT2_DEntry(EXT2_FS*, VFS::DEntry*, _u32, _u8, const std::string&);
        void inflate();
        void load_children();

        void mkdir(const std::string& name);
        void create(const std::string& name);
        void unlink();  // 删除自己
        void unlink(VFS::DEntry *d);  // 删除子项目


        ~EXT2_DEntry();
    };
} // namespace EXT2

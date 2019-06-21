#pragma once
#include "fs/vfs.h"
#include "ext2_fs.h"
#include "ext2_dentry.h"
#include "ext2_inode.h"
#include "types.h"

namespace EXT2 {
    class EXT2_FS;
    class EXT2_DEntry;
    class EXT2_Inode;

    class EXT2_File: public VFS::File {
        EXT2_DEntry *ext2_dentry;
        EXT2_Inode *ext2_inode;
        EXT2_FS *ext2_fs;
    public:
        EXT2_File(EXT2_DEntry *);
        EXT2_File(EXT2_DEntry *, EXT2_Inode *);
        _u32 tell();
        _u32 seek(int pos, int whence);
        _u32 read(_u8*, _u32 size);
        _u32 write(_u8*, _u32 size);
        void resize(_u32 new_size);
    };
}
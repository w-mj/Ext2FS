#ifndef _VFS_H_
#define _VFS_H_

namespace VFS
{
    
    class Inode {
        int a;
    public:
        virtual void read_inode(Inode*) = 0;
    };
    
}; // namespace VFS

#endif
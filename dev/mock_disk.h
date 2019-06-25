#pragma once

#include "block_dev.h"
#include "fs/vfs.h"
#include <stdio.h>
#include <string>
namespace Dev
{

class MockDisk: public BlockDevice {
private:
    // std::fstream f;
    VFS::File& f;
    _u32 tell();
    _u32 length;
    void seek(_u32 pos);
    std::string name;
public:
    MockDisk(VFS::File& f);
    _u32 read(MM::Buf& buf, _u32 pos, _u32 size);
    _u32 write(MM::Buf& buf, _u32 pos, _u32 size);
    
    void open();
    void close();
};

    
} // namespace Dev

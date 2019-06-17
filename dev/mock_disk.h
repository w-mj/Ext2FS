#pragma once

#include "block_dev.h"
#include <stdio.h>

namespace Dev
{

class MockDisk: public BlockDevice {
private:
    // std::fstream f;
    FILE* f;
public:
    _u32 read(MM::Buf& buf, _u32 size);
    _u32 write(MM::Buf& buf, _u32 size);
    _u32 tell();
    void seek(_u32 pos);
    void open(const std::string& path);
    void close();
};

    
} // namespace Dev

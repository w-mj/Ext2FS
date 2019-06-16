#ifndef _MOCK_DISK_H_
#define _MOCK_DISK_H_
#include "block_dev.h"
#include <fstream>

namespace Dev
{

class MockDisk: BlockDevice {
private:
    std::fstream f;
public:
    _u32 read(MM::Buf& buf, _u32 size);
    _u32 write(MM::Buf& buf, _u32 size);
    _u32 tell();
    void seek(_u32 pos);
    void open(const std::string& path);
    void close();
};

    
} // namespace Dev


#endif
#pragma once
#include "mm/buf.h"
#include "types.h"
#include <string>


namespace Dev {
class BlockDevice {
public:
    virtual _u32 read(MM::Buf& buf, _u32 pos, _u32 size) = 0;
    virtual _u32 write(MM::Buf& buf, _u32 pos, _u32 size) = 0;
    // virtual _u32 tell()=0;
    // virtual void seek(_u32 pos)=0;
    // virtual void open(const std::string& path) = 0;
    virtual void close() = 0;
};

} // namespace Dev

#include "mock_disk.h"
#include <unistd.h>

using namespace Dev;

_u32 MockDisk::read(MM::Buf& buf, _u32 size) {
    f.read(buf.data, size);
    return size;
}

_u32 MockDisk::write(MM::Buf& buf, _u32 size) {
    f.write(buf.data, size);
    return size;
}

_u32 MockDisk::tell() {
    return f.tellg();
}

void MockDisk::seek(_u32 pos) {
    f.seekg(pos);
}

void MockDisk::open(const std::string& path) {
    f.open(path, std::ios::in);
}

void MockDisk::close() {
    if (f.is_open())
        f.close();
}
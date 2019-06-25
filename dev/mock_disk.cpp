#include "mock_disk.h"
#include <unistd.h>
#include <iostream>
#include <cstring>
#include "delog/delog.h"

using namespace Dev;

MockDisk::MockDisk(VFS::File& f): f(f) {}


_u32 MockDisk::read(MM::Buf& buf1, _u32 pos, _u32 size) {
    f.seek(pos, SEEK_SET);
    f.read((char*)buf1.data, size);
    return size;
}

_u32 MockDisk::write(MM::Buf& buf1, _u32 pos, _u32 size) {
    f.seek(pos, SEEK_SET);
    f.write((char*)buf1.data, size);
    return size;
}

_u32 MockDisk::tell() {
    return f.tell();
}

void MockDisk::seek(_u32 pos) {
    //_si(pos);
    // fseek(f, pos, SEEK_SET);
    f.seek(pos, SEEK_SET);
}

void MockDisk::open() {
    std::cout << "正在打开文件" << std::endl;
    length = f.size;
}

void MockDisk::close() {
    f.close();
}
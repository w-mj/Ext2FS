#include "mock_disk.h"
#include <unistd.h>
#include <iostream>
#include "delog/delog.h"

using namespace Dev;

_u32 MockDisk::read(MM::Buf& buf, _u32 size) {
    //std::cout << "\n开始读文件"<<ftell(f)<<std::endl;
    fread(buf.data, size, 1, f);
    // f.read(buf.data, size);
    // for (int i = 0; i < size; i++)
    //     std::cout << (int)buf.data[i];
    return size;
}

_u32 MockDisk::write(MM::Buf& buf, _u32 size) {
    fwrite(buf.data, size, 1, f);
    // f.write(buf.data, size);
    return size;
}

_u32 MockDisk::tell() {
    return ftell(f);
}

void MockDisk::seek(_u32 pos) {
    //_si(pos);
    fseek(f, pos, SEEK_SET);
}

void MockDisk::open(const std::string& path) {
    std::cout << "正在打开文件" << std::endl;
    f = fopen(path.c_str(), "ab+");
    // f.open(path, std::ios::binary);
    if (f == nullptr) {
        // std::cout << "文件打开错误"<< std::endl;
        perror("文件打开错误");
    }
}

void MockDisk::close() {
    fclose(f);
}
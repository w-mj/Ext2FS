#include "mock_disk.h"
#include <unistd.h>
#include <iostream>
#include <cstring>
#include "delog/delog.h"

using namespace Dev;

_u32 MockDisk::read(MM::Buf& buf1, _u32 pos, _u32 size) {
    //std::cout << "\n开始读文件"<<ftell(f)<<std::endl;
    // this->seek(pos);
    // f.read(buf.data, size);
    // for (int i = 0; i < size; i++)
    //     std::cout << (int)buf.data[i];
    // return fread(buf.data, size, 1, f);
    memcpy(buf1.data, buf + pos, size);
    return size;
}

_u32 MockDisk::write(MM::Buf& buf1, _u32 pos, _u32 size) {
    // seek(pos);
    
    // f.write(buf.data, size);
    // return fwrite(buf.data, size, 1, f);
    // _dbg_log("写入文件 old");
    // _si(pos);
    // _si(size);
    //_sa(buf + pos, size);
    memcpy(buf + pos, buf1.data, size);
    //_dbg_log("写入文件 new");
    //_sa(buf + pos, size);
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
    name = path;
    f = fopen(path.c_str(), "rb");
    // f.open(path, std::ios::binary);
    if (f == nullptr) {
        // std::cout << "文件打开错误"<< std::endl;
        perror("文件打开错误");
    }

    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);

    buf = new char[length];
    fread(buf, length, 1, f);
    fclose(f);
}

void MockDisk::close() {
    f = fopen(name.c_str(), "wb");
    _dbg_log("保存磁盘");
    // f.open(path, std::ios::binary);
    if (f == nullptr) {
        // std::cout << "文件打开错误"<< std::endl;
        perror("文件打开错误");
    }
    fwrite(buf, length, 1, f);
    fclose(f);
    delete buf;
}
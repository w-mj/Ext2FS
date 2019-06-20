#pragma once

#include "types.h"
namespace MM {
    class Buf {
    public:
        _u8* data = nullptr;
        Buf(_u32 size) {
            data = new _u8[size];
        }
        _u8* raw() {
            return data;
        }
        ~Buf() {
            delete[] data;
        }
    };
    
} // namespace Buf

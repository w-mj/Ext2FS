#ifndef _BUF_H_
#define _BUF_H_
#include "types.h"
namespace MM {
    class Buf {
    public:
        char* data = nullptr;
        Buf(_u32 size) {
            data = new char[size];
        }
        char* raw() {
            return data;
        }
        ~Buf() {
            delete[] data;
        }
    };
    
} // namespace Buf


#endif
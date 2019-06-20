#pragma once

#define _BITS_SIZE(n) (1 << (n))
#define _BITS_MASK(n) (_BITS_SIZE(n) - 1)
#define _BITSET(n) _BITS_SIZE(n)
#define _get_bit(d, bit) ((d) & _BITSET(bit))
#define _set_bit(d, bit) ((d) |= _BITSET(bit))
#define _clr_bit(d, bit) ((d) &= (~(_BITSET(bit))))
#define _chk_bit(d, bit) _get_bit(d, bit)

#define _rshf(a, n) ((a) << (n))
#define _lshf(a, n) ((a) >> (n))
#define _Lclr(a, n) ((a) & (~(_BITS_MASK(n))))
#define _Hclr(a, n) _rshf(_lshf(a, n), n)
#define _LNclr(a, n) ((a & (_BITS_MASK(n))))

#include "types.h"

// 从低位数第一个1，返回值为sizeof(T) * 8时说明全是0
template<typename T> static inline _u8 lowest_1(T n) {
    _u8 i = 0;
    _u8 bits_cnt = sizeof(T) << 3;
    T mask = 1;
    while (i < bits_cnt) {
        if (mask & n)
            return i;
        i++;
        mask <<= 1;
    }
    return i;
}

template<typename T> static inline _u8 highest_1(T n) {
    _u8 bits_cnt = sizeof(T) << 3;
    int i = bits_cnt - 1;
    T mask = 1 << (bits_cnt - 1);
    while (i < bits_cnt) {
        if (mask & n)
            return i;
        i--;
        mask >>= 1;
    }
    return bits_cnt;
}

template<typename T> static inline _u8 lowest_0(T n) {
    return lowest_1(~n);
}

template<typename T> static inline _u8 highest_0(T n) {
    return highest_1(~n);
}
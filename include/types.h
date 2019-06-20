#pragma once


typedef signed char _i8;
typedef unsigned char _u8;
typedef signed short _i16;
typedef unsigned short _u16;
typedef signed int _i32;
typedef unsigned int _u32;
typedef signed long _i64;
typedef unsigned long _u64;
typedef _u32 _I;
typedef char _c;
typedef char* _s;
typedef void* _p;

#ifdef __CHECKER__
#define __bitwise __attribute__((bitwise))
#else
#define __bitwise
#endif

typedef _u16 __bitwise _le16;
typedef _u16 __bitwise _be16;
typedef _u32 __bitwise _le32;
typedef _u32 __bitwise _be32;
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
typedef _u64 __bitwise _le64;
typedef _u64 __bitwise _be64;
#endif

#define _getva_T_(P, params, _T_) do {P = (_T_)va_arg(params, _T_); }while(0)
#define _getva_S(s, params) _getva_T_(s, params, _s)
#define _getva_I(s, params) _getva_T_(s, params, _I)
#define _getva_P(s, params) _getva_T_(s, params, _p)
#define _getva_double(s, params) _getva_T_(s, params, double)
#define _getva_I32(s, params) _getva_T_(s, params, _i32)


#define _Inext(pI, i) ((pI) + (i))->_next
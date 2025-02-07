#ifndef HZ_TYPES_H
#define HZ_TYPES_H

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef long long s64;
typedef int s32;
typedef short s16;
typedef char s8;

typedef double f64;
typedef float f32;

typedef bool b8;
typedef s32 bool32;

#define U32CODE(a, b, c, d) (((u32)(a) << 0) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24)) 

#endif
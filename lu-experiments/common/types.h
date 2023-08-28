#pragma once

typedef unsigned char     u8;
typedef unsigned short    u16;
typedef unsigned long int u32;

typedef signed char     s8;
typedef signed short    s16;
typedef signed long int s32;

typedef unsigned char  bool8;
typedef unsigned short bool16;
typedef unsigned long int bool32;

#define TRUE 1
#define FALSE 0
#define NULL 0

#define ALIGNED(n) 

// strip GCC stuff for basic tests
#define __attribute__(...)

/*//
// shims for basic tests
#define INCBIN(...) {0}
#define INCBIN_U8   INCBIN
#define INCBIN_U16  INCBIN
#define INCBIN_U32  INCBIN
#define INCBIN_S8   INCBIN
#define INCBIN_S16  INCBIN
#define INCBIN_S32  INCBIN
//*/
#pragma once

#include <sys/types.h>
#include "MacroX.h"
/* Even in pure C, we still need a standard boolean typedef */
#ifndef __cplusplus
typedef unsigned char bool;
#define true    1
#define false   0
#endif  // __cplusplus

#ifndef NULL
#define NULL    0
#endif

/* define int types*/
typedef     signed char         int8_t;
typedef     signed short        int16_t;
typedef     signed long			int32_t;
typedef     signed __int64      int64_t;
typedef     unsigned char       uint8_t;
typedef     unsigned short      uint16_t;
typedef     unsigned long       uint32_t;
typedef     unsigned __int64    uint64_t;

/* the following definitions are from VS2010's stdint.h */
#ifndef _INTPTR_T_DEFINED
#define _INTPTR_T_DEFINED
#ifdef _WIN64
typedef __int64 intptr_t;
#else /* _WIN64 */
typedef _W64 int intptr_t;
#endif /* _WIN64 */
#endif /* _INTPTR_T_DEFINED */

#ifndef _UINTPTR_T_DEFINED
#define _UINTPTR_T_DEFINED
#ifdef _WIN64
typedef unsigned __int64 uintptr_t;
#else /* _WIN64 */
typedef _W64 unsigned int uintptr_t;
#endif /* _WIN64 */
#endif /* _UINTPTR_T_DEFINED */

const uint8_t  kUint8Max  = (( uint8_t) 0xFF);
const uint16_t kUint16Max = ((uint16_t) 0xFFFF);
const uint32_t kUint32Max = ((uint32_t) 0xFFFFFFFF);
const uint64_t kUint64Max = ((uint64_t) GG_LONGLONG(0xFFFFFFFFFFFFFFFF));
const int8_t   kInt8Min   = ((  int8_t) 0x80);
const int8_t   kInt8Max   = ((  int8_t) 0x7F);
const int16_t  kInt16Min  = (( int16_t) 0x8000);
const int16_t  kInt16Max  = (( int16_t) 0x7FFF);
const int32_t  kInt32Min  = (( int32_t) 0x80000000);
const int32_t  kInt32Max  = (( int32_t) 0x7FFFFFFF);
const int64_t  kInt64Min  = (( int64_t) GG_LONGLONG(0x8000000000000000));
const int64_t  kInt64Max  = (( int64_t) GG_LONGLONG(0x7FFFFFFFFFFFFFFF));
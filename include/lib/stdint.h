#ifndef STDINT_H
#define STDINT_H
typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef signed short       int16_t;
typedef unsigned short     uint16_t;
typedef signed int         int32_t;
typedef unsigned int       uint32_t;
typedef signed long long   int64_t;
typedef unsigned long long uint64_t;


#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)
typedef uint64_t uintptr_t;
typedef uint64_t size_t;
#else
typedef uint32_t uintptr_t;
typedef uint32_t size_t;
#endif

#endif

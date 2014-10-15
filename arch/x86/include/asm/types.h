#ifndef __ASM_I386_TYPES_H
#define __ASM_I386_TYPES_H

typedef unsigned short umode_t;

/*
 * __xx is ok: it doesn't pollute the POSIX namespace. Use these in the
 * header files exported to user space
 */

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

#if defined(__GNUC__)
__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;
#endif

/*
 * These aren't exported outside the kernel to avoid name space clashes
 */
#ifdef __KERNEL__

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

#if !defined(CONFIG_USE_STDINT) || !defined(__INT64_TYPE__)
typedef signed long long s64;
typedef unsigned long long u64;
#else
typedef __INT64_TYPE__ s64;
typedef __UINT64_TYPE__ u64;
#endif

#define BITS_PER_LONG 32

/* Dma addresses are 32-bits wide.  */

typedef u32 dma_addr_t;

typedef unsigned long long phys_addr_t;
typedef unsigned long long phys_size_t;

#endif /* __KERNEL__ */

#endif

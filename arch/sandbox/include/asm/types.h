/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_SANDBOX_TYPES_H
#define __ASM_SANDBOX_TYPES_H

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

/*
 * Number of bits in a C 'long' on this architecture.
 */
#ifdef	CONFIG_PHYS64
#define BITS_PER_LONG 64
#else	/* CONFIG_PHYS64 */
#define BITS_PER_LONG 32
#endif	/* CONFIG_PHYS64 */

#ifdef	CONFIG_PHYS64
typedef unsigned long long dma_addr_t;
typedef u64 phys_addr_t;
typedef u64 phys_size_t;
#else	/* CONFIG_PHYS64 */
typedef unsigned long dma_addr_t;
typedef u32 phys_addr_t;
typedef u32 phys_size_t;
#endif	/* CONFIG_PHYS64 */

#endif /* __KERNEL__ */

#endif

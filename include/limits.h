/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _LIMITS_H
#define _LIMITS_H

#define SCHAR_MAX   __SCHAR_MAX__
#define SCHAR_MIN   (-SCHAR_MAX - 1)
#define UCHAR_MAX   (SCHAR_MAX * 2 + 1)

#ifdef __CHAR_UNSIGNED__
#define CHAR_MAX    UCHAR_MAX
#define CHAR_MIN    0
#else
#define CHAR_MAX    SCHAR_MAX
#define CHAR_MIN    SCHAR_MIN
#endif

#define SHRT_MAX    __SHRT_MAX__
#define SHRT_MIN    (-SHRT_MAX - 1)
#define USHRT_MAX   (SHRT_MAX * 2 + 1)

#define INT_MAX     __INT_MAX__
#define INT_MIN     (-INT_MAX - 1)
#define UINT_MAX    (INT_MAX * 2U + 1U)

#define LONG_MAX    __LONG_MAX__
#define LONG_MIN    (-LONG_MAX - 1L)
#define ULONG_MAX   (LONG_MAX * 2UL + 1UL)

#define LLONG_MAX   __LONG_LONG_MAX__
#define LLONG_MIN   (-LLONG_MAX - 1LL)
#define ULLONG_MAX  (LLONG_MAX * 2ULL + 1ULL)

#define CHAR_BIT    8
#define UINT32_MAX  0xffffffffU
#define UINT64_MAX  0xffffffffffffffffULL

#if (defined(CONFIG_64BIT) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_64BIT) && defined(CONFIG_SPL_BUILD))
    #define UINTPTR_MAX UINT64_MAX
#else
    #define UINTPTR_MAX UINT32_MAX
#endif

#ifndef SIZE_MAX
#define SIZE_MAX    UINTPTR_MAX
#endif
#ifndef SSIZE_MAX
#define SSIZE_MAX   ((ssize_t)(SIZE_MAX >> 1))
#endif

#endif /* _LIMITS_H */

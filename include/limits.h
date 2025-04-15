/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _LIMITS_H
#define _LIMITS_H

#define INT_MAX     0x7fffffff
#define UINT_MAX    0xffffffffU
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

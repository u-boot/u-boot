/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2023 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */

#ifndef _LIMITS_H
#define _LIMITS_H

#define INT_MAX         0x7fffffff
#define UINT_MAX	0xffffffffUL
#define CHAR_BIT        8
#define UINT32_MAX      0xffffffffUL
#define UINT64_MAX	0xffffffffffffffffUL

#ifdef CONFIG_64BIT
    #define UINTPTR_MAX UINT64_MAX
#else
    #define UINTPTR_MAX UINT32_MAX
#endif

#ifndef SIZE_MAX
#define SIZE_MAX        UINTPTR_MAX
#endif
#ifndef SSIZE_MAX
#define SSIZE_MAX	((ssize_t)(SIZE_MAX >> 1))
#endif

#endif /* _LIMITS_H */

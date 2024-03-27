/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Dummy file to allow mbedtls linked with U-Boot to include limits.h
 *
 * Copyright (c) 2023 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */

#ifndef _MBEDTLS_LIMITS_H
#define _MBEDTLS_LIMITS_H

#undef INT_MAX
#define INT_MAX         0x7fffffff

#undef UINT_MAX
#define UINT_MAX	0xffffffffUL

#undef CHAR_BIT
#define CHAR_BIT        8

#undef UINT32_MAX
#define UINT32_MAX      0xffffffffUL

#undef UINTPTR_MAX
#define UINTPTR_MAX     0xffffffffffffffffUL

#undef SIZE_MAX
#define SIZE_MAX        UINTPTR_MAX

#undef UINT64_MAX
#define UINT64_MAX      UINTPTR_MAX

#endif /* _MBEDTLS_LIMITS_H */

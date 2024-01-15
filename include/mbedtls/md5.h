/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2023 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#include <external/mbedtls/include/mbedtls/md5.h>

#ifndef _MBEDTLS_MD5_H
#define _MBEDTLS_MD5_H

#define MD5_SUM_LEN	16

void
md5_wd_mb(const unsigned char *input, unsigned int len,
	  unsigned char output[16],
	  unsigned int __always_unused chunk_sz);

#endif /* _MBEDTLS_MD5_H */

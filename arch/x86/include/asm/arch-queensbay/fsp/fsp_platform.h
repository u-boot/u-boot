/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	Intel
 */

#ifndef __FSP_PLATFORM_H__
#define __FSP_PLATFORM_H__

#pragma pack(1)

struct fspinit_rtbuf_t {
	struct common_buf_t	common;	/* FSP common runtime data structure */
};

#pragma pack()

#endif

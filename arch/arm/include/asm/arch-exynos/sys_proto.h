/*
 * Copyright (C) 2010 Samsung Electrnoics
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

u32 get_device_type(void);
void invalidate_dcache(u32);
void l2_cache_disable(void);
void l2_cache_enable(void);

#endif

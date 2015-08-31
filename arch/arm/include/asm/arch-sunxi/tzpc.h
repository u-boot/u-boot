/*
 * (C) Copyright 2015 Chen-Yu Tsai <wens@csie.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_TZPC_H
#define _SUNXI_TZPC_H

#ifndef __ASSEMBLY__
struct sunxi_tzpc {
	u32 r0size;		/* 0x00 Size of secure RAM region */
	u32 decport0_status;	/* 0x04 Status of decode protection port 0 */
	u32 decport0_set;	/* 0x08 Set decode protection port 0 */
	u32 decport0_clear;	/* 0x0c Clear decode protection port 0 */
};
#endif

#define SUNXI_TZPC_DECPORT0_RTC	(1 << 1)

void tzpc_init(void);

#endif /* _SUNXI_TZPC_H */

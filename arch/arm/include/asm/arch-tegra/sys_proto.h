/*
 * (C) Copyright 2010,2011
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

struct tegra_sysinfo {
	char *board_string;
};

void invalidate_dcache(void);

extern const struct tegra_sysinfo sysinfo;

#endif

/*
 * (C) Copyright 2012 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * (C) Copyright 2012 Renesas Solutions Corp.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>

#ifndef CONFIG_RCAR_GEN3
int checkboard(void)
{
	printf("Board: %s\n", sysinfo.board_string);
	return 0;
}
#endif

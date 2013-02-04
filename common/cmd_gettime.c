/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 *
 * Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Get Timer overflows after 2^32 / CONFIG_SYS_HZ (32Khz) = 131072 sec
 */
#include <common.h>
#include <command.h>

static int do_gettime(cmd_tbl_t *cmdtp, int flag, int argc,
		      char * const argv[])
{
	unsigned long int val = get_timer(0);

#ifdef CONFIG_SYS_HZ
	printf("Timer val: %lu\n", val);
	printf("Seconds : %lu\n", val / CONFIG_SYS_HZ);
	printf("Remainder : %lu\n", val % CONFIG_SYS_HZ);
	printf("sys_hz = %lu\n", (unsigned long int)CONFIG_SYS_HZ);
#else
	printf("CONFIG_SYS_HZ not defined");
	printf("Timer Val %lu", val);
#endif

	return 0;
}

U_BOOT_CMD(
	gettime,	1,	1,	do_gettime,
	"get timer val elapsed,\n",
	"get time elapsed from uboot start\n"
);

/*
 * U-boot - bf533_linux.c
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

/* Dummy functions, currently not in Use */

#include <common.h>
#include <command.h>
#include <image.h>
#include <zlib.h>
#include <asm/byteorder.h>

#define	LINUX_MAX_ENVS		256
#define	LINUX_MAX_ARGS		256

#define CMD_LINE_ADDR 0xFF900000	/* L1 scratchpad */

#ifdef SHARED_RESOURCES
extern void swap_to(int device_id);
#endif

extern void flush_instruction_cache(void);
extern void flush_data_cache(void);
static char *make_command_line(void);

void do_bootm_linux(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[],
		    bootm_headers_t *images)
{
	int	(*appl) (char *cmdline);
	char	*cmdline;
	ulong	ep = 0;

#ifdef SHARED_RESOURCES
	swap_to(FLASH);
#endif

	/* find kernel entry point */
	if (images->legacy_hdr_valid) {
		ep = image_get_ep (images->legacy_hdr_os);
#if defined(CONFIG_FIT)
	} else if (images->fit_uname_os) {
		fit_unsupported_reset ("AVR32 linux bootm");
		do_reset (cmdtp, flag, argc, argv);
#endif
	} else {
		puts ("Could not find kernel entry point!\n");
		do_reset (cmdtp, flag, argc, argv);
	}
	appl = (int (*)(char *))ep;

	printf("Starting Kernel at = %x\n", appl);
	cmdline = make_command_line();
	if (icache_status()) {
		flush_instruction_cache();
		icache_disable();
	}
	if (dcache_status()) {
		flush_data_cache();
		dcache_disable();
	}
	(*appl) (cmdline);
}

char *make_command_line(void)
{
	char *dest = (char *)CMD_LINE_ADDR;
	char *bootargs;

	if ((bootargs = getenv("bootargs")) == NULL)
		return NULL;

	strncpy(dest, bootargs, 0x1000);
	dest[0xfff] = 0;
	return dest;
}

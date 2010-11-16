/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (c) Copyright 2008 Nobuhiro Iwamatsu <iwamatsu.nobuhiro@renesas.com>
 * (c) Copyright 2008 Renesas Solutions Corp.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <common.h>
#include <command.h>
#include <asm/byteorder.h>

#ifdef CONFIG_SYS_DEBUG
static void hexdump(unsigned char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if ((i % 16) == 0)
			printf("%s%08x: ", i ? "\n" : "",
							(unsigned int)&buf[i]);
		printf("%02x ", buf[i]);
	}
	printf("\n");
}
#endif

#define MOUNT_ROOT_RDONLY	0x000
#define RAMDISK_FLAGS		0x004
#define ORIG_ROOT_DEV		0x008
#define LOADER_TYPE			0x00c
#define INITRD_START		0x010
#define INITRD_SIZE			0x014
#define COMMAND_LINE		0x100

#define RD_PROMPT	(1<<15)
#define RD_DOLOAD	(1<<14)
#define CMD_ARG_RD_PROMPT	"prompt_ramdisk="
#define CMD_ARG_RD_DOLOAD	"load_ramdisk="

#ifdef CONFIG_SH_SDRAM_OFFSET
#define GET_INITRD_START(initrd, linux) (initrd - linux + CONFIG_SH_SDRAM_OFFSET)
#else
#define GET_INITRD_START(initrd, linux) (initrd - linux)
#endif

static void set_sh_linux_param(unsigned long param_addr, unsigned long data)
{
	*(unsigned long *)(param_addr) = data;
}

static unsigned long sh_check_cmd_arg(char *cmdline, char *key, int base)
{
	unsigned long val = 0;
	char *p = strstr(cmdline, key);
	if (p) {
		p += strlen(key);
		val = simple_strtol(p, NULL, base);
	}
	return val;
}

int do_bootm_linux(int flag, int argc, char * const argv[], bootm_headers_t *images)
{
	/* Linux kernel load address */
	void (*kernel) (void) = (void (*)(void))images->ep;
	/* empty_zero_page */
	unsigned char *param
		= (unsigned char *)image_get_load(images->legacy_hdr_os);
	/* Linux kernel command line */
	char *cmdline = (char *)param + COMMAND_LINE;
	/* PAGE_SIZE */
	unsigned long size = images->ep - (unsigned long)param;
	char *bootargs = getenv("bootargs");

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	/* Setup parameters */
	memset(param, 0, size);	/* Clear zero page */

	/* Set commandline */
	strcpy(cmdline, bootargs);

	sh_check_cmd_arg(bootargs, CMD_ARG_RD_DOLOAD, 10);
	/* Initrd */
	if (images->rd_start || images->rd_end) {
		unsigned long ramdisk_flags = 0;
		int val = sh_check_cmd_arg(bootargs, CMD_ARG_RD_PROMPT, 10);
		if (val == 1)
				ramdisk_flags |= RD_PROMPT;
		else
				ramdisk_flags &= ~RD_PROMPT;

		val = sh_check_cmd_arg(bootargs, CMD_ARG_RD_DOLOAD, 10);
		if (val == 1)
				ramdisk_flags |= RD_DOLOAD;
		else
				ramdisk_flags &= ~RD_DOLOAD;

		set_sh_linux_param((unsigned long)param + MOUNT_ROOT_RDONLY, 0x0001);
		set_sh_linux_param((unsigned long)param + RAMDISK_FLAGS, ramdisk_flags);
		set_sh_linux_param((unsigned long)param + ORIG_ROOT_DEV, 0x0200);
		set_sh_linux_param((unsigned long)param + LOADER_TYPE, 0x0001);
		set_sh_linux_param((unsigned long)param + INITRD_START,
			GET_INITRD_START(images->rd_start, CONFIG_SYS_SDRAM_BASE));
		set_sh_linux_param((unsigned long)param + INITRD_SIZE,
			images->rd_end - images->rd_start);
	}

	/* Boot kernel */
	kernel();
	/* does not return */

	return 1;
}

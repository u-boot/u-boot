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

int do_bootm_linux(int flag, int argc, char *argv[], bootm_headers_t *images)
{
	/* Linux kernel load address */
	void (*kernel) (void) = (void (*)(void))images->ep;
	/* empty_zero_page */
	unsigned char *param
		= (unsigned char *)image_get_load(images->legacy_hdr_os);
	/* Linux kernel command line */
	char *cmdline = (char *)param + 0x100;
	/* PAGE_SIZE */
	unsigned long size = images->ep - (unsigned long)param;
	char *bootargs = getenv("bootargs");

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	/* Setup parameters */
	memset(param, 0, size);	/* Clear zero page */
	strcpy(cmdline, bootargs);

	kernel();
	/* does not return */

	return 1;
}

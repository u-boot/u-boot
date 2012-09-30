/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2011
 * Texas Instruments, <www.ti.com>
 *
 * Matt Porter <mporter@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <spl.h>
#include <xyzModem.h>
#include <asm/u-boot.h>
#include <asm/utils.h>

#define BUF_SIZE 1024

static int getcymodem(void) {
	if (tstc())
		return (getc());
	return -1;
}

void spl_ymodem_load_image(void)
{
	int size = 0;
	int err;
	int res;
	int ret;
	connection_info_t info;
	char buf[BUF_SIZE];
	ulong store_addr = ~0;
	ulong addr = 0;

	info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&info, &err);

	if (!ret) {
		while ((res =
			xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			if (addr == 0)
				spl_parse_image_header((struct image_header *)buf);
			store_addr = addr + spl_image.load_addr;
			size += res;
			addr += res;
			memcpy((char *)(store_addr), buf, res);
		}
	} else {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		hang();
	}

	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	printf("Loaded %d bytes\n", size);
}

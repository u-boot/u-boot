/*
 * (C) Copyright 2002
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
 *
 * adapted for VCMA9
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
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
 *
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include "vcma9.h"
#include "../common/common_util.h"

#if defined(CONFIG_CS8900)
#include <../drivers/net/cs8900.h>

static uchar cs8900_chksum(ushort data)
{
	return((data >> 8) & 0x00FF) + (data & 0x00FF);
}

#endif

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */

int do_vcma9(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct eth_device *dev;
	char cs8900_name[10];
	if (strcmp(argv[1], "info") == 0)
	{
		vcma9_print_info();
		return 0;
	}
#if defined(CONFIG_CS8900)
	if (strcmp(argv[1], "cs8900") == 0) {
		sprintf(cs8900_name, "%s-0", CS8900_DRIVERNAME);
		dev = eth_get_dev_by_name(cs8900_name);
		if (!dev) {
			printf("Couldn't find CS8900 driver");
			return 0;
		}
		if (strcmp(argv[2], "read") == 0) {
			uchar addr; ushort data;

			addr = simple_strtoul(argv[3], NULL, 16);
			cs8900_e2prom_read(dev, addr, &data);
			printf("0x%2.2X: 0x%4.4X\n", addr, data);
		} else if (strcmp(argv[2], "write") == 0) {
			uchar addr; ushort data;

			addr = simple_strtoul(argv[3], NULL, 16);
			data = simple_strtoul(argv[4], NULL, 16);
			cs8900_e2prom_write(dev, addr, data);
		} else if (strcmp(argv[2], "setaddr") == 0) {
			uchar addr, i, csum; ushort data;
			uchar ethaddr[6];

			/* check for valid ethaddr */
			if (eth_getenv_enetaddr("ethaddr", ethaddr)) {
				addr = 1;
				data = 0x2158;
				cs8900_e2prom_write(dev, addr, data);
				csum = cs8900_chksum(data);
				addr++;
				for (i = 0; i < 6; i+=2) {
					data = ethaddr[i+1] << 8 |
					       ethaddr[i];
					cs8900_e2prom_write(dev, addr, data);
					csum += cs8900_chksum(data);
					addr++;
				}
				/* calculate header link byte */
				data = 0xA100 | (addr * 2);
				cs8900_e2prom_write(dev, 0, data);
				csum += cs8900_chksum(data);
				/* write checksum word */
				cs8900_e2prom_write(dev, addr, (0 - csum) << 8);
			} else {
				puts("\nplease defined 'ethaddr'\n");
			}
		} else if (strcmp(argv[2], "dump") == 0) {
			uchar addr = 0, endaddr, csum; ushort data;

			puts("Dump of CS8900 config device: ");
			cs8900_e2prom_read(dev, addr, &data);
			if ((data & 0xE000) == 0xA000) {
				endaddr = (data & 0x00FF) / 2;
				csum = cs8900_chksum(data);
				for (addr = 1; addr <= endaddr; addr++) {
					cs8900_e2prom_read(dev, addr, &data);
					printf("\n0x%2.2X: 0x%4.4X", addr, data);
					csum += cs8900_chksum(data);
				}
				printf("\nChecksum: %s", (csum == 0) ? "ok" : "wrong");
			} else {
				puts("no valid config found");
			}
			puts("\n");
		}

		return 0;
	}
#endif

	return (do_mplcommon(cmdtp, flag, argc, argv));
}

U_BOOT_CMD(
	vcma9, 6, 1, do_vcma9,
	"VCMA9 specific commands",
	"flash mem [SrcAddr] - updates U-Boot with image in memory\n"
	"vcma9 info                - displays board information"
);

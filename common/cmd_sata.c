/*
 * Copyright (C) 2000-2005, DENX Software Engineering
 *		Wolfgang Denk <wd@denx.de>
 * Copyright (C) Procsys. All rights reserved.
 *		Mushtaq Khan <mushtaq_k@procsys.com>
 *			<mushtaqk_921@yahoo.co.in>
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
 *		Dave Liu <daveliu@freescale.com>
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
#include <command.h>
#include <part.h>
#include <sata.h>

int sata_curr_device = -1;
block_dev_desc_t sata_dev_desc[CONFIG_SYS_SATA_MAX_DEVICE];

int __sata_initialize(void)
{
	int rc;
	int i;

	for (i = 0; i < CONFIG_SYS_SATA_MAX_DEVICE; i++) {
		memset(&sata_dev_desc[i], 0, sizeof(struct block_dev_desc));
		sata_dev_desc[i].if_type = IF_TYPE_SATA;
		sata_dev_desc[i].dev = i;
		sata_dev_desc[i].part_type = PART_TYPE_UNKNOWN;
		sata_dev_desc[i].type = DEV_TYPE_HARDDISK;
		sata_dev_desc[i].lba = 0;
		sata_dev_desc[i].blksz = 512;
		sata_dev_desc[i].block_read = sata_read;
		sata_dev_desc[i].block_write = sata_write;

		rc = init_sata(i);
		rc = scan_sata(i);
		if ((sata_dev_desc[i].lba > 0) && (sata_dev_desc[i].blksz > 0))
			init_part(&sata_dev_desc[i]);
	}
	sata_curr_device = 0;
	return rc;
}
int sata_initialize(void) __attribute__((weak,alias("__sata_initialize")));

block_dev_desc_t *sata_get_dev(int dev)
{
	return (dev < CONFIG_SYS_SATA_MAX_DEVICE) ? &sata_dev_desc[dev] : NULL;
}

int do_sata(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int rc = 0;

	if (argc == 2 && strcmp(argv[1], "init") == 0)
		return sata_initialize();

	/* If the user has not yet run `sata init`, do it now */
	if (sata_curr_device == -1)
		if (sata_initialize())
			return 1;

	switch (argc) {
	case 0:
	case 1:
		cmd_usage(cmdtp);
		return 1;
	case 2:
		if (strncmp(argv[1],"inf", 3) == 0) {
			int i;
			putc('\n');
			for (i = 0; i < CONFIG_SYS_SATA_MAX_DEVICE; ++i) {
				if (sata_dev_desc[i].type == DEV_TYPE_UNKNOWN)
					continue;
				printf ("SATA device %d: ", i);
				dev_print(&sata_dev_desc[i]);
			}
			return 0;
		} else if (strncmp(argv[1],"dev", 3) == 0) {
			if ((sata_curr_device < 0) || (sata_curr_device >= CONFIG_SYS_SATA_MAX_DEVICE)) {
				puts("\nno SATA devices available\n");
				return 1;
			}
			printf("\nSATA device %d: ", sata_curr_device);
			dev_print(&sata_dev_desc[sata_curr_device]);
			return 0;
		} else if (strncmp(argv[1],"part",4) == 0) {
			int dev, ok;

			for (ok = 0, dev = 0; dev < CONFIG_SYS_SATA_MAX_DEVICE; ++dev) {
				if (sata_dev_desc[dev].part_type != PART_TYPE_UNKNOWN) {
					++ok;
					if (dev)
						putc ('\n');
					print_part(&sata_dev_desc[dev]);
				}
			}
			if (!ok) {
				puts("\nno SATA devices available\n");
				rc ++;
			}
			return rc;
		}
		cmd_usage(cmdtp);
		return 1;
	case 3:
		if (strncmp(argv[1], "dev", 3) == 0) {
			int dev = (int)simple_strtoul(argv[2], NULL, 10);

			printf("\nSATA device %d: ", dev);
			if (dev >= CONFIG_SYS_SATA_MAX_DEVICE) {
				puts ("unknown device\n");
				return 1;
			}
			dev_print(&sata_dev_desc[dev]);

			if (sata_dev_desc[dev].type == DEV_TYPE_UNKNOWN)
				return 1;

			sata_curr_device = dev;

			puts("... is now current device\n");

			return 0;
		} else if (strncmp(argv[1], "part", 4) == 0) {
			int dev = (int)simple_strtoul(argv[2], NULL, 10);

			if (sata_dev_desc[dev].part_type != PART_TYPE_UNKNOWN) {
				print_part(&sata_dev_desc[dev]);
			} else {
				printf("\nSATA device %d not available\n", dev);
				rc = 1;
			}
			return rc;
		}
		cmd_usage(cmdtp);
		return 1;

	default: /* at least 4 args */
		if (strcmp(argv[1], "read") == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong cnt = simple_strtoul(argv[4], NULL, 16);
			ulong n;
			lbaint_t blk = simple_strtoul(argv[3], NULL, 16);

			printf("\nSATA read: device %d block # %ld, count %ld ... ",
				sata_curr_device, blk, cnt);

			n = sata_read(sata_curr_device, blk, cnt, (u32 *)addr);

			/* flush cache after read */
			flush_cache(addr, cnt * sata_dev_desc[sata_curr_device].blksz);

			printf("%ld blocks read: %s\n",
				n, (n==cnt) ? "OK" : "ERROR");
			return (n == cnt) ? 0 : 1;
		} else if (strcmp(argv[1], "write") == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong cnt = simple_strtoul(argv[4], NULL, 16);
			ulong n;

			lbaint_t blk = simple_strtoul(argv[3], NULL, 16);

			printf("\nSATA write: device %d block # %ld, count %ld ... ",
				sata_curr_device, blk, cnt);

			n = sata_write(sata_curr_device, blk, cnt, (u32 *)addr);

			printf("%ld blocks written: %s\n",
				n, (n == cnt) ? "OK" : "ERROR");
			return (n == cnt) ? 0 : 1;
		} else {
			cmd_usage(cmdtp);
			rc = 1;
		}

		return rc;
	}
}

U_BOOT_CMD(
	sata, 5, 1, do_sata,
	"SATA sub system",
	"sata init - init SATA sub system\n"
	"sata info - show available SATA devices\n"
	"sata device [dev] - show or set current device\n"
	"sata part [dev] - print partition table\n"
	"sata read addr blk# cnt\n"
	"sata write addr blk# cnt"
);

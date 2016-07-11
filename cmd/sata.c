/*
 * Copyright (C) 2000-2005, DENX Software Engineering
 *		Wolfgang Denk <wd@denx.de>
 * Copyright (C) Procsys. All rights reserved.
 *		Mushtaq Khan <mushtaq_k@procsys.com>
 *			<mushtaqk_921@yahoo.co.in>
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
 *		Dave Liu <daveliu@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <part.h>
#include <sata.h>

static int sata_curr_device = -1;
struct blk_desc sata_dev_desc[CONFIG_SYS_SATA_MAX_DEVICE];

static unsigned long sata_bread(struct blk_desc *block_dev, lbaint_t start,
				lbaint_t blkcnt, void *dst)
{
	return sata_read(block_dev->devnum, start, blkcnt, dst);
}

static unsigned long sata_bwrite(struct blk_desc *block_dev, lbaint_t start,
				 lbaint_t blkcnt, const void *buffer)
{
	return sata_write(block_dev->devnum, start, blkcnt, buffer);
}

int __sata_initialize(void)
{
	int rc;
	int i;

	for (i = 0; i < CONFIG_SYS_SATA_MAX_DEVICE; i++) {
		memset(&sata_dev_desc[i], 0, sizeof(struct blk_desc));
		sata_dev_desc[i].if_type = IF_TYPE_SATA;
		sata_dev_desc[i].devnum = i;
		sata_dev_desc[i].part_type = PART_TYPE_UNKNOWN;
		sata_dev_desc[i].type = DEV_TYPE_HARDDISK;
		sata_dev_desc[i].lba = 0;
		sata_dev_desc[i].blksz = 512;
		sata_dev_desc[i].log2blksz = LOG2(sata_dev_desc[i].blksz);
		sata_dev_desc[i].block_read = sata_bread;
		sata_dev_desc[i].block_write = sata_bwrite;

		rc = init_sata(i);
		if (!rc) {
			rc = scan_sata(i);
			if (!rc && (sata_dev_desc[i].lba > 0) &&
				(sata_dev_desc[i].blksz > 0))
				part_init(&sata_dev_desc[i]);
		}
	}
	sata_curr_device = 0;
	return rc;
}
int sata_initialize(void) __attribute__((weak,alias("__sata_initialize")));

__weak int __sata_stop(void)
{
	int i, err = 0;

	for (i = 0; i < CONFIG_SYS_SATA_MAX_DEVICE; i++)
		err |= reset_sata(i);

	if (err)
		printf("Could not reset some SATA devices\n");

	return err;
}
int sata_stop(void) __attribute__((weak, alias("__sata_stop")));

#ifdef CONFIG_PARTITIONS
struct blk_desc *sata_get_dev(int dev)
{
	return (dev < CONFIG_SYS_SATA_MAX_DEVICE) ? &sata_dev_desc[dev] : NULL;
}
#endif

static int do_sata(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rc = 0;

	if (argc == 2 && strcmp(argv[1], "stop") == 0)
		return sata_stop();

	if (argc == 2 && strcmp(argv[1], "init") == 0) {
		if (sata_curr_device != -1)
			sata_stop();

		return sata_initialize();
	}

	/* If the user has not yet run `sata init`, do it now */
	if (sata_curr_device == -1)
		if (sata_initialize())
			return 1;

	switch (argc) {
	case 0:
	case 1:
		return CMD_RET_USAGE;
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
					part_print(&sata_dev_desc[dev]);
				}
			}
			if (!ok) {
				puts("\nno SATA devices available\n");
				rc ++;
			}
			return rc;
		}
		return CMD_RET_USAGE;
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
				part_print(&sata_dev_desc[dev]);
			} else {
				printf("\nSATA device %d not available\n", dev);
				rc = 1;
			}
			return rc;
		}
		return CMD_RET_USAGE;

	default: /* at least 4 args */
		if (strcmp(argv[1], "read") == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong cnt = simple_strtoul(argv[4], NULL, 16);
			ulong n;
			lbaint_t blk = simple_strtoul(argv[3], NULL, 16);

			printf("\nSATA read: device %d block # %ld, count %ld ... ",
				sata_curr_device, blk, cnt);

			n = blk_dread(&sata_dev_desc[sata_curr_device],
				      blk, cnt, (u32 *)addr);

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

			n = blk_dwrite(&sata_dev_desc[sata_curr_device],
				       blk, cnt, (u32 *)addr);

			printf("%ld blocks written: %s\n",
				n, (n == cnt) ? "OK" : "ERROR");
			return (n == cnt) ? 0 : 1;
		} else {
			return CMD_RET_USAGE;
		}

		return rc;
	}
}

U_BOOT_CMD(
	sata, 5, 1, do_sata,
	"SATA sub system",
	"init - init SATA sub system\n"
	"sata stop - disable SATA sub system\n"
	"sata info - show available SATA devices\n"
	"sata device [dev] - show or set current device\n"
	"sata part [dev] - print partition table\n"
	"sata read addr blk# cnt\n"
	"sata write addr blk# cnt"
);

/*
 *  U-Boot command for OneNAND support
 *
 *  Copyright (C) 2005-2007 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <command.h>

#ifdef CONFIG_CMD_ONENAND

#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

#include <asm/io.h>

extern struct mtd_info onenand_mtd;
extern struct onenand_chip onenand_chip;

int do_onenand(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int ret = 0;

	switch (argc) {
	case 0:
	case 1:
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;

	case 2:
		if (strncmp(argv[1], "open", 4) == 0) {
			onenand_init();
			return 0;
		}
		onenand_print_device_info(onenand_chip.device_id, 1);
		return 0;

	default:
		/* At least 4 args */
		if (strncmp(argv[1], "erase", 5) == 0) {
			struct erase_info instr = {
				.callback	= NULL,
			};
			ulong start, end;
			ulong block;
			char *endtail;

			if (strncmp(argv[2], "block", 5) == 0) {
				start = simple_strtoul(argv[3], NULL, 10);
				endtail = strchr(argv[3], '-');
				end = simple_strtoul(endtail + 1, NULL, 10);
			} else {
				start = simple_strtoul(argv[2], NULL, 10);
				end = simple_strtoul(argv[3], NULL, 10);
				start -= (unsigned long)onenand_chip.base;
				end -= (unsigned long)onenand_chip.base;

				start >>= onenand_chip.erase_shift;
				end >>= onenand_chip.erase_shift;
				/* Don't include the end block */
				end--;
			}

			if (!end || end < 0)
				end = start;

			printf("Erase block from %d to %d\n", start, end);

			for (block = start; block <= end; block++) {
				instr.addr = block << onenand_chip.erase_shift;
				instr.len = 1 << onenand_chip.erase_shift;
				ret = onenand_erase(&onenand_mtd, &instr);
				if (ret) {
					printf("erase failed %d\n", block);
					break;
				}
			}

			return 0;
		}

		if (strncmp(argv[1], "read", 4) == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong ofs = simple_strtoul(argv[3], NULL, 16);
			size_t len = simple_strtoul(argv[4], NULL, 16);
			size_t retlen = 0;
			int oob = strncmp(argv[1], "read.oob", 8) ? 0 : 1;

			ofs -= (unsigned long)onenand_chip.base;

			if (oob)
				onenand_read_oob(&onenand_mtd, ofs, len,
						 &retlen, (u_char *) addr);
			else
				onenand_read(&onenand_mtd, ofs, len, &retlen,
					     (u_char *) addr);
			printf("Done\n");

			return 0;
		}

		if (strncmp(argv[1], "write", 5) == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong ofs = simple_strtoul(argv[3], NULL, 16);
			size_t len = simple_strtoul(argv[4], NULL, 16);
			size_t retlen = 0;

			ofs -= (unsigned long)onenand_chip.base;

			onenand_write(&onenand_mtd, ofs, len, &retlen,
				      (u_char *) addr);
			printf("Done\n");

			return 0;
		}

		if (strncmp(argv[1], "block", 5) == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong block = simple_strtoul(argv[3], NULL, 10);
			ulong page = simple_strtoul(argv[4], NULL, 10);
			size_t len = simple_strtol(argv[5], NULL, 10);
			size_t retlen = 0;
			ulong ofs;
			int oob = strncmp(argv[1], "block.oob", 9) ? 0 : 1;

			ofs = block << onenand_chip.erase_shift;
			if (page)
				ofs += page << onenand_chip.page_shift;

			if (!len) {
				if (oob)
					len = 64;
				else
					len = 512;
			}

			if (oob)
				onenand_read_oob(&onenand_mtd, ofs, len,
						 &retlen, (u_char *) addr);
			else
				onenand_read(&onenand_mtd, ofs, len, &retlen,
					     (u_char *) addr);
			return 0;
		}

		break;
	}

	return 0;
}

U_BOOT_CMD(
	onenand,	6,	1,	do_onenand,
	"onenand - OneNAND sub-system\n",
	"info   - show available OneNAND devices\n"
	"onenand read[.oob] addr ofs len - read data at ofs with len to addr\n"
	"onenand write addr ofs len - write data at ofs with len from addr\n"
	"onenand erase saddr eaddr - erase block start addr to end addr\n"
	"onenand block[.oob] addr block [page] [len] - "
		"read data with (block [, page]) to addr"
);

#endif /* CONFIG_CMD_ONENAND */

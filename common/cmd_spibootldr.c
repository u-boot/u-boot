/*
 * U-boot - spibootldr.c
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>

#include <asm/blackfin.h>
#include <asm/mach-common/bits/bootrom.h>

int do_spibootldr(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	s32 addr;

	/* Get the address */
	if (argc < 2)
		addr = 0;
	else
		addr = simple_strtoul(argv[1], NULL, 16);

	printf("## Booting ldr image at SPI offset 0x%x ...\n", addr);

	return bfrom_SpiBoot(addr, BFLAG_PERIPHERAL | 4, 0, NULL);
}

U_BOOT_CMD(spibootldr, 2, 0, do_spibootldr,
	"boot ldr image from spi",
	"[offset]\n"
	"    - boot ldr image stored at offset into spi\n");

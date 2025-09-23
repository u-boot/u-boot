// SPDX-License-Identifier: GPL-2.0+
/*
 * Brian Sune <briansune@gmail.com>
 */

#include <vsprintf.h>
#include <command.h>
#include <asm/io.h>

#include <asm/arch/base_addr_ac5.h>

#define RSTMGR_PERMODRST 0x18   /* PERMODRST register offset */

static int do_dmareset(struct cmd_tbl *cmdtp, int flag, int argc,
		       char * const argv[])
{
	u8 val;
	int i, ch;

	if (argc < 2) {
		printf("Usage: dmareset <channel 0-7> [<channel 0-7> ...]\n");
		return CMD_RET_USAGE;
	}

	/* Read current register value */
	val = readb(SOCFPGA_RSTMGR_ADDRESS + RSTMGR_PERMODRST);

	/* Iterate over all channels given as arguments */
	for (i = 1; i < argc; i++) {
		ch = simple_strtoul(argv[i], NULL, 0);
		if (ch < 0 || ch > 7) {
			printf("Error: channel must be 0-7\n");
			return CMD_RET_USAGE;
		}
		val &= ~(1 << ch);
		printf("PL330 DMA channel %d reset released\n", ch);
	}

	/* Write back */
	writeb(val, (SOCFPGA_RSTMGR_ADDRESS + RSTMGR_PERMODRST));

	return 0;
}

U_BOOT_CMD(
	dmareset, 8, 0, do_dmareset,
	"Release PL330 DMA channel reset(s) for SoCFPGA",
	"dmareset <channel 0-7> [<channel 0-7> ...]  - release reset for one or more DMA channels"
);

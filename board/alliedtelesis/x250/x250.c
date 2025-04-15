// SPDX-License-Identifier:    GPL-2.0+

#include <config.h>
#include <asm/global_data.h>
#include <linux/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define DEVICE_BUS_SYNC_CTRL	0xF27004C8

int board_init(void)
{
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

	/* DEV_READYn is not needed for NVS, ignore it when accessing CS1 */
	writel(0x00004001, DEVICE_BUS_SYNC_CTRL);

	return 0;
}

/*
 * Copyright (C) 2010 Albert ARIBAUD <albert.u.boot@aribaud.net>
 *
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <miiphy.h>
#include <asm/arch/orion5x.h>
#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * The ED Mini V2 is equipped with a Macronix MXLV400CB FLASH
 * which CFI does not properly detect, hence the LEGACY config.
 */
#if defined(CONFIG_FLASH_CFI_LEGACY)
#include <flash.h>
ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
	int sectsz[] = CONFIG_SYS_FLASH_SECTSZ;
	int sect;

	if (base != CONFIG_SYS_FLASH_BASE)
		return 0;

	info->size = 0;
	info->sector_count = CONFIG_SYS_MAX_FLASH_SECT;
	/* set each sector's start address and size based */
	for (sect = 0; sect < CONFIG_SYS_MAX_FLASH_SECT; sect++) {
		info->start[sect] = base+info->size;
		info->size += sectsz[sect];
	}
	/* This flash must be accessed in 8-bits mode, no buffer. */
	info->flash_id = 0x01000000;
	info->portwidth = FLASH_CFI_8BIT;
	info->chipwidth = FLASH_CFI_BY8;
	info->buffer_size = 0;
	/* timings are derived from the Macronix datasheet. */
	info->erase_blk_tout = 1000;
	info->write_tout = 10;
	info->buffer_write_tout = 300;
	/* Commands and addresses are for AMD mode 8-bit access. */
	info->vendor = CFI_CMDSET_AMD_LEGACY;
	info->cmd_reset = 0xF0;
	info->interface = FLASH_CFI_X8;
	info->legacy_unlock = 0;
	info->ext_addr = 0;
	info->addr_unlock1 = 0x00000aaa;
	info->addr_unlock2 = 0x00000555;
	/* Manufacturer Macronix, device MX29LV400CB, CFI 1.3. */
	info->manufacturer_id = 0x22;
	info->device_id = 0xBA;
	info->device_id2 = 0;
	info->cfi_version = 0x3133;
	info->cfi_offset = 0x0000;
	info->name = "MX29LV400CB";

	return 1;
}
#endif				/* CONFIG_SYS_FLASH_CFI */

int board_init(void)
{
	/* arch number of board */
	gd->bd->bi_arch_number = MACH_TYPE_EDMINI_V2;

	/* boot parameter start at 256th byte of RAM base */
	gd->bd->bi_boot_params = gd->bd->bi_dram[0].start + 0x100;

	return 0;
}

#if defined(CONFIG_CMD_NET) && defined(CONFIG_RESET_PHY_R)
/* Configure and enable MV88E1116 PHY */
void reset_phy(void)
{
	mv_phy_88e1116_init("egiga0", 8);
}
#endif /* CONFIG_RESET_PHY_R */

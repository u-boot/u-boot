/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>

#include <faraday/ftsdc010.h>
#ifdef CONFIG_FTSMC020
#include <faraday/ftsmc020.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscellaneous platform dependent initializations
 */

int board_init(void)
{
	/*
	 * refer to BOOT_PARAMETER_PA_BASE within
	 * "linux/arch/nds32/include/asm/misc_spec.h"
	 */
	gd->bd->bi_arch_number = MACH_TYPE_ADPAG102;
	gd->bd->bi_boot_params = PHYS_SDRAM_0 + 0x400;

#if !defined(CONFIG_SYS_NO_FLASH)
	ftsmc020_init();	/* initialize Flash */
#endif /* CONFIG_SYS_NO_FLASH */
	return 0;
}

int dram_init(void)
{
	unsigned long sdram_base = PHYS_SDRAM_0;
	unsigned long expected_size = PHYS_SDRAM_0_SIZE;
	unsigned long actual_size;

	actual_size = get_ram_size((void *)sdram_base, expected_size);

	gd->ram_size = actual_size;

	if (expected_size != actual_size) {
		printf("Warning: Only %lu of %lu MiB SDRAM is working\n",
				actual_size >> 20, expected_size >> 20);
	}

	return 0;
}

int board_eth_init(bd_t *bd)
{
	return ftgmac100_initialize(bd);
}

#if !defined(CONFIG_SYS_NO_FLASH)
ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
	if (banknum == 0) {	/* non-CFI boot flash */
		info->portwidth = FLASH_CFI_8BIT;
		info->chipwidth = FLASH_CFI_BY8;
		info->interface = FLASH_CFI_X8;
		return 1;
	} else {
		return 0;
	}
}
#endif /* CONFIG_SYS_NO_FLASH */

#if defined(CONFIG_CMD_PCI) || defined(CONFIG_PCI)
void pci_init_board(void)
{
	/* should be pci_ftpci100_init() */
	extern void pci_ftpci_init();

	pci_ftpci_init();
}
#endif

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	ftsdc010_mmc_init(0);
	return 0;
}
#endif

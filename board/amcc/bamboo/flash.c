/*
 * (C) Copyright 2004-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002 Jun Gu <jung@artesyncp.com>
 * Add support for Am29F016D and dynamic switch setting.
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
 */

/*
 * Modified 4/5/2001
 * Wait for completion of each sector erase command issued
 * 4/5/2001
 * Chris Hallinan - DS4.COM, Inc. - clh@net1plus.com
 */

#include <common.h>
#include <asm/ppc4xx.h>
#include <asm/processor.h>
#include <asm/ppc440.h>
#include "bamboo.h"

#undef DEBUG

#ifdef DEBUG
#define DEBUGF(x...) printf(x)
#else
#define DEBUGF(x...)
#endif				/* DEBUG */

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];	/* info for FLASH chips        */

/*
 * Mark big flash bank (16 bit instead of 8 bit access) in address with bit 0
 */
static unsigned long flash_addr_table[][CONFIG_SYS_MAX_FLASH_BANKS] = {
	{0x87800001, 0xFFF00000, 0xFFF80000}, /* 0:boot from small flash */
	{0x00000000, 0x00000000, 0x00000000}, /* 1:boot from pci 66      */
	{0x87800001, 0x00000000, 0x00000000}, /* 0:boot from nand flash  */
	{0x87F00000, 0x87F80000, 0xFFC00001}, /* 3:boot from big flash 33*/
	{0x87F00000, 0x87F80000, 0xFFC00001}, /* 4:boot from big flash 66*/
	{0x00000000, 0x00000000, 0x00000000}, /* 5:boot from             */
	{0x00000000, 0x00000000, 0x00000000}, /* 6:boot from pci 66      */
	{0x00000000, 0x00000000, 0x00000000}, /* 7:boot from             */
	{0x87C00001, 0xFFF00000, 0xFFF80000}, /* 0:boot from small flash */
};

/*
 * include common flash code (for amcc boards)
 */
#include "../common/flash.c"

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size(vu_long * addr, flash_info_t * info);
static int write_word(flash_info_t * info, ulong dest, ulong data);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init(void)
{
	unsigned long total_b = 0;
	unsigned long size_b[CONFIG_SYS_MAX_FLASH_BANKS];
	unsigned short index = 0;
	int i;
	unsigned long val;
	unsigned long ebc_boot_size;
	unsigned long boot_selection;

	mfsdr(SDR0_PINSTP, val);
	index = (val & SDR0_PSTRP0_BOOTSTRAP_MASK) >> 29;

	if ((index == 5) || (index == 7)) {
		/*
		 * Boot Settings in IIC EEprom address 0xA8 or 0xA4
		 * Read Serial Device Strap Register1 in PPC440EP
		 */
		mfsdr(SDR0_SDSTP1, val);
		boot_selection  = val & SDR0_SDSTP1_BOOT_SEL_MASK;
		ebc_boot_size   = val & SDR0_SDSTP1_EBC_ROM_BS_MASK;

		switch(boot_selection) {
		case SDR0_SDSTP1_BOOT_SEL_EBC:
			switch(ebc_boot_size) {
			case SDR0_SDSTP1_EBC_ROM_BS_16BIT:
				index = 3;
				break;
			case SDR0_SDSTP1_EBC_ROM_BS_8BIT:
				index = 0;
				break;
			}
			break;

		case SDR0_SDSTP1_BOOT_SEL_PCI:
			index = 1;
			break;

		case SDR0_SDSTP1_BOOT_SEL_NDFC:
			index = 2;
			break;
		}
	} else if (index == 0) {
		if (in8(FPGA_SETTING_REG) & FPGA_SET_REG_OP_CODE_FLASH_ABOVE) {
			index = 8; /* sram below op code flash -> new index 8 */
		}
	}

	DEBUGF("\n");
	DEBUGF("FLASH: Index: %d\n", index);

	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
		flash_info[i].sector_count = -1;
		flash_info[i].size = 0;

		/* check whether the address is 0 */
		if (flash_addr_table[index][i] == 0)
			continue;

		DEBUGF("Detection bank %d...\n", i);
		/* call flash_get_size() to initialize sector address */
		size_b[i] = flash_get_size((vu_long *) flash_addr_table[index][i],
				   &flash_info[i]);
		flash_info[i].size = size_b[i];
		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
			printf("## Unknown FLASH on Bank %d - Size = 0x%08lx = %ld MB\n",
			       i, size_b[i], size_b[i] << 20);
			flash_info[i].sector_count = -1;
			flash_info[i].size = 0;
		}

		/* Monitor protection ON by default */
		(void)flash_protect(FLAG_PROTECT_SET, CONFIG_SYS_MONITOR_BASE,
				    CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN - 1,
				    &flash_info[i]);
#if defined(CONFIG_ENV_IS_IN_FLASH)
		(void)flash_protect(FLAG_PROTECT_SET, CONFIG_ENV_ADDR,
				    CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1,
				    &flash_info[i]);
#if defined(CONFIG_ENV_IS_IN_FLASH) && defined(CONFIG_ENV_ADDR_REDUND)
		(void)flash_protect(FLAG_PROTECT_SET, CONFIG_ENV_ADDR_REDUND,
				    CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - 1,
				    &flash_info[i]);
#endif
#endif

		total_b += flash_info[i].size;
	}

	return total_b;
}

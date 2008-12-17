/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/fsl_ddr_sdram.h>
#include <asm/mmu.h>

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
extern void ddr_enable_ecc(unsigned int dram_size);
#endif

phys_size_t initdram(int board_type)
{
	phys_size_t dram_size = fsl_ddr_sdram();

	dram_size = setup_ddr_tlbs(dram_size / 0x100000);

	dram_size *= 0x100000;

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/* Initialize and enable DDR ECC */
	ddr_enable_ecc(dram_size);
#endif

	return dram_size;
}

#if defined(CONFIG_DDR_ECC) || (CONFIG_NUM_DDR_CONTROLLERS > 1)
void board_add_ram_info(int use_default)
{
#if (CONFIG_NUM_DDR_CONTROLLERS > 1)
	volatile ccsr_ddr_t *ddr1 = (void *)(CONFIG_SYS_MPC85xx_DDR_ADDR);
#endif

	puts(" (");

#if (CONFIG_NUM_DDR_CONTROLLERS > 1)
	/* Print interleaving information */
	if (ddr1->cs0_config & 0x20000000) {
		switch ((ddr1->cs0_config >> 24) & 0xf) {
		case 0:
			puts("cache line");
			break;
		case 1:
			puts("page");
			break;
		case 2:
			puts("bank");
			break;
		case 3:
			puts("super-bank");
			break;
		default:
			puts("invalid");
			break;
		}
	} else {
		puts("no");
	}

	puts(" interleaving");
#endif

#if (CONFIG_NUM_DDR_CONTROLLERS > 1) && defined(CONFIG_DDR_ECC)
	puts(", ");
#endif

#if defined(CONFIG_DDR_ECC)
	puts("ECC enabled");
#endif

	puts(")");
}
#endif /* CONFIG_DDR_ECC || CONFIG_NUM_DDR_CONTROLLERS > 1 */

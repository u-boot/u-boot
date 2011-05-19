/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
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
#include <netdev.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>

extern void pci_of_setup(void *blob, bd_t *bd);

#include "../common/ngpixis.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard (void)
{
	u8 sw;
	struct cpu_type *cpu = gd->cpu;
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	unsigned int i;

	printf("Board: %sDS, ", cpu->name);
	printf("Sys ID: 0x%02x, Sys Ver: 0x%02x, FPGA Ver: 0x%02x, ",
		in_8(&pixis->id), in_8(&pixis->arch), in_8(&pixis->scver));

	sw = in_8(&PIXIS_SW(PIXIS_LBMAP_SWITCH));
	sw = (sw & PIXIS_LBMAP_MASK) >> PIXIS_LBMAP_SHIFT;

	if (sw < 0x8)
		printf("vBank: %d\n", sw);
	else if (sw == 0x8)
		puts("Promjet\n");
	else if (sw == 0x9)
		puts("NAND\n");
	else
		printf("invalid setting of SW%u\n", PIXIS_LBMAP_SWITCH);

#ifdef CONFIG_PHYS_64BIT
	puts("36-bit Addressing\n");
#endif

	/* Display the RCW, so that no one gets confused as to what RCW
	 * we're actually using for this boot.
	 */
	puts("Reset Configuration Word (RCW):");
	for (i = 0; i < ARRAY_SIZE(gur->rcwsr); i++) {
		u32 rcw = in_be32(&gur->rcwsr[i]);

		if ((i % 4) == 0)
			printf("\n       %08x:", i * 4);
		printf(" %08x", rcw);
	}
	puts("\n");

	/* Display the actual SERDES reference clocks as configured by the
	 * dip switches on the board.  Note that the SWx registers could
	 * technically be set to force the reference clocks to match the
	 * values that the SERDES expects (or vice versa).  For now, however,
	 * we just display both values and hope the user notices when they
	 * don't match.
	 */
	puts("SERDES Reference Clocks: ");
#if defined(CONFIG_P3041DS) || defined(CONFIG_P5020DS)
	sw = in_8(&PIXIS_SW(5));
	for (i = 0; i < 3; i++) {
		static const char *freq[] = {"100", "125", "156.25", "212.5" };
		unsigned int clock = (sw >> (6 - (2 * i))) & 3;

		printf("Bank%u=%sMhz ", i+1, freq[clock]);
	}
	puts("\n");
#else
	sw = in_8(&PIXIS_SW(3));
	printf("Bank1=%uMHz ", (sw & 0x40) ? 125 : 100);
	printf("Bank2=%sMHz ", (sw & 0x20) ? "156.25" : "125");
	printf("Bank3=%sMHz\n", (sw & 0x10) ? "156.25" : "125");
#endif

	return 0;
}

int board_early_init_f(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/*
	 * P4080 DS board only uses the DDR1_MCK0/3 and DDR2_MCK0/3
	 * disable the DDR1_MCK1/2/4/5 and DDR2_MCK1/2/4/5 to reduce
	 * the noise introduced by these unterminated and unused clock pairs.
	 */
	setbits_be32(&gur->ddrclkdr, 0x001B001B);

	return 0;
}

int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	const u8 flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash + PROMJET region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	/* invalidate existing TLB entry for flash + promjet */
	disable_tlb(flash_esel);

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,	/* tlb, epn, rpn */
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,	/* perms, wimge */
			0, flash_esel, BOOKE_PAGESZ_256M, 1);	/* ts, esel, tsize, iprot */

	set_liodns();
#ifdef CONFIG_SYS_DPAA_QBMAN
	setup_portals();
#endif

	return 0;
}

static const char *serdes_clock_to_string(u32 clock)
{
	switch(clock) {
	case SRDS_PLLCR0_RFCK_SEL_100:
		return "100";
	case SRDS_PLLCR0_RFCK_SEL_125:
		return "125";
	case SRDS_PLLCR0_RFCK_SEL_156_25:
		return "156.25";
	default:
		return "150";
	}
}

#define NUM_SRDS_BANKS	3

int misc_init_r(void)
{
	serdes_corenet_t *srds_regs = (void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	u32 actual[NUM_SRDS_BANKS];
	unsigned int i;
	u8 sw;

#if defined(CONFIG_P3041DS) || defined(CONFIG_P5020DS)
	sw = in_8(&PIXIS_SW(5));
	for (i = 0; i < 3; i++) {
		unsigned int clock = (sw >> (6 - (2 * i))) & 3;
		switch (clock) {
		case 0:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_100;
			break;
		case 1:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_125;
			break;
		case 2:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_156_25;
			break;
		default:
			printf("Warning: SDREFCLK%u switch setting of '11' is "
			       "unsupported\n", i + 1);
			break;
		}
	}
#else
	/* Warn if the expected SERDES reference clocks don't match the
	 * actual reference clocks.  This needs to be done after calling
	 * p4080_erratum_serdes8(), since that function may modify the clocks.
	 */
	sw = in_8(&PIXIS_SW(3));
	actual[0] = (sw & 0x40) ?
		SRDS_PLLCR0_RFCK_SEL_125 : SRDS_PLLCR0_RFCK_SEL_100;
	actual[1] = (sw & 0x20) ?
		SRDS_PLLCR0_RFCK_SEL_156_25 : SRDS_PLLCR0_RFCK_SEL_125;
	actual[2] = (sw & 0x10) ?
		SRDS_PLLCR0_RFCK_SEL_156_25 : SRDS_PLLCR0_RFCK_SEL_125;
#endif

	for (i = 0; i < NUM_SRDS_BANKS; i++) {
		u32 expected = srds_regs->bank[i].pllcr0 & SRDS_PLLCR0_RFCK_SEL_MASK;
		if (expected != actual[i]) {
			printf("Warning: SERDES bank %u expects reference clock"
			       " %sMHz, but actual is %sMHz\n", i + 1,
			       serdes_clock_to_string(expected),
			       serdes_clock_to_string(actual[i]));
		}
	}

	return 0;
}

void ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = getenv_bootm_low();
	size = getenv_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_PCI
	pci_of_setup(blob, bd);
#endif

	fdt_fixup_liodn(blob);
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}

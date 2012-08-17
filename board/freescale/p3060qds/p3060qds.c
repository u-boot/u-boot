/*
 * Copyright 2011 Freescale Semiconductor, Inc.
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
#include <fm_eth.h>
#include <configs/P3060QDS.h>
#include <libfdt.h>
#include <fdt_support.h>

#include "../common/qixis.h"
#include "p3060qds.h"
#include "p3060qds_qixis.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	u8 sw;
	struct cpu_type *cpu = gd->cpu;
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	unsigned int i;

	printf("Board: %s", cpu->name);
	puts("QDS, ");

	printf("Sys ID: 0x%02x, Sys Ver: 0x%02x, FPGA Ver: 0x%02x, ",
		QIXIS_READ(id), QIXIS_READ(arch), QIXIS_READ(scver));

	sw = QIXIS_READ(brdcfg[0]);
	sw = (sw & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

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
	puts("Reset Configuration Word (RCW):");
	for (i = 0; i < ARRAY_SIZE(gur->rcwsr); i++) {
		u32 rcw = in_be32(&gur->rcwsr[i]);

		if ((i % 4) == 0)
			printf("\n       %08x:", i * 4);
		printf(" %08x", rcw);
	}
	puts("\n");

	puts("SERDES Reference Clocks: ");
	sw = QIXIS_READ(brdcfg[2]);
	for (i = 0; i < 3; i++) {
		static const char * const freq[] = {"100", "125", "Reserved",
						"156.25"};
		unsigned int clock = (sw >> (2 * i)) & 3;

		printf("Bank%u=%sMhz ", i+1, freq[clock]);
	}
	puts("\n");

	return 0;
}

int board_early_init_f(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/* only single DDR controller on QDS board, disable DDR1_MCK4/5 */
	setbits_be32(&gur->ddrclkdr, 0x00030000);

	return 0;
}

void board_config_serdes_mux(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	int cfg = (in_be32(&gur->rcwsr[4]) &
			FSL_CORENET_RCWSR4_SRDS_PRTCL) >> 26;

	switch (cfg) {
	case 0x03:
	case 0x06:
		/* set Lane I,J as SGMII */
		QIXIS_WRITE(brdcfg[6], BRDCFG6_SD4MX_B | BRDCFG6_SD3MX_A |
				       BRDCFG6_SD2MX_B | BRDCFG6_SD1MX_A);
		break;
	case 0x16:
	case 0x19:
	case 0x1c:
		/* set Lane I,J as Aurora Debug */
		QIXIS_WRITE(brdcfg[6], BRDCFG6_SD4MX_A | BRDCFG6_SD3MX_B |
				       BRDCFG6_SD2MX_A | BRDCFG6_SD1MX_B);
		break;
	default:
		puts("Invalid SerDes protocol for P3060QDS\n");
		break;
	}
}

void board_config_usb_mux(void)
{
	u8 brdcfg4, brdcfg5, brdcfg7;
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 rcwsr11 = in_be32(&gur->rcwsr[11]);
	u32 ec1 = rcwsr11 & FSL_CORENET_RCWSR11_EC1;
	u32 ec2 = rcwsr11 & FSL_CORENET_RCWSR11_EC2;

	brdcfg4 = QIXIS_READ(brdcfg[4]);
	brdcfg4 &= ~BRDCFG4_EC_MODE_MASK;
	if ((ec1 == FSL_CORENET_RCWSR11_EC1_FM1_USB1) &&
		 (ec2 == FSL_CORENET_RCWSR11_EC2_USB2)) {
		brdcfg4 |= BRDCFG4_EC2_USB_EC1_USB;

	} else if ((ec1 == FSL_CORENET_RCWSR11_EC1_FM1_USB1) &&
		 ((ec2 == FSL_CORENET_RCWSR11_EC2_FM1_DTSEC2) ||
		 (ec2 == FSL_CORENET_RCWSR11_EC2_FM2_DTSEC1))) {
		brdcfg4 |= BRDCFG4_EC2_RGMII_EC1_USB;

	} else if ((ec1 == FSL_CORENET_RCWSR11_EC1_FM1_DTSEC1) &&
		 (ec2 == FSL_CORENET_RCWSR11_EC2_USB2)) {
		brdcfg4 |= BRDCFG4_EC2_USB_EC1_RGMII;

	} else if ((ec1 == FSL_CORENET_RCWSR11_EC1_FM1_DTSEC1) &&
		 ((ec2 == FSL_CORENET_RCWSR11_EC2_FM1_DTSEC2) ||
		 (ec2 == FSL_CORENET_RCWSR11_EC2_FM2_DTSEC1))) {
		brdcfg4 |= BRDCFG4_EC2_RGMII_EC1_RGMII;
	} else {
		brdcfg4 |= BRDCFG4_EC2_MII_EC1_MII;
	}
	QIXIS_WRITE(brdcfg[4], brdcfg4);

	brdcfg5 = QIXIS_READ(brdcfg[5]);
	brdcfg5 &= ~(BRDCFG5_USB1ID_MASK | BRDCFG5_USB2ID_MASK);
	brdcfg5 |= (BRDCFG5_USB1ID_CTRL | BRDCFG5_USB2ID_CTRL);
	QIXIS_WRITE(brdcfg[5], brdcfg5);

	brdcfg7 = BRDCFG7_JTAGMX_COP_JTAG | BRDCFG7_IQ1MX_IRQ_EVT |
		BRDCFG7_G1MX_USB1 | BRDCFG7_D1MX_TSEC3USB | BRDCFG7_I3MX_USB1;
	QIXIS_WRITE(brdcfg[7], brdcfg7);
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

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, flash_esel, BOOKE_PAGESZ_256M, 1);

	set_liodns();
#ifdef CONFIG_SYS_DPAA_QBMAN
	setup_portals();
#endif
	board_config_serdes_mux();
	board_config_usb_mux();

	return 0;
}

static const char *serdes_clock_to_string(u32 clock)
{
	switch (clock) {
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
	serdes_corenet_t *srds_regs;
	u32 actual[NUM_SRDS_BANKS];
	unsigned int i;
	u8 sw;

	sw = QIXIS_READ(brdcfg[2]);
	for (i = 0; i < 3; i++) {
		unsigned int clock = (sw >> (2 * i)) & 3;
		switch (clock) {
		case 0:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_100;
			break;
		case 1:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_125;
			break;
		case 3:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_156_25;
			break;
		default:
			printf("Warning: SDREFCLK%u switch setting of '10' is "
				"unsupported\n", i + 1);
			break;
		}
	}

	srds_regs = (void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	for (i = 0; i < NUM_SRDS_BANKS; i++) {
		u32 pllcr0 = in_be32(&srds_regs->bank[i].pllcr0);
		u32 expected = pllcr0 & SRDS_PLLCR0_RFCK_SEL_MASK;
		if (expected != actual[i]) {
			printf("Warning: SERDES bank %u expects reference clock"
			       " %sMHz, but actual is %sMHz\n", i + 1,
			       serdes_clock_to_string(expected),
			       serdes_clock_to_string(actual[i]));
		}
	}

	return 0;
}

/*
 * This is map of CVDD values. 33 means CVDD is 3.3v, 25 means CVDD is 2.5v,
 * 18 means CVDD is 1.8v.
 */
static u8 IO_VSEL[] = {
	33, 33, 33, 25, 25, 25, 18, 18, 18,
	33, 33, 33, 25, 25, 25, 18, 18, 18,
	33, 33, 33, 25, 25, 25, 18, 18, 18,
	33, 33, 33, 33, 33
};

#define IO_VSEL_MASK	0x1f

/*
 * different CVDD selects diffenert spi flashs, read dutcfg[3] to get CVDD,
 * then set status of  spi flash nodes to 'disabled' according to CVDD.
 * CVDD '33' will select spi flash0 and flash1, CVDD '25' will select spi
 * flash2, CVDD '18' will select spi flash3.
 */
void fdt_fixup_board_spi(void *blob)
{
	u8 sw5 = QIXIS_READ(dutcfg[3]);

	switch (IO_VSEL[sw5 & IO_VSEL_MASK]) {
	/* 3.3v */
	case 33:
		do_fixup_by_compat(blob, "atmel,at45db081d", "status",
				"disabled", strlen("disabled") + 1, 1);
		do_fixup_by_compat(blob, "spansion,sst25wf040", "status",
				"disabled", strlen("disabled") + 1, 1);
		break;
	/* 2.5v */
	case 25:
		do_fixup_by_compat(blob, "spansion,s25sl12801", "status",
				"disabled", strlen("disabled") + 1, 1);
		do_fixup_by_compat(blob, "spansion,en25q32", "status",
				"disabled", strlen("disabled") + 1, 1);
		do_fixup_by_compat(blob, "spansion,sst25wf040", "status",
				"disabled", strlen("disabled") + 1, 1);
		break;
	/* 1.8v */
	case 18:
		do_fixup_by_compat(blob, "spansion,s25sl12801", "status",
				"disabled", strlen("disabled") + 1, 1);
		do_fixup_by_compat(blob, "spansion,en25q32", "status",
				"disabled", strlen("disabled") + 1, 1);
		do_fixup_by_compat(blob, "atmel,at45db081d", "status",
				"disabled", strlen("disabled") + 1, 1);
		break;
	}
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
	fdt_fixup_dr_usb(blob, bd);
	fdt_fixup_board_spi(blob);

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
	fdt_fixup_board_enet(blob);
#endif
}

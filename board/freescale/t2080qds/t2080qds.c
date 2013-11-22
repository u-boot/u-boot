/*
 * Copyright 2009-2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <netdev.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>
#include <fm_eth.h>

#include "../common/qixis.h"
#include "../common/vsc3316_3308.h"
#include "t2080qds.h"
#include "t2080qds_qixis.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	char buf[64];
	u8 sw;
	struct cpu_type *cpu = gd->arch.cpu;
	static const char *freq[4] = {
		"100.00MHZ(from 8T49N222A)", "125.00MHz",
		"156.25MHZ", "100.00MHz"
	};

	printf("Board: %sQDS, ", cpu->name);
	sw = QIXIS_READ(arch);
	printf("Sys ID: 0x%02x, Board Arch: V%d, ", QIXIS_READ(id), sw >> 4);
	printf("Board Version: %c, boot from ", (sw & 0xf) + 'A' - 1);

	sw = QIXIS_READ(brdcfg[0]);
	sw = (sw & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

	if (sw < 0x8)
		printf("vBank%d\n", sw);
	else if (sw == 0x8)
		puts("Promjet\n");
	else if (sw == 0x9)
		puts("NAND\n");
	else
		printf("invalid setting of SW%u\n", QIXIS_LBMAP_SWITCH);

	printf("FPGA: v%d (%s), build %d", (int)QIXIS_READ(scver),
	       qixis_read_tag(buf), (int)qixis_read_minor());
	/* the timestamp string contains "\n" at the end */
	printf(" on %s", qixis_read_time(buf));

	puts("SERDES Reference Clocks:\n");
	sw = QIXIS_READ(brdcfg[2]);
	printf("SD1_CLK1=%s, SD1_CLK2=%s\n", freq[sw >> 6],
	       freq[(sw >> 4) & 0x3]);
	printf("SD2_CLK1=%s, SD2_CLK2=%s\n", freq[(sw & 0xf) >> 2],
	       freq[sw & 0x3]);

	return 0;
}

int select_i2c_ch_pca9547(u8 ch)
{
	int ret;

	ret = i2c_write(I2C_MUX_PCA_ADDR_PRI, 0, 1, &ch, 1);
	if (ret) {
		puts("PCA: failed to select proper channel\n");
		return ret;
	}

	return 0;
}

int brd_mux_lane_to_slot(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_prtcl_s1, srds_prtcl_s2;

	srds_prtcl_s1 = in_be32(&gur->rcwsr[4]) &
				FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	srds_prtcl_s1 >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;
	srds_prtcl_s2 = in_be32(&gur->rcwsr[4]) &
				FSL_CORENET2_RCWSR4_SRDS2_PRTCL;
	srds_prtcl_s2 >>= FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT;

	switch (srds_prtcl_s1) {
	case 0:
		/* SerDes1 is not enabled */
		break;
	case 0x1c:
	case 0x95:
	case 0xa2:
	case 0x94:
		/* SD1(A:D) => SLOT3 SGMII
		 * SD1(G:H) => SLOT1 SGMII
		 */
		QIXIS_WRITE(brdcfg[12], 0x58);
		break;
	case 0x51:
		/* SD1(A:D) => SLOT3 XAUI
		 * SD1(E)   => SLOT1 PCIe4
		 * SD1(F:H) => SLOT2 SGMII
		 */
		QIXIS_WRITE(brdcfg[12], 0x15);
		break;
	case 0x66:
	case 0x67:
		/* SD1(A:D) => XFI cage
		 * SD1(E:H) => SLOT1 PCIe4
		 */
		QIXIS_WRITE(brdcfg[12], 0xfe);
		break;
	case 0x6b:
		/* SD1(A:D) => XFI cage
		 * SD1(E)   => SLOT1 PCIe4
		 * SD1(F:H) => SLOT2 SGMII
		 */
		QIXIS_WRITE(brdcfg[12], 0xf1);
		break;
	case 0x6c:
	case 0x6d:
		/* SD1(A:B) => XFI cage
		 * SD1(C:D) => SLOT3 SGMII
		 * SD1(E:H) => SLOT1 PCIe4
		 */
		QIXIS_WRITE(brdcfg[12], 0xda);
		break;
	default:
		printf("WARNING: unsupported for SerDes1 Protocol %d\n",
		       srds_prtcl_s1);
		return -1;
	}

	switch (srds_prtcl_s2) {
	case 0:
		/* SerDes2 is not enabled */
		break;
	case 0x01:
	case 0x02:
		/* SD2(A:H) => SLOT4 PCIe1 */
		QIXIS_WRITE(brdcfg[13], 0x20);
		break;
	case 0x15:
	case 0x16:
		/*
		 * SD2(A:D) => SLOT4 PCIe1
		 * SD2(E:F) => SLOT5 PCIe2
		 * SD2(G:H) => SATA1,SATA2
		 */
		QIXIS_WRITE(brdcfg[13], 0xb0);
		break;
	case 0x18:
		/*
		 * SD2(A:D) => SLOT4 PCIe1
		 * SD2(E:F) => SLOT5 Aurora
		 * SD2(G:H) => SATA1,SATA2
		 */
		QIXIS_WRITE(brdcfg[13], 0x70);
		break;
	case 0x1f:
		/*
		 * SD2(A:D) => SLOT4 PCIe1
		 * SD2(E:H) => SLOT5 PCIe2
		 */
		QIXIS_WRITE(brdcfg[13], 0xa0);
		break;
	case 0x29:
	case 0x2d:
	case 0x2e:
		/*
		 * SD2(A:D) => SLOT4 SRIO2
		 * SD2(E:H) => SLOT5 SRIO1
		 */
		QIXIS_WRITE(brdcfg[13], 0x50);
		break;
	default:
		printf("WARNING: unsupported for SerDes2 Protocol %d\n",
		       srds_prtcl_s2);
		return -1;
	}
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

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		0, flash_esel, BOOKE_PAGESZ_256M, 1);

	set_liodns();
#ifdef CONFIG_SYS_DPAA_QBMAN
	setup_portals();
#endif

	/* Disable remote I2C connection to qixis fpga */
	QIXIS_WRITE(brdcfg[5], QIXIS_READ(brdcfg[5]) & ~BRDCFG5_IRE);

	brd_mux_lane_to_slot();
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT);

	return 0;
}

unsigned long get_board_sys_clk(void)
{
	u8 sysclk_conf = QIXIS_READ(brdcfg[1]);
#ifdef CONFIG_FSL_QIXIS_CLOCK_MEASUREMENT
	/* use accurate clock measurement */
	int freq = QIXIS_READ(clk_freq[0]) << 8 | QIXIS_READ(clk_freq[1]);
	int base = QIXIS_READ(clk_base[0]) << 8 | QIXIS_READ(clk_base[1]);
	u32 val;

	val =  freq * base;
	if (val) {
		debug("SYS Clock measurement is: %d\n", val);
		return val;
	} else {
		printf("Warning: SYS clock measurement is invalid, ");
		printf("using value from brdcfg1.\n");
	}
#endif

	switch (sysclk_conf & 0x0F) {
	case QIXIS_SYSCLK_83:
		return 83333333;
	case QIXIS_SYSCLK_100:
		return 100000000;
	case QIXIS_SYSCLK_125:
		return 125000000;
	case QIXIS_SYSCLK_133:
		return 133333333;
	case QIXIS_SYSCLK_150:
		return 150000000;
	case QIXIS_SYSCLK_160:
		return 160000000;
	case QIXIS_SYSCLK_166:
		return 166666666;
	}
	return 66666666;
}

unsigned long get_board_ddr_clk(void)
{
	u8 ddrclk_conf = QIXIS_READ(brdcfg[1]);
#ifdef CONFIG_FSL_QIXIS_CLOCK_MEASUREMENT
	/* use accurate clock measurement */
	int freq = QIXIS_READ(clk_freq[2]) << 8 | QIXIS_READ(clk_freq[3]);
	int base = QIXIS_READ(clk_base[0]) << 8 | QIXIS_READ(clk_base[1]);
	u32 val;

	val =  freq * base;
	if (val) {
		debug("DDR Clock measurement is: %d\n", val);
		return val;
	} else {
		printf("Warning: DDR clock measurement is invalid, ");
		printf("using value from brdcfg1.\n");
	}
#endif

	switch ((ddrclk_conf & 0x30) >> 4) {
	case QIXIS_DDRCLK_100:
		return 100000000;
	case QIXIS_DDRCLK_125:
		return 125000000;
	case QIXIS_DDRCLK_133:
		return 133333333;
	}
	return 66666666;
}

int misc_init_r(void)
{
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
	fdt_fixup_dr_usb(blob, bd);

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
	fdt_fixup_board_enet(blob);
#endif
}

/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/ppa.h>
#include <asm/arch/soc.h>
#include <hwconfig.h>
#include <ahci.h>
#include <mmc.h>
#include <scsi.h>
#include <fm_eth.h>
#include <fsl_csu.h>
#include <fsl_esdhc.h>
#include "cpld.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	static const char *freq[2] = {"100.00MHZ", "156.25MHZ"};
	u8 cfg_rcw_src1, cfg_rcw_src2;
	u16 cfg_rcw_src;
	u8 sd1refclk_sel;

	puts("Board: LS1046ARDB, boot from ");

	cfg_rcw_src1 = CPLD_READ(cfg_rcw_src1);
	cfg_rcw_src2 = CPLD_READ(cfg_rcw_src2);
	cpld_rev_bit(&cfg_rcw_src1);
	cfg_rcw_src = cfg_rcw_src1;
	cfg_rcw_src = (cfg_rcw_src << 1) | cfg_rcw_src2;

	if (cfg_rcw_src == 0x44)
		printf("QSPI vBank %d\n", CPLD_READ(vbank));
	else if (cfg_rcw_src == 0x40)
		puts("SD\n");
	else
		puts("Invalid setting of SW5\n");

	printf("CPLD:  V%x.%x\nPCBA:  V%x.0\n", CPLD_READ(cpld_ver),
	       CPLD_READ(cpld_ver_sub), CPLD_READ(pcba_ver));

	puts("SERDES Reference Clocks:\n");
	sd1refclk_sel = CPLD_READ(sd1refclk_sel);
	printf("SD1_CLK1 = %s, SD1_CLK2 = %s\n", freq[sd1refclk_sel], freq[0]);

	return 0;
}

int dram_init(void)
{
	gd->ram_size = initdram(0);

	return 0;
}

int board_early_init_f(void)
{
	fsl_lsch2_early_init_f();

	return 0;
}

int board_init(void)
{
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;

#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
#endif

#ifdef CONFIG_FSL_LS_PPA
	ppa_init();
#endif

	/* invert AQR105 IRQ pins polarity */
	out_be32(&scfg->intpcr, AQR105_IRQ_MASK);

	return 0;
}

void config_board_mux(void)
{
#ifdef CONFIG_HAS_FSL_XHCI_USB
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;
	u32 usb_pwrfault;

	/* USB3 is not used, configure mux to IIC4_SCL/IIC4_SDA */
	out_be32(&scfg->rcwpmuxcr0, 0x3300);
	out_be32(&scfg->usbdrvvbus_selcr, SCFG_USBDRVVBUS_SELCR_USB1);
	usb_pwrfault = (SCFG_USBPWRFAULT_DEDICATED <<
			SCFG_USBPWRFAULT_USB3_SHIFT) |
			(SCFG_USBPWRFAULT_DEDICATED <<
			SCFG_USBPWRFAULT_USB2_SHIFT) |
			(SCFG_USBPWRFAULT_SHARED <<
			SCFG_USBPWRFAULT_USB1_SHIFT);
	out_be32(&scfg->usbpwrfault_selcr, usb_pwrfault);
#endif
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	config_board_mux();
	return 0;
}
#endif

int ft_board_setup(void *blob, bd_t *bd)
{
	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];

	/* fixup DT for the two DDR banks */
	base[0] = gd->bd->bi_dram[0].start;
	size[0] = gd->bd->bi_dram[0].size;
	base[1] = gd->bd->bi_dram[1].start;
	size[1] = gd->bd->bi_dram[1].size;

	fdt_fixup_memory_banks(blob, base, size, 2);
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
#endif

	return 0;
}

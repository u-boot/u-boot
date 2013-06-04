/*
 * Board functions for IGEP COM AQUILA/CYGNUS based boards
 *
 * Copyright (C) 2013, ISEE 2007 SL - http://www.isee.biz/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <errno.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

static struct wd_timer *wdtimer = (struct wd_timer *)WDT_BASE;

/* MII mode defines */
#define RMII_MODE_ENABLE	0x4D

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

#ifdef CONFIG_SPL_BUILD
static const struct ddr_data ddr3_data = {
	.datardsratio0 = K4B2G1646EBIH9_RD_DQS,
	.datawdsratio0 = K4B2G1646EBIH9_WR_DQS,
	.datafwsratio0 = K4B2G1646EBIH9_PHY_FIFO_WE,
	.datawrsratio0 = K4B2G1646EBIH9_PHY_WR_DATA,
	.datadldiff0 = PHY_DLL_LOCK_DIFF,
};

static const struct cmd_control ddr3_cmd_ctrl_data = {
	.cmd0csratio = K4B2G1646EBIH9_RATIO,
	.cmd0dldiff = K4B2G1646EBIH9_DLL_LOCK_DIFF,
	.cmd0iclkout = K4B2G1646EBIH9_INVERT_CLKOUT,

	.cmd1csratio = K4B2G1646EBIH9_RATIO,
	.cmd1dldiff = K4B2G1646EBIH9_DLL_LOCK_DIFF,
	.cmd1iclkout = K4B2G1646EBIH9_INVERT_CLKOUT,

	.cmd2csratio = K4B2G1646EBIH9_RATIO,
	.cmd2dldiff = K4B2G1646EBIH9_DLL_LOCK_DIFF,
	.cmd2iclkout = K4B2G1646EBIH9_INVERT_CLKOUT,
};

static struct emif_regs ddr3_emif_reg_data = {
	.sdram_config = K4B2G1646EBIH9_EMIF_SDCFG,
	.ref_ctrl = K4B2G1646EBIH9_EMIF_SDREF,
	.sdram_tim1 = K4B2G1646EBIH9_EMIF_TIM1,
	.sdram_tim2 = K4B2G1646EBIH9_EMIF_TIM2,
	.sdram_tim3 = K4B2G1646EBIH9_EMIF_TIM3,
	.zq_config = K4B2G1646EBIH9_ZQ_CFG,
	.emif_ddr_phy_ctlr_1 = K4B2G1646EBIH9_EMIF_READ_LATENCY,
};
#endif

/*
 * Early system init of muxing and clocks.
 */
void s_init(void)
{
	/*
	 * Save the boot parameters passed from romcode.
	 * We cannot delay the saving further than this,
	 * to prevent overwrites.
	 */
#ifdef CONFIG_SPL_BUILD
	save_omap_boot_params();
#endif

	/* WDT1 is already running when the bootloader gets control
	 * Disable it to avoid "random" resets
	 */
	writel(0xAAAA, &wdtimer->wdtwspr);
	while (readl(&wdtimer->wdtwwps) != 0x0)
		;
	writel(0x5555, &wdtimer->wdtwspr);
	while (readl(&wdtimer->wdtwwps) != 0x0)
		;

#ifdef CONFIG_SPL_BUILD
	/* Setup the PLLs and the clocks for the peripherals */
	pll_init();

	/* Enable RTC32K clock */
	rtc32k_enable();

	enable_uart0_pin_mux();

	uart_soft_reset();
	gd = &gdata;

	preloader_console_init();

	/* Configure board pin mux */
	enable_board_pin_mux();

	config_ddr(303, K4B2G1646EBIH9_IOCTRL_VALUE, &ddr3_data,
		   &ddr3_cmd_ctrl_data, &ddr3_emif_reg_data, 0);
#endif
}

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_DRAM_1 + 0x100;

	gpmc_init();

	return 0;
}

#if defined(CONFIG_DRIVER_TI_CPSW)
static void cpsw_control(int enabled)
{
	/* VTP can be added here */

	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_id		= 0,
		.phy_if		= PHY_INTERFACE_MODE_RMII,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 1,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};

int board_eth_init(bd_t *bis)
{
	int rv, ret = 0;
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;

	if (!eth_getenv_enetaddr("ethaddr", mac_addr)) {
		/* try reading mac address from efuse */
		mac_lo = readl(&cdev->macid0l);
		mac_hi = readl(&cdev->macid0h);
		mac_addr[0] = mac_hi & 0xFF;
		mac_addr[1] = (mac_hi & 0xFF00) >> 8;
		mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
		mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
		mac_addr[4] = mac_lo & 0xFF;
		mac_addr[5] = (mac_lo & 0xFF00) >> 8;
		if (is_valid_ether_addr(mac_addr))
			eth_setenv_enetaddr("ethaddr", mac_addr);
	}

	writel(RMII_MODE_ENABLE, &cdev->miisel);

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		ret += rv;

	return ret;
}
#endif


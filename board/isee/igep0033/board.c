/*
 * Board functions for IGEP COM AQUILA based boards
 *
 * Copyright (C) 2013, ISEE 2007 SL - http://www.isee.biz/
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

#ifdef CONFIG_SPL_BUILD
static const struct ddr_data ddr3_data = {
	.datardsratio0 = K4B2G1646EBIH9_RD_DQS,
	.datawdsratio0 = K4B2G1646EBIH9_WR_DQS,
	.datafwsratio0 = K4B2G1646EBIH9_PHY_FIFO_WE,
	.datawrsratio0 = K4B2G1646EBIH9_PHY_WR_DATA,
};

static const struct cmd_control ddr3_cmd_ctrl_data = {
	.cmd0csratio = K4B2G1646EBIH9_RATIO,
	.cmd0iclkout = K4B2G1646EBIH9_INVERT_CLKOUT,

	.cmd1csratio = K4B2G1646EBIH9_RATIO,
	.cmd1iclkout = K4B2G1646EBIH9_INVERT_CLKOUT,

	.cmd2csratio = K4B2G1646EBIH9_RATIO,
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

#define OSC    (V_OSCK/1000000)
const struct dpll_params dpll_ddr = {
		400, OSC-1, 1, -1, -1, -1, -1};

const struct dpll_params *get_dpll_ddr_params(void)
{
	return &dpll_ddr;
}

void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	enable_board_pin_mux();
}

const struct ctrl_ioregs ioregs = {
	.cm0ioctl		= K4B2G1646EBIH9_IOCTRL_VALUE,
	.cm1ioctl		= K4B2G1646EBIH9_IOCTRL_VALUE,
	.cm2ioctl		= K4B2G1646EBIH9_IOCTRL_VALUE,
	.dt0ioctl		= K4B2G1646EBIH9_IOCTRL_VALUE,
	.dt1ioctl		= K4B2G1646EBIH9_IOCTRL_VALUE,
};

void sdram_init(void)
{
	config_ddr(400, &ioregs, &ddr3_data,
		   &ddr3_cmd_ctrl_data, &ddr3_emif_reg_data, 0);
}
#endif

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

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
		.phy_addr	= 0,
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
	.bd_ram_ofs		= 0x2000,
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

	writel((GMII1_SEL_RMII | RMII1_IO_CLK_EN),
	       &cdev->miisel);

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		ret += rv;

	return ret;
}
#endif

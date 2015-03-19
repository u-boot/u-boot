/*
 * board.c
 *
 * Board functions for B&R LEIT Board
 *
 * Copyright (C) 2013 Hannes Petermaier <oe5hpm@oevsv.at>
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
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
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <power/tps65217.h>
#include "../common/bur_common.h"
#include <lcd.h>
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

/* --------------------------------------------------------------------------*/
/* -- defines for GPIO -- */
#define	REPSWITCH	(0+20)	/* GPIO0_20 */

#if defined(CONFIG_SPL_BUILD)
/* TODO: check ram-timing ! */
static const struct ddr_data ddr3_data = {
	.datardsratio0 = MT41K256M16HA125E_RD_DQS,
	.datawdsratio0 = MT41K256M16HA125E_WR_DQS,
	.datafwsratio0 = MT41K256M16HA125E_PHY_FIFO_WE,
	.datawrsratio0 = MT41K256M16HA125E_PHY_WR_DATA,
};

static const struct cmd_control ddr3_cmd_ctrl_data = {
	.cmd0csratio = MT41K256M16HA125E_RATIO,
	.cmd0iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

	.cmd1csratio = MT41K256M16HA125E_RATIO,
	.cmd1iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

	.cmd2csratio = MT41K256M16HA125E_RATIO,
	.cmd2iclkout = MT41K256M16HA125E_INVERT_CLKOUT,
};

static struct emif_regs ddr3_emif_reg_data = {
	.sdram_config = MT41K256M16HA125E_EMIF_SDCFG,
	.ref_ctrl = MT41K256M16HA125E_EMIF_SDREF,
	.sdram_tim1 = MT41K256M16HA125E_EMIF_TIM1,
	.sdram_tim2 = MT41K256M16HA125E_EMIF_TIM2,
	.sdram_tim3 = MT41K256M16HA125E_EMIF_TIM3,
	.zq_config = MT41K256M16HA125E_ZQ_CFG,
	.emif_ddr_phy_ctlr_1 = MT41K256M16HA125E_EMIF_READ_LATENCY,
};

static const struct ctrl_ioregs ddr3_ioregs = {
	.cm0ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
	.cm1ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
	.cm2ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
	.dt0ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
	.dt1ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
};

#ifdef CONFIG_SPL_OS_BOOT
/*
 * called from spl_nand.c
 * return 0 for loading linux, return 1 for loading u-boot
 */
int spl_start_uboot(void)
{
	if (0 == gpio_get_value(REPSWITCH)) {
		mdelay(1000);
		printf("SPL: entering u-boot instead kernel image.\n");
		return 1;
	}
	return 0;
}
#endif /* CONFIG_SPL_OS_BOOT */

#define OSC	(V_OSCK/1000000)
static const struct dpll_params dpll_ddr3 = { 400, OSC-1, 1, -1, -1, -1, -1};

void am33xx_spl_board_init(void)
{
	struct cm_perpll *const cmper = (struct cm_perpll *)CM_PER;
	/*struct cm_wkuppll *const cmwkup = (struct cm_wkuppll *)CM_WKUP;*/
	struct cm_dpll *const cmdpll = (struct cm_dpll *)CM_DPLL;

	/*
	 * in TRM they write a reset value of 1 (=CLK_M_OSC) for the
	 * CLKSEL_TIMER6_CLK Register, in fact reset value is 0, so we need set
	 * the source of timer6 clk to CLK_M_OSC
	 */
	writel(0x01, &cmdpll->clktimer6clk);

	/* enable additional clocks of modules which are accessed later */
	u32 *const clk_domains[] = {
		&cmper->lcdcclkstctrl,
		0
	};

	u32 *const clk_modules_tsspecific[] = {
		&cmper->lcdclkctrl,
		&cmper->timer5clkctrl,
		&cmper->timer6clkctrl,
		0
	};
	do_enable_clocks(clk_domains, clk_modules_tsspecific, 1);

	/* setup LCD-Pixel Clock */
	writel(0x2, &cmdpll->clklcdcpixelclk);	/* clock comes from perPLL M2 */

	/* setup I2C */
	enable_i2c_pin_mux();
	i2c_set_bus_num(0);
	i2c_init(CONFIG_SYS_OMAP24_I2C_SPEED, CONFIG_SYS_OMAP24_I2C_SLAVE);
	pmicsetup(0);
}

const struct dpll_params *get_dpll_ddr_params(void)
{
	return &dpll_ddr3;
}

void sdram_init(void)
{
	config_ddr(400, &ddr3_ioregs,
		   &ddr3_data,
		   &ddr3_cmd_ctrl_data,
		   &ddr3_emif_reg_data, 0);
}
#endif /* CONFIG_SPL_BUILD */

/* Basic board specific setup.  Pinmux has been handled already. */
int board_init(void)
{
#if defined(CONFIG_HW_WATCHDOG)
	hw_watchdog_init();
#endif
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
#ifdef CONFIG_NAND
	gpmc_init();
#endif
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	if (0 == gpio_get_value(REPSWITCH)) {
		lcd_position_cursor(1, 8);
		lcd_puts(
		"switching to network-console ...       ");
		setenv("bootcmd", "run netconsole");
	}
	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */

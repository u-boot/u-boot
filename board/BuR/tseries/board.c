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

DECLARE_GLOBAL_DATA_PTR;

/* --------------------------------------------------------------------------*/
/* -- defines for GPIO -- */
#define	ETHLED_ORANGE	(96+16)	/* GPIO3_16 */
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
		blink(5, 125, ETHLED_ORANGE);
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
	pmicsetup(1000);
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
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
	gpmc_init();
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	gpio_direction_output(ETHLED_ORANGE, 0);

	if (0 == gpio_get_value(REPSWITCH)) {
		printf("\n\n\n"
		"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
		"!!!!!!! recovery switch activated !!!!!!!\n"
		"!!!!!!!     running usbupdate     !!!!!!!\n"
		"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n\n");
		setenv("bootcmd", "sleep 2; run netupdate;");
	}

	printf("turning on display power+backlight ... ");
	tps65217_reg_write(TPS65217_PROT_LEVEL_NONE, TPS65217_WLEDCTRL1,
			   0x09, TPS65217_MASK_ALL_BITS);	/* 200 Hz, ON */
	tps65217_reg_write(TPS65217_PROT_LEVEL_NONE, TPS65217_WLEDCTRL2,
			   0x62, TPS65217_MASK_ALL_BITS);	/* 100% */
	printf("ok.\n");

	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */

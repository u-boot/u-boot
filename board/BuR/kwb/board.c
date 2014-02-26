/*
 * board.c
 *
 * Board functions for B&R KWB Board
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

/* -------------------------------------------------------------------------*/
/* -- defines for used GPIO Hardware -- */
#define KEY						(0+4)
#define LCD_PWR						(0+5)
#define PUSH_KEY					(0+31)
#define USB2SD_NRST					(32+29)
#define USB2SD_PWR					(96+13)
/* -------------------------------------------------------------------------*/
/* -- PSOC Resetcontroller Register defines -- */

/* I2C Address of controller */
#define	RSTCTRL_ADDR				0x75
/* Register for CTRL-word */
#define RSTCTRL_CTRLREG				0x01
/* Register for giving some information to VxWorks OS */
#define RSTCTRL_SCRATCHREG			0x04

/* -- defines for RSTCTRL_CTRLREG  -- */
#define	RSTCTRL_FORCE_PWR_NEN			0x0404

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

#define OSC	(V_OSCK/1000000)
const struct dpll_params dpll_ddr3 = { 400, OSC-1, 1, -1, -1, -1, -1};

void am33xx_spl_board_init(void)
{
	unsigned int oldspeed;
	unsigned short buf;

	struct cm_perpll *const cmper = (struct cm_perpll *)CM_PER;
	struct cm_wkuppll *const cmwkup = (struct cm_wkuppll *)CM_WKUP;
	/*
	 * enable additional clocks of modules which are accessed later from
	 * VxWorks OS
	 */
	u32 *const clk_domains[] = { 0 };

	u32 *const clk_modules_kwbspecific[] = {
		&cmwkup->wkup_adctscctrl,
		&cmper->spi1clkctrl,
		&cmper->dcan0clkctrl,
		&cmper->dcan1clkctrl,
		&cmper->epwmss0clkctrl,
		&cmper->epwmss1clkctrl,
		&cmper->epwmss2clkctrl,
		0
	};
	do_enable_clocks(clk_domains, clk_modules_kwbspecific, 1);

	/* power-OFF LCD-Display */
	gpio_direction_output(LCD_PWR, 0);

	/* setup I2C */
	enable_i2c0_pin_mux();
	i2c_init(CONFIG_SYS_OMAP24_I2C_SPEED, CONFIG_SYS_OMAP24_I2C_SLAVE);

	/* power-ON  3V3 via Resetcontroller */
	oldspeed = i2c_get_bus_speed();
	if (0 != i2c_set_bus_speed(CONFIG_SYS_OMAP24_I2C_SPEED_PSOC)) {
		buf = RSTCTRL_FORCE_PWR_NEN;
		i2c_write(RSTCTRL_ADDR, RSTCTRL_CTRLREG, 1,
			  (uint8_t *)&buf, sizeof(buf));
		i2c_set_bus_speed(oldspeed);
	} else {
		puts("ERROR: i2c_set_bus_speed failed! (turn on PWR_nEN)\n");
	}

#if defined(CONFIG_AM335X_USB0)
	/* power on USB2SD Controller */
	gpio_direction_output(USB2SD_PWR, 1);
	mdelay(1);
	/* give a reset Pulse to USB2SD Controller */
	gpio_direction_output(USB2SD_NRST, 0);
	mdelay(1);
	gpio_set_value(USB2SD_NRST, 1);
#endif
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
/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
	gpmc_init();
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	const unsigned int ton  = 250;
	const unsigned int toff = 1000;
	unsigned int cnt  = 3;
	unsigned short buf = 0xAAAA;
	unsigned int oldspeed;

	tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,
			   TPS65217_WLEDCTRL2, 0x32, 0xFF); /* 50% dimlevel */

	if (gpio_get_value(KEY)) {
		do {
			/* turn on light */
			tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,
					   TPS65217_WLEDCTRL1, 0x09, 0xFF);
			mdelay(ton);
			/* turn off light */
			tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,
					   TPS65217_WLEDCTRL1, 0x01, 0xFF);
			mdelay(toff);
			cnt--;
			if (!gpio_get_value(KEY) &&
			    gpio_get_value(PUSH_KEY) && 1 == cnt) {
				puts("updating from USB ...\n");
				setenv("bootcmd", "run usbupdate");
				break;
			} else if (!gpio_get_value(KEY)) {
				break;
			}
		} while (cnt);
	}

	switch (cnt) {
	case 0:
		puts("3 blinks ... entering BOOT mode.\n");
		buf = 0x0000;
		break;
	case 1:
		puts("2 blinks ... entering DIAGNOSE mode.\n");
		buf = 0x0F0F;
		break;
	case 2:
		puts("1 blinks ... entering SERVICE mode.\n");
		buf = 0xB4B4;
		break;
	case 3:
		puts("0 blinks ... entering RUN mode.\n");
		buf = 0x0404;
		break;
	}
	mdelay(ton);
	/* turn on light */
	tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,
			   TPS65217_WLEDCTRL1, 0x09, 0xFF);
	/* write bootinfo into scratchregister of resetcontroller */
	oldspeed = i2c_get_bus_speed();
	if (0 != i2c_set_bus_speed(CONFIG_SYS_OMAP24_I2C_SPEED_PSOC)) {
		i2c_write(RSTCTRL_ADDR, RSTCTRL_SCRATCHREG, 1,
			  (uint8_t *)&buf, sizeof(buf));
		i2c_set_bus_speed(oldspeed);
	} else {
		puts("ERROR: i2c_set_bus_speed failed! (scratchregister)\n");
	}
	/*
	 * reset VBAR registers to its reset location, VxWorks 6.9.3.2 does
	 * expect that vectors are there, original u-boot moves them to _start
	 */
	__asm__("ldr r0,=0x20000");
	__asm__("mcr p15, 0, r0, c12, c0, 0"); /* Set VBAR */

	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */

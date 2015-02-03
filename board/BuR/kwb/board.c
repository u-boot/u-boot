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
#include <lcd.h>

/* -------------------------------------------------------------------------*/
/* -- defines for used GPIO Hardware -- */
#define ESC_KEY					(0+19)
#define LCD_PWR					(0+5)
#define PUSH_KEY				(0+31)
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
#define	RSTCTRL_CAN_STB				0x4040

#define VXWORKS_BOOTLINE			0x80001100
#define DEFAULT_BOOTLINE	"cpsw(0,0):pme/vxWorks"
#define VXWORKS_USER		"u=vxWorksFTP pw=vxWorks tn=vxtarget"

DECLARE_GLOBAL_DATA_PTR;

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
		&cmper->lcdclkctrl,
		&cmper->lcdcclkstctrl,
		0
	};
	do_enable_clocks(clk_domains, clk_modules_kwbspecific, 1);
	/* setup LCD-Pixel Clock */
	writel(0x2, CM_DPLL + 0x34);
	/* power-OFF LCD-Display */
	gpio_direction_output(LCD_PWR, 0);

	/* setup I2C */
	enable_i2c0_pin_mux();
	i2c_init(CONFIG_SYS_OMAP24_I2C_SPEED, CONFIG_SYS_OMAP24_I2C_SLAVE);

	/* power-ON  3V3 via Resetcontroller */
	oldspeed = i2c_get_bus_speed();
	if (i2c_set_bus_speed(CONFIG_SYS_OMAP24_I2C_SPEED_PSOC) >= 0) {
		buf = RSTCTRL_FORCE_PWR_NEN | RSTCTRL_CAN_STB;
		i2c_write(RSTCTRL_ADDR, RSTCTRL_CTRLREG, 1,
			  (uint8_t *)&buf, sizeof(buf));
		i2c_set_bus_speed(oldspeed);
	} else {
		puts("ERROR: i2c_set_bus_speed failed! (turn on PWR_nEN)\n");
	}

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
	const unsigned int toff = 1000;
	unsigned int cnt  = 3;
	unsigned short buf = 0xAAAA;
	unsigned char scratchreg = 0;
	unsigned int oldspeed;

	/* try to read out some boot-instruction from resetcontroller */
	oldspeed = i2c_get_bus_speed();
	if (i2c_set_bus_speed(CONFIG_SYS_OMAP24_I2C_SPEED_PSOC) >= 0) {
		i2c_read(RSTCTRL_ADDR, RSTCTRL_SCRATCHREG, 1,
			 &scratchreg, sizeof(scratchreg));
		i2c_set_bus_speed(oldspeed);
	} else {
		puts("ERROR: i2c_set_bus_speed failed! (scratchregister)\n");
	}

	if (gpio_get_value(ESC_KEY)) {
		do {
			lcd_position_cursor(1, 8);
			switch (cnt) {
			case 3:
				lcd_puts(
				"release ESC-KEY to enter SERVICE-mode.");
				break;
			case 2:
				lcd_puts(
				"release ESC-KEY to enter DIAGNOSE-mode.");
				break;
			case 1:
				lcd_puts(
				"release ESC-KEY to enter BOOT-mode.    ");
				break;
			}
			mdelay(toff);
			cnt--;
			if (!gpio_get_value(ESC_KEY) &&
			    gpio_get_value(PUSH_KEY) && 2 == cnt) {
				lcd_position_cursor(1, 8);
				lcd_puts(
				"switching to network-console ...       ");
				setenv("bootcmd", "run netconsole");
				cnt = 4;
				break;
			} else if (!gpio_get_value(ESC_KEY) &&
			    gpio_get_value(PUSH_KEY) && 1 == cnt) {
				lcd_position_cursor(1, 8);
				lcd_puts(
				"updating U-BOOT from USB ...           ");
				setenv("bootcmd", "run usbupdate");
				cnt = 4;
				break;
			} else if ((!gpio_get_value(ESC_KEY) &&
				    gpio_get_value(PUSH_KEY) && cnt == 0) ||
				    (gpio_get_value(ESC_KEY) &&
				    gpio_get_value(PUSH_KEY) && cnt == 0)) {
				lcd_position_cursor(1, 8);
				lcd_puts(
				"starting script from network ...      ");
				setenv("bootcmd", "run netscript");
				cnt = 4;
				break;
			} else if (!gpio_get_value(ESC_KEY)) {
				break;
			}
		} while (cnt);
	} else if (scratchreg == 0xCC) {
		lcd_position_cursor(1, 8);
		lcd_puts(
		"starting vxworks from network ...      ");
		setenv("bootcmd", "run netboot");
		cnt = 4;
	} else if (scratchreg == 0xCD) {
		lcd_position_cursor(1, 8);
		lcd_puts(
		"starting script from network ...      ");
		setenv("bootcmd", "run netscript");
		cnt = 4;
	} else if (scratchreg == 0xCE) {
		lcd_position_cursor(1, 8);
		lcd_puts(
		"starting AR from eMMC ...             ");
		setenv("bootcmd", "run mmcboot");
		cnt = 4;
	}

	lcd_position_cursor(1, 8);
	switch (cnt) {
	case 0:
		lcd_puts("entering BOOT-mode.                    ");
		setenv("bootcmd", "run defaultAR");
		buf = 0x0000;
		break;
	case 1:
		lcd_puts("entering DIAGNOSE-mode.                ");
		buf = 0x0F0F;
		break;
	case 2:
		lcd_puts("entering SERVICE mode.                 ");
		buf = 0xB4B4;
		break;
	case 3:
		lcd_puts("loading OS...                          ");
		buf = 0x0404;
		break;
	}
	/* write bootinfo into scratchregister of resetcontroller */
	oldspeed = i2c_get_bus_speed();
	if (i2c_set_bus_speed(CONFIG_SYS_OMAP24_I2C_SPEED_PSOC) >= 0) {
		i2c_write(RSTCTRL_ADDR, RSTCTRL_SCRATCHREG, 1,
			  (uint8_t *)&buf, sizeof(buf));
		i2c_set_bus_speed(oldspeed);
	} else {
		puts("ERROR: i2c_set_bus_speed failed! (scratchregister)\n");
	}
	/* setup vxworks bootline */
	char *vxworksbootline = (char *)VXWORKS_BOOTLINE;

	/* setup default IP, in case if there is nothing in environment */
	if (!getenv("ipaddr")) {
		setenv("ipaddr", "192.168.60.1");
		setenv("netmask", "255.255.255.0");
		setenv("serverip", "192.168.60.254");
		setenv("gatewayip", "192.168.60.254");
		puts("net: had no IP! made default setup.\n");
	}

	sprintf(vxworksbootline,
		"%s h=%s e=%s:%s g=%s %s o=0x%08x;0x%08x;0x%08x;0x%08x",
		DEFAULT_BOOTLINE,
		getenv("serverip"),
		getenv("ipaddr"), getenv("netmask"),
		getenv("gatewayip"),
		VXWORKS_USER,
		(unsigned int) gd->fb_base-0x20,
		(u32)getenv_ulong("vx_memtop", 16, gd->fb_base-0x20),
		(u32)getenv_ulong("vx_romfsbase", 16, 0),
		(u32)getenv_ulong("vx_romfssize", 16, 0));

	/*
	 * reset VBAR registers to its reset location, VxWorks 6.9.3.2 does
	 * expect that vectors are there, original u-boot moves them to _start
	 */
	__asm__("ldr r0,=0x20000");
	__asm__("mcr p15, 0, r0, c12, c0, 0"); /* Set VBAR */

	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */

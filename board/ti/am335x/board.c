/*
 * board.c
 *
 * Board functions for TI AM335X based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
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
#ifdef CONFIG_SPL_BUILD
static struct uart_sys *uart_base = (struct uart_sys *)DEFAULT_UART_BASE;
#endif

/* MII mode defines */
#define MII_MODE_ENABLE		0x0
#define RGMII_MODE_ENABLE	0x3A

/* GPIO that controls power to DDR on EVM-SK */
#define GPIO_DDR_VTT_EN		7

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

static struct am335x_baseboard_id __attribute__((section (".data"))) header;

static inline int board_is_bone(void)
{
	return !strncmp(header.name, "A335BONE", HDR_NAME_LEN);
}

static inline int board_is_bone_lt(void)
{
	return !strncmp(header.name, "A335BNLT", HDR_NAME_LEN);
}

static inline int board_is_evm_sk(void)
{
	return !strncmp("A335X_SK", header.name, HDR_NAME_LEN);
}

static inline int board_is_idk(void)
{
	return !strncmp(header.config, "SKU#02", 6);
}

/*
 * Read header information from EEPROM into global structure.
 */
static int read_eeprom(void)
{
	/* Check if baseboard eeprom is available */
	if (i2c_probe(CONFIG_SYS_I2C_EEPROM_ADDR)) {
		puts("Could not probe the EEPROM; something fundamentally "
			"wrong on the I2C bus.\n");
		return -ENODEV;
	}

	/* read the eeprom using i2c */
	if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0, 2, (uchar *)&header,
							sizeof(header))) {
		puts("Could not read the EEPROM; something fundamentally"
			" wrong on the I2C bus.\n");
		return -EIO;
	}

	if (header.magic != 0xEE3355AA) {
		/*
		 * read the eeprom using i2c again,
		 * but use only a 1 byte address
		 */
		if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0, 1,
					(uchar *)&header, sizeof(header))) {
			puts("Could not read the EEPROM; something "
				"fundamentally wrong on the I2C bus.\n");
			return -EIO;
		}

		if (header.magic != 0xEE3355AA) {
			printf("Incorrect magic number (0x%x) in EEPROM\n",
					header.magic);
			return -EINVAL;
		}
	}

	return 0;
}

/* UART Defines */
#ifdef CONFIG_SPL_BUILD
#define UART_RESET		(0x1 << 1)
#define UART_CLK_RUNNING_MASK	0x1
#define UART_SMART_IDLE_EN	(0x1 << 0x3)

static void rtc32k_enable(void)
{
	struct rtc_regs *rtc = (struct rtc_regs *)AM335X_RTC_BASE;

	/*
	 * Unlock the RTC's registers.  For more details please see the
	 * RTC_SS section of the TRM.  In order to unlock we need to
	 * write these specific values (keys) in this order.
	 */
	writel(0x83e70b13, &rtc->kick0r);
	writel(0x95a4f1e0, &rtc->kick1r);

	/* Enable the RTC 32K OSC by setting bits 3 and 6. */
	writel((1 << 3) | (1 << 6), &rtc->osc);
}

static const struct ddr_data ddr2_data = {
	.datardsratio0 = ((MT47H128M16RT25E_RD_DQS<<30) |
			  (MT47H128M16RT25E_RD_DQS<<20) |
			  (MT47H128M16RT25E_RD_DQS<<10) |
			  (MT47H128M16RT25E_RD_DQS<<0)),
	.datawdsratio0 = ((MT47H128M16RT25E_WR_DQS<<30) |
			  (MT47H128M16RT25E_WR_DQS<<20) |
			  (MT47H128M16RT25E_WR_DQS<<10) |
			  (MT47H128M16RT25E_WR_DQS<<0)),
	.datawiratio0 = ((MT47H128M16RT25E_PHY_WRLVL<<30) |
			 (MT47H128M16RT25E_PHY_WRLVL<<20) |
			 (MT47H128M16RT25E_PHY_WRLVL<<10) |
			 (MT47H128M16RT25E_PHY_WRLVL<<0)),
	.datagiratio0 = ((MT47H128M16RT25E_PHY_GATELVL<<30) |
			 (MT47H128M16RT25E_PHY_GATELVL<<20) |
			 (MT47H128M16RT25E_PHY_GATELVL<<10) |
			 (MT47H128M16RT25E_PHY_GATELVL<<0)),
	.datafwsratio0 = ((MT47H128M16RT25E_PHY_FIFO_WE<<30) |
			  (MT47H128M16RT25E_PHY_FIFO_WE<<20) |
			  (MT47H128M16RT25E_PHY_FIFO_WE<<10) |
			  (MT47H128M16RT25E_PHY_FIFO_WE<<0)),
	.datawrsratio0 = ((MT47H128M16RT25E_PHY_WR_DATA<<30) |
			  (MT47H128M16RT25E_PHY_WR_DATA<<20) |
			  (MT47H128M16RT25E_PHY_WR_DATA<<10) |
			  (MT47H128M16RT25E_PHY_WR_DATA<<0)),
	.datauserank0delay = MT47H128M16RT25E_PHY_RANK0_DELAY,
	.datadldiff0 = PHY_DLL_LOCK_DIFF,
};

static const struct cmd_control ddr2_cmd_ctrl_data = {
	.cmd0csratio = MT47H128M16RT25E_RATIO,
	.cmd0dldiff = MT47H128M16RT25E_DLL_LOCK_DIFF,
	.cmd0iclkout = MT47H128M16RT25E_INVERT_CLKOUT,

	.cmd1csratio = MT47H128M16RT25E_RATIO,
	.cmd1dldiff = MT47H128M16RT25E_DLL_LOCK_DIFF,
	.cmd1iclkout = MT47H128M16RT25E_INVERT_CLKOUT,

	.cmd2csratio = MT47H128M16RT25E_RATIO,
	.cmd2dldiff = MT47H128M16RT25E_DLL_LOCK_DIFF,
	.cmd2iclkout = MT47H128M16RT25E_INVERT_CLKOUT,
};

static const struct emif_regs ddr2_emif_reg_data = {
	.sdram_config = MT47H128M16RT25E_EMIF_SDCFG,
	.ref_ctrl = MT47H128M16RT25E_EMIF_SDREF,
	.sdram_tim1 = MT47H128M16RT25E_EMIF_TIM1,
	.sdram_tim2 = MT47H128M16RT25E_EMIF_TIM2,
	.sdram_tim3 = MT47H128M16RT25E_EMIF_TIM3,
	.emif_ddr_phy_ctlr_1 = MT47H128M16RT25E_EMIF_READ_LATENCY,
};

static const struct ddr_data ddr3_data = {
	.datardsratio0 = MT41J128MJT125_RD_DQS,
	.datawdsratio0 = MT41J128MJT125_WR_DQS,
	.datafwsratio0 = MT41J128MJT125_PHY_FIFO_WE,
	.datawrsratio0 = MT41J128MJT125_PHY_WR_DATA,
	.datadldiff0 = PHY_DLL_LOCK_DIFF,
};

static const struct cmd_control ddr3_cmd_ctrl_data = {
	.cmd0csratio = MT41J128MJT125_RATIO,
	.cmd0dldiff = MT41J128MJT125_DLL_LOCK_DIFF,
	.cmd0iclkout = MT41J128MJT125_INVERT_CLKOUT,

	.cmd1csratio = MT41J128MJT125_RATIO,
	.cmd1dldiff = MT41J128MJT125_DLL_LOCK_DIFF,
	.cmd1iclkout = MT41J128MJT125_INVERT_CLKOUT,

	.cmd2csratio = MT41J128MJT125_RATIO,
	.cmd2dldiff = MT41J128MJT125_DLL_LOCK_DIFF,
	.cmd2iclkout = MT41J128MJT125_INVERT_CLKOUT,
};

static struct emif_regs ddr3_emif_reg_data = {
	.sdram_config = MT41J128MJT125_EMIF_SDCFG,
	.ref_ctrl = MT41J128MJT125_EMIF_SDREF,
	.sdram_tim1 = MT41J128MJT125_EMIF_TIM1,
	.sdram_tim2 = MT41J128MJT125_EMIF_TIM2,
	.sdram_tim3 = MT41J128MJT125_EMIF_TIM3,
	.zq_config = MT41J128MJT125_ZQ_CFG,
	.emif_ddr_phy_ctlr_1 = MT41J128MJT125_EMIF_READ_LATENCY,
};
#endif

/*
 * early system init of muxing and clocks.
 */
void s_init(void)
{
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

	/* UART softreset */
	u32 regVal;

#ifdef CONFIG_SERIAL1
	enable_uart0_pin_mux();
#endif /* CONFIG_SERIAL1 */
#ifdef CONFIG_SERIAL2
	enable_uart1_pin_mux();
#endif /* CONFIG_SERIAL2 */
#ifdef CONFIG_SERIAL3
	enable_uart2_pin_mux();
#endif /* CONFIG_SERIAL3 */
#ifdef CONFIG_SERIAL4
	enable_uart3_pin_mux();
#endif /* CONFIG_SERIAL4 */
#ifdef CONFIG_SERIAL5
	enable_uart4_pin_mux();
#endif /* CONFIG_SERIAL5 */
#ifdef CONFIG_SERIAL6
	enable_uart5_pin_mux();
#endif /* CONFIG_SERIAL6 */

	regVal = readl(&uart_base->uartsyscfg);
	regVal |= UART_RESET;
	writel(regVal, &uart_base->uartsyscfg);
	while ((readl(&uart_base->uartsyssts) &
		UART_CLK_RUNNING_MASK) != UART_CLK_RUNNING_MASK)
		;

	/* Disable smart idle */
	regVal = readl(&uart_base->uartsyscfg);
	regVal |= UART_SMART_IDLE_EN;
	writel(regVal, &uart_base->uartsyscfg);

	gd = &gdata;

	preloader_console_init();

	/* Initalize the board header */
	enable_i2c0_pin_mux();
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	if (read_eeprom() < 0)
		puts("Could not get board ID.\n");

	enable_board_pin_mux(&header);
	if (board_is_evm_sk()) {
		/*
		 * EVM SK 1.2A and later use gpio0_7 to enable DDR3.
		 * This is safe enough to do on older revs.
		 */
		gpio_request(GPIO_DDR_VTT_EN, "ddr_vtt_en");
		gpio_direction_output(GPIO_DDR_VTT_EN, 1);
	}

	if (board_is_evm_sk() || board_is_bone_lt())
		config_ddr(303, MT41J128MJT125_IOCTRL_VALUE, &ddr3_data,
			   &ddr3_cmd_ctrl_data, &ddr3_emif_reg_data);
	else
		config_ddr(266, MT47H128M16RT25E_IOCTRL_VALUE, &ddr2_data,
			   &ddr2_cmd_ctrl_data, &ddr2_emif_reg_data);
#endif
}

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	if (read_eeprom() < 0)
		puts("Could not get board ID.\n");

	gd->bd->bi_boot_params = PHYS_DRAM_1 + 0x100;

	gpmc_init();

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	char safe_string[HDR_NAME_LEN + 1];

	/* Now set variables based on the header. */
	strncpy(safe_string, (char *)header.name, sizeof(header.name));
	safe_string[sizeof(header.name)] = 0;
	setenv("board_name", safe_string);

	strncpy(safe_string, (char *)header.version, sizeof(header.version));
	safe_string[sizeof(header.version)] = 0;
	setenv("board_rev", safe_string);
#endif

	return 0;
}
#endif

#ifdef CONFIG_DRIVER_TI_CPSW
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
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_id		= 1,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= AM335X_CPSW_MDIO_BASE,
	.cpsw_base		= AM335X_CPSW_BASE,
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
#endif

#if defined(CONFIG_DRIVER_TI_CPSW) || \
	(defined(CONFIG_USB_ETHER) && defined(CONFIG_MUSB_GADGET))
int board_eth_init(bd_t *bis)
{
	int rv, n = 0;
#ifdef CONFIG_DRIVER_TI_CPSW
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;

	if (!eth_getenv_enetaddr("ethaddr", mac_addr)) {
		debug("<ethaddr> not set. Reading from E-fuse\n");
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
		else
			goto try_usbether;
	}

	if (board_is_bone() || board_is_bone_lt() || board_is_idk()) {
		writel(MII_MODE_ENABLE, &cdev->miisel);
		cpsw_slaves[0].phy_if = cpsw_slaves[1].phy_if =
				PHY_INTERFACE_MODE_MII;
	} else {
		writel(RGMII_MODE_ENABLE, &cdev->miisel);
		cpsw_slaves[0].phy_if = cpsw_slaves[1].phy_if =
				PHY_INTERFACE_MODE_RGMII;
	}

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		n += rv;
#endif
try_usbether:
#if defined(CONFIG_USB_ETHER) && !defined(CONFIG_SPL_BUILD)
	rv = usb_eth_initialize(bis);
	if (rv < 0)
		printf("Error %d registering USB_ETHER\n", rv);
	else
		n += rv;
#endif
	return n;
}
#endif

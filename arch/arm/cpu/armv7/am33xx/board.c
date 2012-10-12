/*
 * board.c
 *
 * Common board functions for AM33XX based boards
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

DECLARE_GLOBAL_DATA_PTR;

struct wd_timer *wdtimer = (struct wd_timer *)WDT_BASE;
struct uart_sys *uart_base = (struct uart_sys *)DEFAULT_UART_BASE;

static const struct gpio_bank gpio_bank_am33xx[4] = {
	{ (void *)AM33XX_GPIO0_BASE, METHOD_GPIO_24XX },
	{ (void *)AM33XX_GPIO1_BASE, METHOD_GPIO_24XX },
	{ (void *)AM33XX_GPIO2_BASE, METHOD_GPIO_24XX },
	{ (void *)AM33XX_GPIO3_BASE, METHOD_GPIO_24XX },
};

const struct gpio_bank *const omap_gpio_bank = gpio_bank_am33xx;

/* MII mode defines */
#define MII_MODE_ENABLE		0x0
#define RGMII_MODE_ENABLE	0xA

/* GPIO that controls power to DDR on EVM-SK */
#define GPIO_DDR_VTT_EN		7

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

static struct am335x_baseboard_id __attribute__((section (".data"))) header;

static inline int board_is_bone(void)
{
	return !strncmp(header.name, "A335BONE", HDR_NAME_LEN);
}

static inline int board_is_evm_sk(void)
{
	return !strncmp("A335X_SK", header.name, HDR_NAME_LEN);
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
#endif

/*
 * Determine what type of DDR we have.
 */
static short inline board_memory_type(void)
{
	/* The following boards are known to use DDR3. */
	if (board_is_evm_sk())
		return EMIF_REG_SDRAM_TYPE_DDR3;

	return EMIF_REG_SDRAM_TYPE_DDR2;
}

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

	/* UART softreset */
	u32 regVal;

	enable_uart0_pin_mux();

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

	config_ddr(board_memory_type());
#endif
}

#if defined(CONFIG_OMAP_HSMMC) && !defined(CONFIG_SPL_BUILD)
int board_mmc_init(bd_t *bis)
{
	int ret;
	
	ret = omap_mmc_init(0, 0, 0);
	if (ret)
		return ret;

	return omap_mmc_init(1, 0, 0);
}
#endif

void setup_clocks_for_console(void)
{
	/* Not yet implemented */
	return;
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

	return 0;
}

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

int board_eth_init(bd_t *bis)
{
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
			return -1;
	}

	if (board_is_bone()) {
		writel(MII_MODE_ENABLE, &cdev->miisel);
		cpsw_slaves[0].phy_if = cpsw_slaves[1].phy_if =
				PHY_INTERFACE_MODE_MII;
	} else {
		writel(RGMII_MODE_ENABLE, &cdev->miisel);
		cpsw_slaves[0].phy_if = cpsw_slaves[1].phy_if =
				PHY_INTERFACE_MODE_RGMII;
	}

	return cpsw_register(&cpsw_data);
}
#endif

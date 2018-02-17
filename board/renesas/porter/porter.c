/*
 * board/renesas/porter/porter.c
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
 * Copyright (C) 2015 Cogent Embedded, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <common.h>
#include <malloc.h>
#include <dm.h>
#include <dm/platform_data/serial_sh.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <netdev.h>
#include <miiphy.h>
#include <i2c.h>
#include <div64.h>
#include "qos.h"

DECLARE_GLOBAL_DATA_PTR;

#define CLK2MHZ(clk)	(clk / 1000 / 1000)
void s_init(void)
{
	struct rcar_rwdt *rwdt = (struct rcar_rwdt *)RWDT_BASE;
	struct rcar_swdt *swdt = (struct rcar_swdt *)SWDT_BASE;
	u32 stc;

	/* Watchdog init */
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA5A5A500, &swdt->swtcsra);

	/* CPU frequency setting. Set to 1.5GHz */
	stc = ((1500 / CLK2MHZ(CONFIG_SYS_CLK_FREQ)) - 1) << PLL0_STC_BIT;
	clrsetbits_le32(PLL0CR, PLL0_STC_MASK, stc);

	/* QoS */
	qos_init();
}

#define TMU0_MSTP125	BIT(25)

#define SD2CKCR		0xE615026C
#define SD_97500KHZ	0x7

int board_early_init_f(void)
{
	mstp_clrbits_le32(MSTPSR1, SMSTPCR1, TMU0_MSTP125);

	/*
	 * SD0 clock is set to 97.5MHz by default.
	 * Set SD2 to the 97.5MHz as well.
	 */
	writel(SD_97500KHZ, SD2CKCR);

	return 0;
}

#define ETHERNET_PHY_RESET	176	/* GPIO 5 22 */

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	/* Force ethernet PHY out of reset */
	gpio_request(ETHERNET_PHY_RESET, "phy_reset");
	gpio_direction_output(ETHERNET_PHY_RESET, 0);
	mdelay(10);
	gpio_direction_output(ETHERNET_PHY_RESET, 1);

	return 0;
}

int dram_init(void)
{
	if (fdtdec_setup_memory_size() != 0)
		return -EINVAL;

	return 0;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

/* porter has KSZ8041RNLI */
#define PHY_CONTROL1		0x1E
#define PHY_LED_MODE		0xC0000
#define PHY_LED_MODE_ACK	0x4000
int board_phy_config(struct phy_device *phydev)
{
	int ret = phy_read(phydev, MDIO_DEVAD_NONE, PHY_CONTROL1);
	ret &= ~PHY_LED_MODE;
	ret |= PHY_LED_MODE_ACK;
	ret = phy_write(phydev, MDIO_DEVAD_NONE, PHY_CONTROL1, (u16)ret);

	return 0;
}

const struct rmobile_sysinfo sysinfo = {
	CONFIG_ARCH_RMOBILE_BOARD_STRING
};

void reset_cpu(ulong addr)
{
	struct udevice *dev;
	const u8 pmic_bus = 6;
	const u8 pmic_addr = 0x5a;
	u8 data;
	int ret;

	ret = i2c_get_chip_for_busnum(pmic_bus, pmic_addr, 1, &dev);
	if (ret)
		hang();

	ret = dm_i2c_read(dev, 0x13, &data, 1);
	if (ret)
		hang();

	data |= BIT(1);

	ret = dm_i2c_write(dev, 0x13, &data, 1);
	if (ret)
		hang();
}

#ifdef CONFIG_SPL_BUILD
#include <spl.h>
void board_init_f(ulong dummy)
{
	board_early_init_f();
}

void spl_board_init(void)
{
	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();
}

void board_boot_order(u32 *spl_boot_list)
{
	/* Boot from SPI NOR with YMODEM UART fallback. */
	spl_boot_list[0] = BOOT_DEVICE_SPI;
	spl_boot_list[1] = BOOT_DEVICE_UART;
	spl_boot_list[2] = BOOT_DEVICE_NONE;
}
#endif

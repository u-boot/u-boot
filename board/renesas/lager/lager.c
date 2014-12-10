/*
 * board/renesas/lager/lager.c
 *     This file is lager board support.
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 * Copyright (C) 2013 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <common.h>
#include <malloc.h>
#include <netdev.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/mmc.h>
#include <miiphy.h>
#include <i2c.h>
#include <mmc.h>
#include "qos.h"

DECLARE_GLOBAL_DATA_PTR;

#define CLK2MHZ(clk)	(clk / 1000 / 1000)
void s_init(void)
{
	struct rcar_rwdt *rwdt = (struct rcar_rwdt *)RWDT_BASE;
	struct rcar_swdt *swdt = (struct rcar_swdt *)SWDT_BASE;

	/* Watchdog init */
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA5A5A500, &swdt->swtcsra);

	/* CPU frequency setting. Set to 1.4GHz */
	if (rmobile_get_cpu_rev_integer() >= R8A7790_CUT_ES2X) {
		u32 stat = 0;
		u32 stc = ((1400 / CLK2MHZ(CONFIG_SYS_CLK_FREQ)) - 1)
			<< PLL0_STC_BIT;
		clrsetbits_le32(PLL0CR, PLL0_STC_MASK, stc);

		do {
			stat = readl(PLLECR) & PLL0ST;
		} while (stat == 0x0);
	}

	/* QoS(Quality-of-Service) Init */
	qos_init();
}

#define TMU0_MSTP125	(1 << 25)
#define SCIF0_MSTP721	(1 << 21)
#define ETHER_MSTP813	(1 << 13)
#define MMC1_MSTP305    (1 << 5)

int board_early_init_f(void)
{
	/* TMU0 */
	mstp_clrbits_le32(MSTPSR1, SMSTPCR1, TMU0_MSTP125);
	/* SCIF0 */
	mstp_clrbits_le32(MSTPSR7, SMSTPCR7, SCIF0_MSTP721);
	/* ETHER */
	mstp_clrbits_le32(MSTPSR8, SMSTPCR8, ETHER_MSTP813);
	/* eMMC */
	mstp_clrbits_le32(MSTPSR3, SMSTPCR3, MMC1_MSTP305);

	return 0;
}

DECLARE_GLOBAL_DATA_PTR;
int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	/* Init PFC controller */
	r8a7790_pinmux_init();

	/* ETHER Enable */
	gpio_request(GPIO_FN_ETH_CRS_DV, NULL);
	gpio_request(GPIO_FN_ETH_RX_ER, NULL);
	gpio_request(GPIO_FN_ETH_RXD0, NULL);
	gpio_request(GPIO_FN_ETH_RXD1, NULL);
	gpio_request(GPIO_FN_ETH_LINK, NULL);
	gpio_request(GPIO_FN_ETH_REF_CLK, NULL);
	gpio_request(GPIO_FN_ETH_MDIO, NULL);
	gpio_request(GPIO_FN_ETH_TXD1, NULL);
	gpio_request(GPIO_FN_ETH_TX_EN, NULL);
	gpio_request(GPIO_FN_ETH_MAGIC, NULL);
	gpio_request(GPIO_FN_ETH_TXD0, NULL);
	gpio_request(GPIO_FN_ETH_MDC, NULL);
	gpio_request(GPIO_FN_IRQ0, NULL);

	gpio_request(GPIO_GP_5_31, NULL);	/* PHY_RST */
	gpio_direction_output(GPIO_GP_5_31, 0);
	mdelay(20);
	gpio_set_value(GPIO_GP_5_31, 1);
	udelay(1);

	return 0;
}

#define CXR24 0xEE7003C0 /* MAC address high register */
#define CXR25 0xEE7003C8 /* MAC address low register */
int board_eth_init(bd_t *bis)
{
	int ret = -ENODEV;

#ifdef CONFIG_SH_ETHER
	u32 val;
	unsigned char enetaddr[6];

	ret = sh_eth_initialize(bis);
	if (!eth_getenv_enetaddr("ethaddr", enetaddr))
		return ret;

	/* Set Mac address */
	val = enetaddr[0] << 24 | enetaddr[1] << 16 |
	    enetaddr[2] << 8 | enetaddr[3];
	writel(val, CXR24);

	val = enetaddr[4] << 8 | enetaddr[5];
	writel(val, CXR25);

#endif

	return ret;
}

/* lager has KSZ8041NL/RNL */
#define PHY_CONTROL1	0x1E
#define PHY_LED_MODE	0xC0000
#define PHY_LED_MODE_ACK	0x4000
int board_phy_config(struct phy_device *phydev)
{
	int ret = phy_read(phydev, MDIO_DEVAD_NONE, PHY_CONTROL1);
	ret &= ~PHY_LED_MODE;
	ret |= PHY_LED_MODE_ACK;
	ret = phy_write(phydev, MDIO_DEVAD_NONE, PHY_CONTROL1, (u16)ret);

	return 0;
}

int board_mmc_init(bd_t *bis)
{
	int ret = 0;

#ifdef CONFIG_SH_MMCIF
	gpio_request(GPIO_FN_MMC1_D0, NULL);
	gpio_request(GPIO_FN_MMC1_D1, NULL);
	gpio_request(GPIO_FN_MMC1_D2, NULL);
	gpio_request(GPIO_FN_MMC1_D3, NULL);
	gpio_request(GPIO_FN_MMC1_D4, NULL);
	gpio_request(GPIO_FN_MMC1_D5, NULL);
	gpio_request(GPIO_FN_MMC1_D6, NULL);
	gpio_request(GPIO_FN_MMC1_D7, NULL);
	gpio_request(GPIO_FN_MMC1_CLK, NULL);
	gpio_request(GPIO_FN_MMC1_CMD, NULL);

	ret = mmcif_mmc_init();
#endif
	return ret;
}


int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

const struct rmobile_sysinfo sysinfo = {
	CONFIG_RMOBILE_BOARD_STRING
};

void reset_cpu(ulong addr)
{
	u8 val;

	i2c_set_bus_num(3); /* PowerIC connected to ch3 */
	i2c_init(400000, 0);
	i2c_read(CONFIG_SYS_I2C_POWERIC_ADDR, 0x13, 1, &val, 1);
	val |= 0x02;
	i2c_write(CONFIG_SYS_I2C_POWERIC_ADDR, 0x13, 1, &val, 1);
}

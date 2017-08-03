/*
 * board/renesas/koelsch/koelsch.c
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: GPL-2.0
 *
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

#define TMU0_MSTP125	(1 << 25)
#define SCIF0_MSTP721	(1 << 21)
#define ETHER_MSTP813	(1 << 13)

#define SDHI0_MSTP314	(1 << 14)
#define SDHI1_MSTP312	(1 << 12)
#define SDHI2_MSTP311	(1 << 11)

#define SD1CKCR		0xE6150078
#define SD2CKCR		0xE615026C
#define SD_97500KHZ	0x7

int board_early_init_f(void)
{
	mstp_clrbits_le32(MSTPSR1, SMSTPCR1, TMU0_MSTP125);

	/* SCIF0 */
	mstp_clrbits_le32(MSTPSR7, SMSTPCR7, SCIF0_MSTP721);

	/* ETHER */
	mstp_clrbits_le32(MSTPSR8, SMSTPCR8, ETHER_MSTP813);

	/* SDHI  */
	mstp_clrbits_le32(MSTPSR3, SMSTPCR3,
			  SDHI0_MSTP314 | SDHI1_MSTP312 | SDHI2_MSTP311);

	/*
	 * SD0 clock is set to 97.5MHz by default.
	 * Set SD1 and SD2 to the 97.5MHz as well.
	 */
	writel(SD_97500KHZ, SD1CKCR);
	writel(SD_97500KHZ, SD2CKCR);

	return 0;
}

/* LSI pin pull-up control */
#define PUPR5 0xe6060114
#define PUPR5_ETH 0x3FFC0000
#define PUPR5_ETH_MAGIC	(1 << 27)
int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	/* Init PFC controller */
	r8a7791_pinmux_init();

	/* ETHER Enable */
	gpio_request(GPIO_FN_ETH_CRS_DV, NULL);
	gpio_request(GPIO_FN_ETH_RX_ER, NULL);
	gpio_request(GPIO_FN_ETH_RXD0, NULL);
	gpio_request(GPIO_FN_ETH_RXD1, NULL);
	gpio_request(GPIO_FN_ETH_LINK, NULL);
	gpio_request(GPIO_FN_ETH_REFCLK, NULL);
	gpio_request(GPIO_FN_ETH_MDIO, NULL);
	gpio_request(GPIO_FN_ETH_TXD1, NULL);
	gpio_request(GPIO_FN_ETH_TX_EN, NULL);
	gpio_request(GPIO_FN_ETH_TXD0, NULL);
	gpio_request(GPIO_FN_ETH_MDC, NULL);
	gpio_request(GPIO_FN_IRQ0, NULL);

	mstp_clrbits_le32(PUPR5, PUPR5, PUPR5_ETH & ~PUPR5_ETH_MAGIC);
	gpio_request(GPIO_GP_5_22, NULL); /* PHY_RST */
	mstp_clrbits_le32(PUPR5, PUPR5, PUPR5_ETH_MAGIC);

	gpio_direction_output(GPIO_GP_5_22, 0);
	mdelay(20);
	gpio_set_value(GPIO_GP_5_22, 1);
	udelay(1);

	return 0;
}

#define CXR24 0xEE7003C0 /* MAC address high register */
#define CXR25 0xEE7003C8 /* MAC address low register */
int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_SH_ETHER
	int ret = -ENODEV;
	u32 val;
	unsigned char enetaddr[6];

	ret = sh_eth_initialize(bis);
	if (!eth_env_get_enetaddr("ethaddr", enetaddr))
		return ret;

	/* Set Mac address */
	val = enetaddr[0] << 24 | enetaddr[1] << 16 |
		enetaddr[2] << 8 | enetaddr[3];
	writel(val, CXR24);

	val = enetaddr[4] << 8 | enetaddr[5];
	writel(val, CXR25);

	return ret;
#else
	return 0;
#endif
}

int board_mmc_init(bd_t *bis)
{
	int ret = -ENODEV;

#ifdef CONFIG_SH_SDHI
	gpio_request(GPIO_FN_SD0_DATA0, NULL);
	gpio_request(GPIO_FN_SD0_DATA1, NULL);
	gpio_request(GPIO_FN_SD0_DATA2, NULL);
	gpio_request(GPIO_FN_SD0_DATA3, NULL);
	gpio_request(GPIO_FN_SD0_CLK, NULL);
	gpio_request(GPIO_FN_SD0_CMD, NULL);
	gpio_request(GPIO_FN_SD0_CD, NULL);
	gpio_request(GPIO_FN_SD2_DATA0, NULL);
	gpio_request(GPIO_FN_SD2_DATA1, NULL);
	gpio_request(GPIO_FN_SD2_DATA2, NULL);
	gpio_request(GPIO_FN_SD2_DATA3, NULL);
	gpio_request(GPIO_FN_SD2_CLK, NULL);
	gpio_request(GPIO_FN_SD2_CMD, NULL);
	gpio_request(GPIO_FN_SD2_CD, NULL);

	/* SDHI 0 */
	gpio_request(GPIO_GP_7_17, NULL);
	gpio_request(GPIO_GP_2_12, NULL);
	gpio_direction_output(GPIO_GP_7_17, 1); /* power on */
	gpio_direction_output(GPIO_GP_2_12, 1); /* 1: 3.3V, 0: 1.8V */

	ret = sh_sdhi_init(CONFIG_SYS_SH_SDHI0_BASE, 0,
			   SH_SDHI_QUIRK_16BIT_BUF);
	if (ret)
		return ret;

	/* SDHI 1 */
	gpio_request(GPIO_GP_7_18, NULL);
	gpio_request(GPIO_GP_2_13, NULL);
	gpio_direction_output(GPIO_GP_7_18, 1); /* power on */
	gpio_direction_output(GPIO_GP_2_13, 1); /* 1: 3.3V, 0: 1.8V */

	ret = sh_sdhi_init(CONFIG_SYS_SH_SDHI1_BASE, 1, 0);
	if (ret)
		return ret;

	/* SDHI 2 */
	gpio_request(GPIO_GP_7_19, NULL);
	gpio_request(GPIO_GP_2_26, NULL);
	gpio_direction_output(GPIO_GP_7_19, 1); /* power on */
	gpio_direction_output(GPIO_GP_2_26, 1); /* 1: 3.3V, 0: 1.8V */

	ret = sh_sdhi_init(CONFIG_SYS_SH_SDHI2_BASE, 2, 0);
#endif
	return ret;
}

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

/* koelsch has KSZ8041NL/RNL */
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

const struct rmobile_sysinfo sysinfo = {
	CONFIG_ARCH_RMOBILE_BOARD_STRING
};

void reset_cpu(ulong addr)
{
	u8 val;

	i2c_set_bus_num(2); /* PowerIC connected to ch2 */
	i2c_read(CONFIG_SYS_I2C_POWERIC_ADDR, 0x13, 1, &val, 1);
	val |= 0x02;
	i2c_write(CONFIG_SYS_I2C_POWERIC_ADDR, 0x13, 1, &val, 1);
}

static const struct sh_serial_platdata serial_platdata = {
	.base = SCIF0_BASE,
	.type = PORT_SCIF,
	.clk = 14745600,
	.clk_mode = EXT_CLK,
};

U_BOOT_DEVICE(koelsch_serials) = {
	.name = "serial_sh",
	.platdata = &serial_platdata,
};

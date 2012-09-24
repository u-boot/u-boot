/*
 * Copyright (C) 2012 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * Copyright (C) 2012 Renesas Solutions Corp.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <common.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <netdev.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

#define MODEMR			(0xFFCC0020)
#define MODEMR_MASK		(0x6)
#define MODEMR_533MHZ	(0x2)

int checkboard(void)
{
	u32 r = readl(MODEMR);
	if ((r & MODEMR_MASK) & MODEMR_533MHZ)
		puts("CPU Clock: 533MHz\n");
	else
		puts("CPU Clock: 400MHz\n");

	puts("BOARD: Alpha Project. AP-SH4A-4A\n");
	return 0;
}

#define MSTPSR1			(0xFFC80044)
#define MSTPCR1			(0xFFC80034)
#define MSTPSR1_GETHER	(1 << 14)

/* IPSR3 */
#define ET0_ETXD0 (0x4 << 3)
#define ET0_GTX_CLK_A (0x4 << 6)
#define ET0_ETXD1_A (0x4 << 9)
#define ET0_ETXD2_A (0x4 << 12)
#define ET0_ETXD3_A (0x4 << 15)
#define ET0_ETXD4 (0x3 << 18)
#define ET0_ETXD5_A (0x5 << 21)
#define ET0_ETXD6_A (0x5 << 24)
#define ET0_ETXD7 (0x4 << 27)
#define IPSR3_ETH_ENABLE \
	(ET0_ETXD0 | ET0_GTX_CLK_A | ET0_ETXD1_A | ET0_ETXD2_A | \
	ET0_ETXD3_A | ET0_ETXD4 | ET0_ETXD5_A | ET0_ETXD6_A | ET0_ETXD7)

/* IPSR4 */
#define ET0_ERXD7	(0x4)
#define ET0_RX_DV	(0x4 << 3)
#define ET0_RX_ER	(0x4 << 6)
#define ET0_CRS		(0x4 << 9)
#define ET0_COL		(0x4 << 12)
#define ET0_MDC		(0x4 << 15)
#define ET0_MDIO_A	(0x3 << 18)
#define ET0_LINK_A	(0x3 << 20)
#define ET0_PHY_INT_A (0x3 << 24)

#define IPSR4_ETH_ENABLE \
	(ET0_ERXD7 | ET0_RX_DV | ET0_RX_ER | ET0_CRS | ET0_COL | \
	ET0_MDC | ET0_MDIO_A | ET0_LINK_A | ET0_PHY_INT_A)

/* IPSR8 */
#define ET0_ERXD0	(0x4 << 20)
#define ET0_ERXD1	(0x4 << 23)
#define ET0_ERXD2_A (0x3 << 26)
#define ET0_ERXD3_A (0x3 << 28)
#define IPSR8_ETH_ENABLE \
	(ET0_ERXD0 | ET0_ERXD1 | ET0_ERXD2_A | ET0_ERXD3_A)

/* IPSR10 */
#define RX4_D	(0x1 << 22)
#define TX4_D	(0x1 << 23)
#define IPSR10_SCIF_ENABLE (RX4_D | TX4_D)

/* IPSR11 */
#define ET0_ERXD4	(0x4 <<  4)
#define ET0_ERXD5	(0x4 <<  7)
#define ET0_ERXD6	(0x3 << 10)
#define ET0_TX_EN	(0x2 << 19)
#define ET0_TX_ER	(0x2 << 21)
#define ET0_TX_CLK_A (0x4 << 23)
#define ET0_RX_CLK_A (0x3 << 26)
#define IPSR11_ETH_ENABLE \
	(ET0_ERXD4 | ET0_ERXD5 | ET0_ERXD6 | ET0_TX_EN | ET0_TX_ER | \
	ET0_TX_CLK_A | ET0_RX_CLK_A)

#define GPSR1_INIT (0xFFFF7FFF)
#define GPSR2_INIT (0x4005FEFF)
#define GPSR3_INIT (0x2EFFFFFF)
#define GPSR4_INIT (0xC7000000)

int board_init(void)
{
	u32 data;

	/* Set IPSR register */
	data = readl(IPSR3);
	data |= IPSR3_ETH_ENABLE;
	writel(~data, PMMR);
	writel(data, IPSR3);

	data = readl(IPSR4);
	data |= IPSR4_ETH_ENABLE;
	writel(~data, PMMR);
	writel(data, IPSR4);

	data = readl(IPSR8);
	data |= IPSR8_ETH_ENABLE;
	writel(~data, PMMR);
	writel(data, IPSR8);

	data = readl(IPSR10);
	data |= IPSR10_SCIF_ENABLE;
	writel(~data, PMMR);
	writel(data, IPSR10);

	data = readl(IPSR11);
	data |= IPSR11_ETH_ENABLE;
	writel(~data, PMMR);
	writel(data, IPSR11);

	/* GPIO select */
	data = GPSR1_INIT;
	writel(~data, PMMR);
	writel(data, GPSR1);

	data = GPSR2_INIT;
	writel(~data, PMMR);
	writel(data, GPSR2);

	data = GPSR3_INIT;
	writel(~data, PMMR);
	writel(data, GPSR3);

	data = GPSR4_INIT;
	writel(~data, PMMR);
	writel(data, GPSR4);

	data = 0x0;
	writel(~data, PMMR);
	writel(data, GPSR5);

	/* mode select */
	data = MODESEL2_INIT;
	writel(~data, PMMR);
	writel(data, MODESEL2);

#if defined(CONFIG_SH_ETHER)
	u32 r = readl(MSTPSR1);
	if (r & MSTPSR1_GETHER)
		writel((r & ~MSTPSR1_GETHER), MSTPCR1);
#endif
	return 0;
}

int board_late_init(void)
{
	u8 mac[6];

	/* Read Mac Address and set*/
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	i2c_set_bus_num(CONFIG_SYS_I2C_MODULE);

	/* Read MAC address */
	i2c_read(0x50, 0x0, 0, mac, 6);

	if (is_valid_ether_addr(mac))
		eth_setenv_enetaddr("ethaddr", mac);

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_memsize = CONFIG_SYS_SDRAM_SIZE;
	printf("DRAM:  %dMB\n", CONFIG_SYS_SDRAM_SIZE / (1024 * 1024));

	return 0;
}

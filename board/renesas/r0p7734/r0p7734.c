/*
 * Copyright (C) 2011 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * Copyright (C) 2011 Renesas Solutions Corp.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

	puts("BOARD: Renesas Technology Corp. R0P7734C00000RZ\n");
	return 0;
}

#define MSTPSR1			(0xFFC80044)
#define MSTPCR1			(0xFFC80034)
#define MSTPSR1_GETHER	(1 << 14)

int board_init(void)
{
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
	i2c_read(0x50, 0x10, 0, mac, 6);

	if (is_valid_ethaddr(mac))
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

#ifdef CONFIG_SMC911X
int board_eth_init(bd_t *bis)
{
	int rc = 0;
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
	return rc;
}
#endif

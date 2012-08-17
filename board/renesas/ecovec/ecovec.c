/*
 * Copyright (C) 2009, 2011 Renesas Solutions Corp.
 * Copyright (C) 2009 Kuninori Morimoto <morimoto.kuninori@renesas.com>
 * Copyright (C) 2011 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <i2c.h>
#include <netdev.h>

/* USB power management register */
#define UPONCR0 0xA40501D4

int checkboard(void)
{
	puts("BOARD: ecovec\n");
	return 0;
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_memsize = CONFIG_SYS_SDRAM_SIZE;
	printf("DRAM: %dMB\n", CONFIG_SYS_SDRAM_SIZE / (1024 * 1024));
	return 0;
}

static void debug_led(u8 led)
{
	/* PDGR[0-4] is debug LED */
	outb((inb(PGDR) & ~0x0F) | (led & 0x0F), PGDR);
}

int board_late_init(void)
{
	u8 mac[6];
	char env_mac[17];

	udelay(1000);

	/* SH-Eth (PLCR, PNCR, PXCR, PSELx )*/
	outw(inw(PLCR) & ~0xFFF0, PLCR);
	outw(inw(PNCR) & ~0x000F, PNCR);
	outw(inw(PXCR) & ~0x0FC0, PXCR);
	outw((inw(PSELB) & ~0x030F) | 0x020A, PSELB);
	outw((inw(PSELC) & ~0x0307) | 0x0207, PSELC);
	outw((inw(PSELE) & ~0x00c0) | 0x0080, PSELE);

	debug_led(1 << 3);

	outl(inl(MSTPCR2) & ~0x10000000, MSTPCR2);

	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	i2c_set_bus_num(CONFIG_SYS_I2C_MODULE); /* Use I2C 1 */

	/* Read MAC address */
	i2c_read(0x50, 0x10, 0, mac, 6);

	/* Set MAC address */
	sprintf(env_mac, "%02X:%02X:%02X:%02X:%02X:%02X",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	setenv("ethaddr", env_mac);

	debug_led(0x0F);

	return 0;
}

int board_init(void)
{

	/* LED (PTG) */
	outw((inw(PGCR) & ~0xFF) | 0x66, PGCR);
	outw((inw(HIZCRA) & ~0x02), HIZCRA);

	debug_led(1 << 0);

	/* SCIF0 (PTF, PTM) */
	outw(inw(PFCR) & ~0x30, PFCR);
	outw(inw(PMCR) & ~0x0C, PMCR);
	outw((inw(PSELA) & ~0x40) | 0x40, PSELA);

	debug_led(1 << 1);

	/* RMII (PTA) */
	outw((inw(PACR) & ~0x0C) | 0x04, PACR);
	outb((inb(PADR) & ~0x02) | 0x02, PADR);

	debug_led(1 << 2);

	/* USB host */
	outw((inw(PBCR) & ~0x300) | 0x100, PBCR);
	outb((inb(PBDR) & ~0x10) | 0x10, PBDR);
	outl(inl(MSTPCR2) & 0x100000, MSTPCR2);
	outw(0x0600, UPONCR0);

	debug_led(1 << 3);

	/* debug switch */
	outw((inw(PVCR) & ~0x03) | 0x02 , PVCR);

	return 0;
}

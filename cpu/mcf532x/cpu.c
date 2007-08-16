/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>

#include <asm/immap.h>

int do_reset(cmd_tbl_t * cmdtp, bd_t * bd, int flag, int argc, char *argv[])
{
	volatile wdog_t *wdp = (wdog_t *) (MMAP_WDOG);

	wdp->cr = 0;
	udelay(1000);

	/* enable watchdog, set timeout to 0 and wait */
	wdp->cr = WTM_WCR_EN;
	while (1) ;

	/* we don't return! */
	return 0;
};

int checkcpu(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	volatile ccm_t *ccm = (ccm_t *) MMAP_CCM;
	u16 msk;
	u16 id = 0;
	u8 ver;

	puts("CPU:   ");
	msk = (ccm->cir >> 6);
	ver = (ccm->cir & 0x003f);
	switch (msk) {
	case 0x54:
		id = 5329;
		break;
	case 0x59:
		id = 5328;
		break;
	case 0x61:
		id = 5327;
		break;
	}

	if (id) {
		printf("Freescale MCF%d (Mask:%01x Version:%x)\n", id, msk,
		       ver);
		printf("       CPU CLK %d Mhz BUS CLK %d Mhz\n",
		       (int)(gd->cpu_clk / 1000000),
		       (int)(gd->bus_clk / 1000000));
	}

	return 0;
};

#if defined(CONFIG_WATCHDOG)
/* Called by macro WATCHDOG_RESET */
void watchdog_reset(void)
{
	volatile wdog_t *wdp = (wdog_t *) (MMAP_WDOG);

	wdp->sr = 0x5555;	/* Count register */
}

int watchdog_disable(void)
{
	volatile wdog_t *wdp = (wdog_t *) (MMAP_WDOG);

	/* UserManual, once the wdog is disabled, wdog cannot be re-enabled */
	wdp->cr |= WTM_WCR_HALTED;	/* halted watchdog timer */

	puts("WATCHDOG:disabled\n");
	return (0);
}

int watchdog_init(void)
{
	volatile wdog_t *wdp = (wdog_t *) (MMAP_WDOG);
	u32 wdog_module = 0;

	/* set timeout and enable watchdog */
	wdog_module = ((CFG_CLK / 1000) * CONFIG_WATCHDOG_TIMEOUT);
	wdog_module |= (wdog_module / 8192);
	wdp->mr = wdog_module;

	wdp->cr = WTM_WCR_EN;
	puts("WATCHDOG:enabled\n");

	return (0);
}
#endif				/* CONFIG_WATCHDOG */

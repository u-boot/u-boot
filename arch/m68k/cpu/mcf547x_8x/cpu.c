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
#include <netdev.h>

#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

int do_reset(cmd_tbl_t * cmdtp, bd_t * bd, int flag, int argc, char * const argv[])
{
	volatile gptmr_t *gptmr = (gptmr_t *) (MMAP_GPTMR);

	gptmr->pre = 10;
	gptmr->cnt = 1;

	/* enable watchdog, set timeout to 0 and wait */
	gptmr->mode = GPT_TMS_SGPIO;
	gptmr->ctrl = GPT_CTRL_WDEN | GPT_CTRL_CE;

	/* we don't return! */
	return 1;
};

int checkcpu(void)
{
	volatile siu_t *siu = (siu_t *) MMAP_SIU;
	u16 id = 0;

	puts("CPU:   ");

	switch ((siu->jtagid & 0x000FF000) >> 12) {
	case 0x0C:
		id = 5485;
		break;
	case 0x0D:
		id = 5484;
		break;
	case 0x0E:
		id = 5483;
		break;
	case 0x0F:
		id = 5482;
		break;
	case 0x10:
		id = 5481;
		break;
	case 0x11:
		id = 5480;
		break;
	case 0x12:
		id = 5475;
		break;
	case 0x13:
		id = 5474;
		break;
	case 0x14:
		id = 5473;
		break;
	case 0x15:
		id = 5472;
		break;
	case 0x16:
		id = 5471;
		break;
	case 0x17:
		id = 5470;
		break;
	}

	if (id) {
		char buf1[32], buf2[32];

		printf("Freescale MCF%d\n", id);
		printf("       CPU CLK %s MHz BUS CLK %s MHz\n",
		       strmhz(buf1, gd->cpu_clk),
		       strmhz(buf2, gd->bus_clk));
	}

	return 0;
};

#if defined(CONFIG_HW_WATCHDOG)
/* Called by macro WATCHDOG_RESET */
void hw_watchdog_reset(void)
{
	volatile gptmr_t *gptmr = (gptmr_t *) (MMAP_GPTMR);

	gptmr->ocpw = 0xa5;
}

int watchdog_disable(void)
{
	volatile gptmr_t *gptmr = (gptmr_t *) (MMAP_GPTMR);

	/* UserManual, once the wdog is disabled, wdog cannot be re-enabled */
	gptmr->mode = 0;
	gptmr->ctrl = 0;

	puts("WATCHDOG:disabled\n");

	return (0);
}

int watchdog_init(void)
{

	volatile gptmr_t *gptmr = (gptmr_t *) (MMAP_GPTMR);

	gptmr->pre = CONFIG_WATCHDOG_TIMEOUT;
	gptmr->cnt = CONFIG_SYS_TIMER_PRESCALER * 1000;

	gptmr->mode = GPT_TMS_SGPIO;
	gptmr->ctrl = GPT_CTRL_CE | GPT_CTRL_WDEN;
	puts("WATCHDOG:enabled\n");

	return (0);
}
#endif				/* CONFIG_HW_WATCHDOG */

#if defined(CONFIG_FSLDMAFEC) || defined(CONFIG_MCFFEC)
/* Default initializations for MCFFEC controllers.  To override,
 * create a board-specific function called:
 * 	int board_eth_init(bd_t *bis)
 */

int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_FSLDMAFEC)
	mcdmafec_initialize(bis);
#endif
#if defined(CONFIG_MCFFEC)
	mcffec_initialize(bis);
#endif
	return 0;
}
#endif

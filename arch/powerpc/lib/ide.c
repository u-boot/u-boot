/*
 * (C) Copyright 2000-2011
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 *
 */

/* Code taken from cmd_ide.c */
#include <common.h>
#include <ata.h>
#include "ide.h"

#ifdef CONFIG_IDE_8xx_DIRECT
#include <mpc8xx.h>
#include <pcmcia.h>
DECLARE_GLOBAL_DATA_PTR;

/* Timings for IDE Interface
 *
 * SETUP / LENGTH / HOLD - cycles valid for 50 MHz clk
 * 70	   165	    30	   PIO-Mode 0, [ns]
 *  4	     9	     2		       [Cycles]
 * 50	   125	    20	   PIO-Mode 1, [ns]
 *  3	     7	     2		       [Cycles]
 * 30	   100	    15	   PIO-Mode 2, [ns]
 *  2	     6	     1		       [Cycles]
 * 30	    80	    10	   PIO-Mode 3, [ns]
 *  2	     5	     1		       [Cycles]
 * 25	    70	    10	   PIO-Mode 4, [ns]
 *  2	     4	     1		       [Cycles]
 */

static const pio_config_t pio_config_ns[IDE_MAX_PIO_MODE+1] = {
    /*  Setup  Length  Hold  */
	{ 70,	165,	30 },		/* PIO-Mode 0, [ns]	*/
	{ 50,	125,	20 },		/* PIO-Mode 1, [ns]	*/
	{ 30,	101,	15 },		/* PIO-Mode 2, [ns]	*/
	{ 30,	 80,	10 },		/* PIO-Mode 3, [ns]	*/
	{ 25,	 70,	10 },		/* PIO-Mode 4, [ns]	*/
};

static pio_config_t pio_config_clk[IDE_MAX_PIO_MODE+1];

#ifndef CONFIG_SYS_PIO_MODE
#define CONFIG_SYS_PIO_MODE	0	/* use a relaxed default */
#endif
static int pio_mode = CONFIG_SYS_PIO_MODE;

/* Make clock cycles and always round up */

#define PCMCIA_MK_CLKS(t, T) (((t) * (T) + 999U) / 1000U)

static void set_pcmcia_timing(int pmode)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	volatile pcmconf8xx_t *pcmp = &(immr->im_pcmcia);
	ulong timings;

	debug("Set timing for PIO Mode %d\n", pmode);

	timings = PCMCIA_SHT(pio_config_clk[pmode].t_hold)
		| PCMCIA_SST(pio_config_clk[pmode].t_setup)
		| PCMCIA_SL(pio_config_clk[pmode].t_length);

	/*
	 * IDE 0
	 */
	pcmp->pcmc_pbr0 = CONFIG_SYS_PCMCIA_PBR0;
#if (CONFIG_SYS_PCMCIA_POR0 != 0)
	pcmp->pcmc_por0 = CONFIG_SYS_PCMCIA_POR0 | timings;
#else
	pcmp->pcmc_por0 = CONFIG_SYS_PCMCIA_POR0;
#endif
	debug("PBR0: %08x  POR0: %08x\n", pcmp->pcmc_pbr0, pcmp->pcmc_por0);

	pcmp->pcmc_pbr1 = CONFIG_SYS_PCMCIA_PBR1;
#if (CONFIG_SYS_PCMCIA_POR1 != 0)
	pcmp->pcmc_por1 = CONFIG_SYS_PCMCIA_POR1 | timings;
#else
	pcmp->pcmc_por1 = CONFIG_SYS_PCMCIA_POR1;
#endif
	debug("PBR1: %08x  POR1: %08x\n", pcmp->pcmc_pbr1, pcmp->pcmc_por1);

	pcmp->pcmc_pbr2 = CONFIG_SYS_PCMCIA_PBR2;
#if (CONFIG_SYS_PCMCIA_POR2 != 0)
	pcmp->pcmc_por2 = CONFIG_SYS_PCMCIA_POR2 | timings;
#else
	pcmp->pcmc_por2 = CONFIG_SYS_PCMCIA_POR2;
#endif
	debug("PBR2: %08x  POR2: %08x\n", pcmp->pcmc_pbr2, pcmp->pcmc_por2);

	pcmp->pcmc_pbr3 = CONFIG_SYS_PCMCIA_PBR3;
#if (CONFIG_SYS_PCMCIA_POR3 != 0)
	pcmp->pcmc_por3 = CONFIG_SYS_PCMCIA_POR3 | timings;
#else
	pcmp->pcmc_por3 = CONFIG_SYS_PCMCIA_POR3;
#endif
	debug("PBR3: %08x  POR3: %08x\n", pcmp->pcmc_pbr3, pcmp->pcmc_por3);

	/*
	 * IDE 1
	 */
	pcmp->pcmc_pbr4 = CONFIG_SYS_PCMCIA_PBR4;
#if (CONFIG_SYS_PCMCIA_POR4 != 0)
	pcmp->pcmc_por4 = CONFIG_SYS_PCMCIA_POR4 | timings;
#else
	pcmp->pcmc_por4 = CONFIG_SYS_PCMCIA_POR4;
#endif
	debug("PBR4: %08x  POR4: %08x\n", pcmp->pcmc_pbr4, pcmp->pcmc_por4);

	pcmp->pcmc_pbr5 = CONFIG_SYS_PCMCIA_PBR5;
#if (CONFIG_SYS_PCMCIA_POR5 != 0)
	pcmp->pcmc_por5 = CONFIG_SYS_PCMCIA_POR5 | timings;
#else
	pcmp->pcmc_por5 = CONFIG_SYS_PCMCIA_POR5;
#endif
	debug("PBR5: %08x  POR5: %08x\n", pcmp->pcmc_pbr5, pcmp->pcmc_por5);

	pcmp->pcmc_pbr6 = CONFIG_SYS_PCMCIA_PBR6;
#if (CONFIG_SYS_PCMCIA_POR6 != 0)
	pcmp->pcmc_por6 = CONFIG_SYS_PCMCIA_POR6 | timings;
#else
	pcmp->pcmc_por6 = CONFIG_SYS_PCMCIA_POR6;
#endif
	debug("PBR6: %08x  POR6: %08x\n", pcmp->pcmc_pbr6, pcmp->pcmc_por6);

	pcmp->pcmc_pbr7 = CONFIG_SYS_PCMCIA_PBR7;
#if (CONFIG_SYS_PCMCIA_POR7 != 0)
	pcmp->pcmc_por7 = CONFIG_SYS_PCMCIA_POR7 | timings;
#else
	pcmp->pcmc_por7 = CONFIG_SYS_PCMCIA_POR7;
#endif
	debug("PBR7: %08x  POR7: %08x\n", pcmp->pcmc_pbr7, pcmp->pcmc_por7);

}

int ide_preinit(void)
{
	int i;
	/* Initialize PIO timing tables */
	for (i = 0; i <= IDE_MAX_PIO_MODE; ++i) {
		pio_config_clk[i].t_setup =
			PCMCIA_MK_CLKS(pio_config_ns[i].t_setup, gd->bus_clk);
		pio_config_clk[i].t_length =
			PCMCIA_MK_CLKS(pio_config_ns[i].t_length, gd->bus_clk);
		pio_config_clk[i].t_hold =
			PCMCIA_MK_CLKS(pio_config_ns[i].t_hold, gd->bus_clk);
		debug("PIO Mode %d: setup=%2d ns/%d clk" "  len=%3d ns/%d clk"
			"  hold=%2d ns/%d clk\n", i, pio_config_ns[i].t_setup,
			pio_config_clk[i].t_setup, pio_config_ns[i].t_length,
			pio_config_clk[i].t_length, pio_config_ns[i].t_hold,
			pio_config_clk[i].t_hold);
	}

	return 0;
}

int ide_init_postreset(void)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	volatile pcmconf8xx_t *pcmp = &(immr->im_pcmcia);

	/* PCMCIA / IDE initialization for common mem space */
	pcmp->pcmc_pgcrb = 0;

	/* start in PIO mode 0 - most relaxed timings */
	pio_mode = 0;
	set_pcmcia_timing(pio_mode);
	return 0;
}
#endif /* CONFIG_IDE_8xx_DIRECT */

#ifdef CONFIG_IDE_8xx_PCCARD
int ide_preinit(void)
{
	ide_devices_found = 0;
	/* initialize the PCMCIA IDE adapter card */
	pcmcia_on();
	if (!ide_devices_found)
		return 1;
	udelay(1000000);/* 1 s */
	return 0;
}
#endif

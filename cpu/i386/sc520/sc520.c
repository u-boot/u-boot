/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB <daniel@omicron.se>.
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

/* stuff specific for the sc520,
 * but idependent of implementation */

#include <common.h>
#include <asm/io.h>
#include <asm/ic/sc520.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * utility functions for boards based on the AMD sc520
 *
 * void init_sc520(void)
 * unsigned long init_sc520_dram(void)
 */

volatile sc520_mmcr_t *sc520_mmcr = (sc520_mmcr_t *)0xfffef000;

void init_sc520(void)
{
	/* Set the UARTxCTL register at it's slower,
	 * baud clock giving us a 1.8432 MHz reference
	 */
	sc520_mmcr->uart1ctl = 0x07;
	sc520_mmcr->uart2ctl = 0x07;

	/* first set the timer pin mapping */
	sc520_mmcr->clksel = 0x72;	/* no clock frequency selected, use 1.1892MHz */

	/* enable PCI bus arbitrer */
	sc520_mmcr->sysarbctl = 0x02;	/* enable concurrent mode */

	sc520_mmcr->sysarbmenb = 0x1f;	/* enable external grants */
	sc520_mmcr->hbctl = 0x04;	/* enable posted-writes */

	if (CONFIG_SYS_SC520_HIGH_SPEED) {
		sc520_mmcr->cpuctl = 0x02;	/* set it to 133 MHz and write back */
		gd->cpu_clk = 133000000;
		printf("## CPU Speed set to 133MHz\n");
	} else {
		sc520_mmcr->cpuctl = 0x01;	/* set it to 100 MHz and write back */
		printf("## CPU Speed set to 100MHz\n");
		gd->cpu_clk = 100000000;
	}


	/* wait at least one millisecond */
	asm("movl	$0x2000,%%ecx\n"
	    "0:		pushl %%ecx\n"
	    "popl	%%ecx\n"
	    "loop 0b\n": : : "ecx");

	/* turn on the SDRAM write buffer */
	sc520_mmcr->dbctl = 0x11;

	/* turn on the cache and disable write through */
	asm("movl	%%cr0, %%eax\n"
	    "andl	$0x9fffffff, %%eax\n"
	    "movl	%%eax, %%cr0\n"  : : : "eax");
}

unsigned long init_sc520_dram(void)
{
	bd_t *bd = gd->bd;

	u32 dram_present=0;
	u32 dram_ctrl;
#ifdef CONFIG_SYS_SDRAM_DRCTMCTL
	/* these memory control registers are set up in the assember part,
	 * in sc520_asm.S, during 'mem_init'.  If we muck with them here,
	 * after we are running a stack in RAM, we have troubles.  Besides,
	 * these refresh and delay values are better ? simply specified
	 * outright in the include/configs/{cfg} file since the HW designer
	 * simply dictates it.
	 */
#else
	int val;

	int cas_precharge_delay = CONFIG_SYS_SDRAM_PRECHARGE_DELAY;
	int refresh_rate        = CONFIG_SYS_SDRAM_REFRESH_RATE;
	int ras_cas_delay       = CONFIG_SYS_SDRAM_RAS_CAS_DELAY;

	/* set SDRAM speed here */

	refresh_rate/=78;
	if (refresh_rate<=1) {
		val = 0;  /* 7.8us */
	} else if (refresh_rate==2) {
		val = 1;  /* 15.6us */
	} else if (refresh_rate==3 || refresh_rate==4) {
		val = 2;  /* 31.2us */
	} else {
		val = 3;  /* 62.4us */
	}

	sc520_mmcr->drcctl = (sc520_mmcr->drcctl & 0xcf) | (val<<4);

	val = sc520_mmcr->drctmctl & 0xf0;

	if (cas_precharge_delay==3) {
		val |= 0x04;   /* 3T */
	} else if (cas_precharge_delay==4) {
		val |= 0x08;   /* 4T */
	} else if (cas_precharge_delay>4) {
		val |= 0x0c;
	}

	if (ras_cas_delay > 3) {
		val |= 2;
	} else {
		val |= 1;
	}
	sc520_mmcr->drctmctl = val;
#endif

	/* We read-back the configuration of the dram
	 * controller that the assembly code wrote */
	dram_ctrl = sc520_mmcr->drcbendadr;

	bd->bi_dram[0].start = 0;
	if (dram_ctrl & 0x80) {
		/* bank 0 enabled */
		dram_present = bd->bi_dram[1].start = (dram_ctrl & 0x7f) << 22;
		bd->bi_dram[0].size = bd->bi_dram[1].start;

	} else {
		bd->bi_dram[0].size = 0;
		bd->bi_dram[1].start = bd->bi_dram[0].start;
	}

	if (dram_ctrl & 0x8000) {
		/* bank 1 enabled */
		dram_present = bd->bi_dram[2].start = (dram_ctrl & 0x7f00) << 14;
		bd->bi_dram[1].size = bd->bi_dram[2].start -  bd->bi_dram[1].start;
	} else {
		bd->bi_dram[1].size = 0;
		bd->bi_dram[2].start = bd->bi_dram[1].start;
	}

	if (dram_ctrl & 0x800000) {
		/* bank 2 enabled */
		dram_present = bd->bi_dram[3].start = (dram_ctrl & 0x7f0000) << 6;
		bd->bi_dram[2].size = bd->bi_dram[3].start -  bd->bi_dram[2].start;
	} else {
		bd->bi_dram[2].size = 0;
		bd->bi_dram[3].start = bd->bi_dram[2].start;
	}

	if (dram_ctrl & 0x80000000) {
		/* bank 3 enabled */
		dram_present  = (dram_ctrl & 0x7f000000) >> 2;
		bd->bi_dram[3].size = dram_present -  bd->bi_dram[3].start;
	} else {
		bd->bi_dram[3].size = 0;
	}


#if 0
	printf("Configured %d bytes of dram\n", dram_present);
#endif
	gd->ram_size = dram_present;

	return dram_present;
}

#ifdef CONFIG_SYS_SC520_RESET
void reset_cpu(ulong addr)
{
	printf("Resetting using SC520 MMCR\n");
	/* Write a '1' to the SYS_RST of the RESCFG MMCR */
	sc520_mmcr->rescfg = 0x01;

	/* NOTREACHED */
}
#endif

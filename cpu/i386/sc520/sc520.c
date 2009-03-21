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
 * void write_mmcr_byte(u16 mmcr, u8 data)
 * void write_mmcr_word(u16 mmcr, u16 data)
 * void write_mmcr_long(u16 mmcr, u32 data)
 *
 * u8   read_mmcr_byte(u16 mmcr)
 * u16  read_mmcr_word(u16 mmcr)
 * u32  read_mmcr_long(u16 mmcr)
 *
 * void init_sc520(void)
 * unsigned long init_sc520_dram(void)
 */

static u32 mmcr_base= 0xfffef000;

void write_mmcr_byte(u16 mmcr, u8 data)
{
	writeb(data, mmcr+mmcr_base);
}

void write_mmcr_word(u16 mmcr, u16 data)
{
	writew(data, mmcr+mmcr_base);
}

void write_mmcr_long(u16 mmcr, u32 data)
{
	writel(data, mmcr+mmcr_base);
}

u8 read_mmcr_byte(u16 mmcr)
{
	return readb(mmcr+mmcr_base);
}

u16 read_mmcr_word(u16 mmcr)
{
	return readw(mmcr+mmcr_base);
}

u32 read_mmcr_long(u16 mmcr)
{
	return readl(mmcr+mmcr_base);
}


void init_sc520(void)
{
	/* Set the UARTxCTL register at it's slower,
	 * baud clock giving us a 1.8432 MHz reference
	 */
	write_mmcr_byte(SC520_UART1CTL, 7);
	write_mmcr_byte(SC520_UART2CTL, 7);

	/* first set the timer pin mapping */
	write_mmcr_byte(SC520_CLKSEL, 0x72);	/* no clock frequency selected, use 1.1892MHz */

	/* enable PCI bus arbitrer */
	write_mmcr_byte(SC520_SYSARBCTL,0x02);  /* enable concurrent mode */

	write_mmcr_word(SC520_SYSARBMENB,0x1f); /* enable external grants */
	write_mmcr_word(SC520_HBCTL,0x04);      /* enable posted-writes */


	if (CONFIG_SYS_SC520_HIGH_SPEED) {
		write_mmcr_byte(SC520_CPUCTL, 0x2);	/* set it to 133 MHz and write back */
		gd->cpu_clk = 133000000;
		printf("## CPU Speed set to 133MHz\n");
	} else {
		write_mmcr_byte(SC520_CPUCTL, 1);	/* set CPU to 100 MHz and write back cache */
		printf("## CPU Speed set to 100MHz\n");
		gd->cpu_clk = 100000000;
	}


	/* wait at least one millisecond */
	asm("movl	$0x2000,%%ecx\n"
	    "wait_loop:	pushl %%ecx\n"
	    "popl	%%ecx\n"
	    "loop wait_loop\n": : : "ecx");

	/* turn on the SDRAM write buffer */
	write_mmcr_byte(SC520_DBCTL, 0x11);

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

	write_mmcr_byte(SC520_DRCCTL, (read_mmcr_byte(SC520_DRCCTL) & 0xcf) | (val<<4));

	val = read_mmcr_byte(SC520_DRCTMCTL);
	val &= 0xf0;

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
	write_mmcr_byte(SC520_DRCTMCTL, val);
#endif

	/* We read-back the configuration of the dram
	 * controller that the assembly code wrote */
	dram_ctrl = read_mmcr_long(SC520_DRCBENDADR);

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
	write_mmcr_word(SC520_RESCFG, 0x0001);

	/* NOTREACHED */
}
#endif

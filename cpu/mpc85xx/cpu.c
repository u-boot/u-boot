/*
 * (C) Copyright 2002, 2003 Motorola Inc.
 * Xianghua Xiao (X.Xiao@motorola.com)
 *
 * (C) Copyright 2000
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
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <asm/cache.h>

/* ------------------------------------------------------------------------- */

int checkcpu (void)
{
	uint pir = get_pir();
	uint pvr = get_pvr();

	printf("Motorola PowerPC ProcessorID=%08x Rev. ",pir);
	switch(pvr) {
	default:
		printf("PVR=%08x", pvr);
		break;
	}

	printf("\n");

	return 0;
}


/* ------------------------------------------------------------------------- */

int do_reset (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	/*
	 * Initiate hard reset in debug control register DBCR0
	 * Make sure MSR[DE] = 1
	 */
	__asm__ __volatile__("lis   3, 0x7000" ::: "r3");
	mtspr(DBCR0,3);
	return 1;
}


/*
 * Get timebase clock frequency
 */
unsigned long get_tbclk (void)
{

	sys_info_t  sys_info;

	get_sys_info(&sys_info);
	return ((sys_info.freqSystemBus + 3L) / 4L);
}


#if defined(CONFIG_WATCHDOG)
void
watchdog_reset(void)
{
	int re_enable = disable_interrupts();
	reset_85xx_watchdog();
	if (re_enable) enable_interrupts();
}

void
reset_85xx_watchdog(void)
{
	/*
	 * Clear TSR(WIS) bit by writing 1
	 */
	unsigned long val;
	val = mfspr(tsr);
	val |= 0x40000000;
	mtspr(tsr, val);
}
#endif	/* CONFIG_WATCHDOG */

#if defined(CONFIG_DDR_ECC)
__inline__ void dcbz(const void* addr)
{
	__asm__ __volatile__ ("dcbz 0,%0" :: "r" (addr));
}

__inline__ void dcbf(const void* addr)
{
	__asm__ __volatile__ ("dcbf 0,%0" :: "r" (addr));
}

void dma_init(void) {
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_dma_t *dma = &immap->im_dma;

	dma->satr0 = 0x02c40000;
	dma->datr0 = 0x02c40000;
	asm("sync; isync; msync");
	return;
}

uint dma_check(void) {
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_dma_t *dma = &immap->im_dma;
	volatile uint status = dma->sr0;

	/* While the channel is busy, spin */
	while((status & 4) == 4) {
		status = dma->sr0;
	}

	if (status != 0) {
		printf ("DMA Error: status = %x\n", status);
	}
	return status;
}

int dma_xfer(void *dest, uint count, void *src) {
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_dma_t *dma = &immap->im_dma;

	dma->dar0 = (uint) dest;
	dma->sar0 = (uint) src;
	dma->bcr0 = count;
	dma->mr0 = 0xf000004;
	asm("sync;isync;msync");
	dma->mr0 = 0xf000005;
	asm("sync;isync;msync");
	return dma_check();
}
#endif

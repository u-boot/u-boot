/*
 * Copyright 2004,2007 Freescale Semiconductor, Inc.
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

int checkcpu (void)
{
	sys_info_t sysinfo;
	uint lcrr;		/* local bus clock ratio register */
	uint clkdiv;		/* clock divider portion of lcrr */
	uint pvr, svr;
	uint fam;
	uint ver;
	uint major, minor;

	svr = get_svr();
	ver = SVR_VER(svr);
	major = SVR_MAJ(svr);
	minor = SVR_MIN(svr);

	puts("CPU:   ");
	switch (ver) {
	case SVR_8540:
		puts("8540");
		break;
	case SVR_8541:
		puts("8541");
		break;
	case SVR_8555:
		puts("8555");
		break;
	case SVR_8560:
		puts("8560");
		break;
	case SVR_8548:
		puts("8548");
		break;
	case SVR_8548_E:
		puts("8548_E");
		break;
	case SVR_8544:
		puts("8544");
		break;
	case SVR_8544_E:
		puts("8544_E");
		break;
	case SVR_8568_E:
		puts("8568_E");
		break;
	default:
		puts("Unknown");
		break;
	}
	printf(", Version: %d.%d, (0x%08x)\n", major, minor, svr);

	pvr = get_pvr();
	fam = PVR_FAM(pvr);
	ver = PVR_VER(pvr);
	major = PVR_MAJ(pvr);
	minor = PVR_MIN(pvr);

	printf("Core:  ");
	switch (fam) {
	case PVR_FAM(PVR_85xx):
	    puts("E500");
	    break;
	default:
	    puts("Unknown");
	    break;
	}
	printf(", Version: %d.%d, (0x%08x)\n", major, minor, pvr);

	get_sys_info(&sysinfo);

	puts("Clock Configuration:\n");
	printf("       CPU:%4lu MHz, ", sysinfo.freqProcessor / 1000000);
	printf("CCB:%4lu MHz,\n", sysinfo.freqSystemBus / 1000000);
	printf("       DDR:%4lu MHz, ", sysinfo.freqSystemBus / 2000000);

#if defined(CFG_LBC_LCRR)
	lcrr = CFG_LBC_LCRR;
#else
	{
	    volatile immap_t *immap = (immap_t *)CFG_IMMR;
	    volatile ccsr_lbc_t *lbc= &immap->im_lbc;

	    lcrr = lbc->lcrr;
	}
#endif
	clkdiv = lcrr & 0x0f;
	if (clkdiv == 2 || clkdiv == 4 || clkdiv == 8) {
#if defined(CONFIG_MPC8548) || defined(CONFIG_MPC8544)
		/*
		 * Yes, the entire PQ38 family use the same
		 * bit-representation for twice the clock divider values.
		 */
		 clkdiv *= 2;
#endif
		printf("LBC:%4lu MHz\n",
		       sysinfo.freqSystemBus / 1000000 / clkdiv);
	} else {
		printf("LBC: unknown (lcrr: 0x%08x)\n", lcrr);
	}

	if (ver == SVR_8560) {
		printf("CPM:  %lu Mhz\n",
		       sysinfo.freqSystemBus / 1000000);
	}

	puts("L1:    D-cache 32 kB enabled\n       I-cache 32 kB enabled\n");

	return 0;
}


/* ------------------------------------------------------------------------- */

int do_reset (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	uint pvr;
	uint ver;
	pvr = get_pvr();
	ver = PVR_VER(pvr);
	if (ver & 1){
	/* e500 v2 core has reset control register */
		volatile unsigned int * rstcr;
		rstcr = (volatile unsigned int *)(CFG_IMMR + 0xE00B0);
		*rstcr = 0x2;		/* HRESET_REQ */
	}else{
	/*
	 * Initiate hard reset in debug control register DBCR0
	 * Make sure MSR[DE] = 1
	 */
		unsigned long val, msr;

		msr = mfmsr ();
		msr |= MSR_DE;
		mtmsr (msr);

		val = mfspr(DBCR0);
		val |= 0x70000000;
		mtspr(DBCR0,val);
	}
	return 1;
}


/*
 * Get timebase clock frequency
 */
unsigned long get_tbclk (void)
{

	sys_info_t  sys_info;

	get_sys_info(&sys_info);
	return ((sys_info.freqSystemBus + 7L) / 8L);
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
	val = mfspr(SPRN_TSR);
	val |= TSR_WIS;
	mtspr(SPRN_TSR, val);
}
#endif	/* CONFIG_WATCHDOG */

#if defined(CONFIG_DDR_ECC)
void dma_init(void) {
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_dma_t *dma = &immap->im_dma;

	dma->satr0 = 0x02c40000;
	dma->datr0 = 0x02c40000;
	dma->sr0 = 0xfffffff; /* clear any errors */
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

	/* clear MR0[CS] channel start bit */
	dma->mr0 &= 0x00000001;
	asm("sync;isync;msync");

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

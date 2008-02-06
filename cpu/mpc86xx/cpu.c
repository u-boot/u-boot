/*
 * Copyright 2006 Freescale Semiconductor
 * Jeff Brown
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
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
#include <mpc86xx.h>
#include <asm/fsl_law.h>


int
checkcpu(void)
{
	sys_info_t sysinfo;
	uint pvr, svr;
	uint ver;
	uint major, minor;
	uint lcrr;		/* local bus clock ratio register */
	uint clkdiv;		/* clock divider portion of lcrr */
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;

	puts("Freescale PowerPC\n");

	pvr = get_pvr();
	ver = PVR_VER(pvr);
	major = PVR_MAJ(pvr);
	minor = PVR_MIN(pvr);

	puts("CPU:\n");
	puts("    Core: ");

	switch (ver) {
	case PVR_VER(PVR_86xx):
	{
		uint msscr0 = mfspr(MSSCR0);
		printf("E600 Core %d", (msscr0 & 0x20) ? 1 : 0 );
		if (gur->pordevsr & MPC86xx_PORDEVSR_CORE1TE)
			puts("\n    Core1Translation Enabled");
		debug(" (MSSCR0=%x, PORDEVSR=%x)", msscr0, gur->pordevsr);
	}
	break;
	default:
		puts("Unknown");
		break;
	}
	printf(", Version: %d.%d, (0x%08x)\n", major, minor, pvr);

	svr = get_svr();
	ver = SVR_SOC_VER(svr);
	major = SVR_MAJ(svr);
	minor = SVR_MIN(svr);

	puts("    System: ");
	switch (ver) {
	case SVR_8641:
	    if (SVR_SUBVER(svr) == 1) {
		puts("8641D");
	    } else {
		puts("8641");
	    }
	    break;
	case SVR_8610:
		puts("8610");
		break;
	default:
		puts("Unknown");
		break;
	}
	printf(", Version: %d.%d, (0x%08x)\n", major, minor, svr);

	get_sys_info(&sysinfo);

	puts("    Clocks: ");
	printf("CPU:%4lu MHz, ", sysinfo.freqProcessor / 1000000);
	printf("MPX:%4lu MHz, ", sysinfo.freqSystemBus / 1000000);
	printf("DDR:%4lu MHz, ", sysinfo.freqSystemBus / 2000000);

#if defined(CFG_LBC_LCRR)
	lcrr = CFG_LBC_LCRR;
#else
	{
		volatile immap_t *immap = (immap_t *) CFG_IMMR;
		volatile ccsr_lbc_t *lbc = &immap->im_lbc;

		lcrr = lbc->lcrr;
	}
#endif
	clkdiv = lcrr & 0x0f;
	if (clkdiv == 2 || clkdiv == 4 || clkdiv == 8) {
		printf("LBC:%4lu MHz\n",
		       sysinfo.freqSystemBus / 1000000 / clkdiv);
	} else {
		printf("    LBC: unknown (lcrr: 0x%08x)\n", lcrr);
	}

	puts("    L2: ");
	if (get_l2cr() & 0x80000000)
		puts("Enabled\n");
	else
		puts("Disabled\n");

	return 0;
}


static inline void
soft_restart(unsigned long addr)
{
#if !defined(CONFIG_MPC8641HPCN) && !defined(CONFIG_MPC8610HPCD)

	/*
	 * SRR0 has system reset vector, SRR1 has default MSR value
	 * rfi restores MSR from SRR1 and sets the PC to the SRR0 value
	 */

	__asm__ __volatile__ ("mtspr	26, %0"		:: "r" (addr));
	__asm__ __volatile__ ("li	4, (1 << 6)"	::: "r4");
	__asm__ __volatile__ ("mtspr	27, 4");
	__asm__ __volatile__ ("rfi");

#else /* CONFIG_MPC8641HPCN */

	out8(PIXIS_BASE + PIXIS_RST, 0);

#endif /* !CONFIG_MPC8641HPCN */

	while (1) ;		/* not reached */
}


/*
 * No generic way to do board reset. Simply call soft_reset.
 */
void
do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#if !defined(CONFIG_MPC8641HPCN) && !defined(CONFIG_MPC8610HPCD)

#ifdef CFG_RESET_ADDRESS
	ulong addr = CFG_RESET_ADDRESS;
#else
	/*
	 * note: when CFG_MONITOR_BASE points to a RAM address,
	 * CFG_MONITOR_BASE - sizeof (ulong) is usually a valid
	 * address. Better pick an address known to be invalid on your
	 * system and assign it to CFG_RESET_ADDRESS.
	 */
	ulong addr = CFG_MONITOR_BASE - sizeof(ulong);
#endif

	/* flush and disable I/D cache */
	__asm__ __volatile__ ("mfspr	3, 1008"	::: "r3");
	__asm__ __volatile__ ("ori	5, 5, 0xcc00"	::: "r5");
	__asm__ __volatile__ ("ori	4, 3, 0xc00"	::: "r4");
	__asm__ __volatile__ ("andc	5, 3, 5"	::: "r5");
	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("mtspr	1008, 4");
	__asm__ __volatile__ ("isync");
	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("mtspr	1008, 5");
	__asm__ __volatile__ ("isync");
	__asm__ __volatile__ ("sync");

	soft_restart(addr);

#else /* CONFIG_MPC8641HPCN */

	out8(PIXIS_BASE + PIXIS_RST, 0);

#endif /* !CONFIG_MPC8641HPCN */

	while (1) ;		/* not reached */
}


/*
 * Get timebase clock frequency
 */
unsigned long
get_tbclk(void)
{
	sys_info_t sys_info;

	get_sys_info(&sys_info);
	return (sys_info.freqSystemBus + 3L) / 4L;
}


#if defined(CONFIG_WATCHDOG)
void
watchdog_reset(void)
{
}
#endif	/* CONFIG_WATCHDOG */


#if defined(CONFIG_DDR_ECC)
void
dma_init(void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile ccsr_dma_t *dma = &immap->im_dma;

	dma->satr0 = 0x00040000;
	dma->datr0 = 0x00040000;
	asm("sync; isync");
}

uint
dma_check(void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile ccsr_dma_t *dma = &immap->im_dma;
	volatile uint status = dma->sr0;

	/* While the channel is busy, spin */
	while ((status & 4) == 4) {
		status = dma->sr0;
	}

	if (status != 0) {
		printf("DMA Error: status = %x\n", status);
	}
	return status;
}

int
dma_xfer(void *dest, uint count, void *src)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile ccsr_dma_t *dma = &immap->im_dma;

	dma->dar0 = (uint) dest;
	dma->sar0 = (uint) src;
	dma->bcr0 = count;
	dma->mr0 = 0xf000004;
	asm("sync;isync");
	dma->mr0 = 0xf000005;
	asm("sync;isync");
	return dma_check();
}

#endif	/* CONFIG_DDR_ECC */


/*
 * Print out the state of various machine registers.
 * Currently prints out LAWs and BR0/OR0
 */
void mpc86xx_reginfo(void)
{
	immap_t *immap = (immap_t *)CFG_IMMR;
	ccsr_lbc_t *lbc = &immap->im_lbc;

	print_laws();

	printf ("Local Bus Controller Registers\n"
		"\tBR0\t0x%08X\tOR0\t0x%08X \n", in_be32(&lbc->br0), in_be32(&lbc->or0));
	printf("\tBR1\t0x%08X\tOR1\t0x%08X \n", in_be32(&lbc->br1), in_be32(&lbc->or1));
	printf("\tBR2\t0x%08X\tOR2\t0x%08X \n", in_be32(&lbc->br2), in_be32(&lbc->or2));
	printf("\tBR3\t0x%08X\tOR3\t0x%08X \n", in_be32(&lbc->br3), in_be32(&lbc->or3));
	printf("\tBR4\t0x%08X\tOR4\t0x%08X \n", in_be32(&lbc->br4), in_be32(&lbc->or4));
	printf("\tBR5\t0x%08X\tOR5\t0x%08X \n", in_be32(&lbc->br5), in_be32(&lbc->or5));
	printf("\tBR6\t0x%08X\tOR6\t0x%08X \n", in_be32(&lbc->br6), in_be32(&lbc->or6));
	printf("\tBR7\t0x%08X\tOR7\t0x%08X \n", in_be32(&lbc->br7), in_be32(&lbc->or7));

}

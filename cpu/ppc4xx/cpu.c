/*
 * (C) Copyright 2000-2003
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

/*
 * CPU specific code
 *
 * written or collected and sometimes rewritten by
 * Magnus Damm <damm@bitsmart.com>
 *
 * minor modifications by
 * Wolfgang Denk <wd@denx.de>
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <asm/cache.h>
#include <ppc4xx.h>


#if defined(CONFIG_440)
static int do_chip_reset( unsigned long sys0, unsigned long sys1 );
#endif

/* ------------------------------------------------------------------------- */

int checkcpu (void)
{
#if defined(CONFIG_405GP) || \
    defined(CONFIG_405CR) || \
    defined(CONFIG_405EP) || \
    defined(CONFIG_440)   || \
    defined(CONFIG_IOP480)
	uint pvr = get_pvr();
#endif
#if defined(CONFIG_405GP) || \
    defined(CONFIG_405CR) || \
    defined(CONFIG_405EP) || \
    defined(CONFIG_IOP480)
	DECLARE_GLOBAL_DATA_PTR;

	ulong clock = gd->cpu_clk;
	char buf[32];
#endif

#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || defined(CONFIG_405EP)
	PPC405_SYS_INFO sys_info;

	puts ("CPU:   ");

	get_sys_info(&sys_info);

#ifdef CONFIG_405GP
	puts ("IBM PowerPC 405GP");
	if (pvr == PVR_405GPR_RB) {
		putc('r');
	}
	puts (" Rev. ");
#endif
#ifdef CONFIG_405CR
	puts ("IBM PowerPC 405CR Rev. ");
#endif
#ifdef CONFIG_405EP
	puts ("IBM PowerPC 405EP Rev. ");
#endif
	switch (pvr) {
	case PVR_405GP_RB:
	case PVR_405GPR_RB:
		putc('B');
		break;
	case PVR_405GP_RC:
#ifdef CONFIG_405CR
	case PVR_405CR_RC:
#endif
		putc('C');
		break;
	case PVR_405GP_RD:
		putc('D');
		break;
#ifdef CONFIG_405GP
	case PVR_405GP_RE:
		putc('E');
		break;
#endif
	case PVR_405CR_RA:
		putc('A');
		break;
	case PVR_405CR_RB:
	case PVR_405EP_RB:
		putc('B');
		break;
	default:
		printf ("? (PVR=%08x)", pvr);
		break;
	}

	printf (" at %s MHz (PLB=%lu, OPB=%lu, EBC=%lu MHz)\n", strmhz(buf, clock),
	       sys_info.freqPLB / 1000000,
	       sys_info.freqPLB / sys_info.pllOpbDiv / 1000000,
	       sys_info.freqPLB / sys_info.pllExtBusDiv / 1000000);

#if defined(CONFIG_405GP)
	if (mfdcr(strap) & PSR_PCI_ASYNC_EN) {
		printf ("       PCI async ext clock used, ");
	} else {
		printf ("       PCI sync clock at %lu MHz, ",
		       sys_info.freqPLB / sys_info.pllPciDiv / 1000000);
	}
	printf ("%sternal PCI arbiter enabled\n",
		(mfdcr(strap) & PSR_PCI_ARBIT_EN) ? "in" : "ex");
#elif defined(CONFIG_405EP)
	printf ("       IIC Boot EEPROM %sabled\n",
		(mfdcr(cpc0_boot) & CPC0_BOOT_SEP) ? "en" : "dis");
	printf ("       PCI async ext clock used, ");
	printf ("%sternal PCI arbiter enabled\n",
		(mfdcr(cpc0_pci) & CPC0_PCI_ARBIT_EN) ? "in" : "ex");
#endif

#if defined(CONFIG_405EP)
	printf ("       16 kB I-Cache 16 kB D-Cache");
#else
	printf ("       16 kB I-Cache %d kB D-Cache",
		((pvr | 0x00000001) == PVR_405GPR_RB) ? 16 : 8);
#endif
#endif  /* defined(CONFIG_405GP) || defined(CONFIG_405CR) */

#ifdef CONFIG_IOP480
	printf ("PLX IOP480 (PVR=%08x)", pvr);
	printf (" at %s MHz:", strmhz(buf, clock));
	printf (" %u kB I-Cache", 4);
	printf (" %u kB D-Cache", 2);
#endif

#if defined(CONFIG_440)
	puts ("IBM PowerPC 440 G");
	switch(pvr) {
	case PVR_440GP_RB:
		puts("P Rev. B");
		/* See errata 1.12: CHIP_4 */
		if ((mfdcr(cpc0_sys0) != mfdcr(cpc0_strp0)) ||
		    (mfdcr(cpc0_sys1) != mfdcr(cpc0_strp1)) ){
			puts (  "\n\t CPC0_SYSx DCRs corrupted. "
				"Resetting chip ...\n");
			udelay( 1000 * 1000 ); /* Give time for serial buf to clear */
			do_chip_reset ( mfdcr(cpc0_strp0),
					mfdcr(cpc0_strp1) );
		}
		break;
	case PVR_440GP_RC:
		puts("P Rev. C");
		break;
	case PVR_440GX_RA:
		puts("X Rev. A");
		break;
	case PVR_440GX_RB:
		puts("X Rev. B");
		break;
	default:
		printf ("UNKNOWN (PVR=%08x)", pvr);
		break;
	}
#endif
	puts ("\n");

	return 0;
}


/* ------------------------------------------------------------------------- */

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	/*
	 * Initiate system reset in debug control register DBCR
	 */
	__asm__ __volatile__("lis   3, 0x3000" ::: "r3");
#if defined(CONFIG_440)
	__asm__ __volatile__("mtspr 0x134, 3");
#else
	__asm__ __volatile__("mtspr 0x3f2, 3");
#endif
	return 1;
}

#if defined(CONFIG_440)
static
int do_chip_reset (unsigned long sys0, unsigned long sys1)
{
	/* Changes to cpc0_sys0 and cpc0_sys1 require chip
	 * reset.
	 */
	mtdcr (cntrl0, mfdcr (cntrl0) | 0x80000000);	/* Set SWE */
	mtdcr (cpc0_sys0, sys0);
	mtdcr (cpc0_sys1, sys1);
	mtdcr (cntrl0, mfdcr (cntrl0) & ~0x80000000);	/* Clr SWE */
	mtspr (dbcr0, 0x20000000);	/* Reset the chip */

	return 1;
}
#endif


/*
 * Get timebase clock frequency
 */
unsigned long get_tbclk (void)
{
#if defined(CONFIG_440)

	sys_info_t  sys_info;

	get_sys_info(&sys_info);
	return (sys_info.freqProcessor);

#elif defined(CONFIG_405GP) || \
      defined(CONFIG_405CR) || \
      defined(CONFIG_405) || \
      defined(CONFIG_405EP)

	PPC405_SYS_INFO sys_info;

	get_sys_info(&sys_info);
	return (sys_info.freqProcessor);

#elif defined(CONFIG_IOP480)

	return (66000000);

#else

# error get_tbclk() not implemented

#endif

}


#if defined(CONFIG_WATCHDOG)
void
watchdog_reset(void)
{
	int re_enable = disable_interrupts();
	reset_4xx_watchdog();
	if (re_enable) enable_interrupts();
}

void
reset_4xx_watchdog(void)
{
	/*
	 * Clear TSR(WIS) bit
	 */
	mtspr(tsr, 0x40000000);
}
#endif	/* CONFIG_WATCHDOG */

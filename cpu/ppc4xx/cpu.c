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


#if defined(CONFIG_405GP)
#define PCI_ARBITER_ENABLED	(mfdcr(strap) & PSR_PCI_ARBIT_EN)
#define PCI_ASYNC_ENABLED	(mfdcr(strap) & PSR_PCI_ASYNC_EN)
#endif

#if defined(CONFIG_405EP)
#define PCI_ARBITER_ENABLED	(mfdcr(cpc0_pci) & CPC0_PCI_ARBIT_EN)
#define I2C_BOOTROM_ENABLED	(mfdcr(cpc0_boot) & CPC0_BOOT_SEP)
#endif

#if defined(CONFIG_440EP) || defined(CONFIG_440GR)
#define SDR0_SDSTP1_PAE		(0x80000000 >> 21)
#define SDR0_SDSTP1_PAME	(0x80000000 >> 27)

#define PCI_ARBITER_ENABLED	(mfdcr(cpc0_strp1) & SDR0_SDSTP1_PAE)
#define PCI_ASYNC_ENABLED	(mfdcr(cpc0_strp1) & SDR0_SDSTP1_PAME)
#endif

#if defined(CONFIG_440GP)
#define CPC0_STRP1_PAE		(0x80000000 >> 11)

#define PCI_ARBITER_ENABLED	(mfdcr(cpc0_strp1) & CPC0_STRP1_PAE)
#endif

#if defined(CONFIG_440GX)
#define SDR0_SDSTP1_PAE		(0x80000000 >> 13)

#define PCI_ARBITER_ENABLED	(mfdcr(cpc0_strp1) & SDR0_SDSTP1_PAE)
#endif

#if defined(CONFIG_440)
#define FREQ_EBC		(sys_info.freqEPB)
#else
#define FREQ_EBC		(sys_info.freqPLB / sys_info.pllExtBusDiv)
#endif


#if defined(CONFIG_440)
static int do_chip_reset(unsigned long sys0, unsigned long sys1);
#endif


int checkcpu (void)
{
#if !defined(CONFIG_405)	/* not used on Xilinx 405 FPGA implementations */
	DECLARE_GLOBAL_DATA_PTR;
	uint pvr = get_pvr();
	ulong clock = gd->cpu_clk;
	char buf[32];

#if !defined(CONFIG_IOP480)
	sys_info_t sys_info;

	puts ("CPU:   ");

	get_sys_info(&sys_info);

	puts("AMCC PowerPC 4");

#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || defined(CONFIG_405EP)
	puts("05");
#endif
#if defined(CONFIG_440)
	puts("40");
#endif

	switch (pvr) {
	case PVR_405GP_RB:
		puts("GP Rev. B");
		break;

	case PVR_405GP_RC:
		puts("GP Rev. C");
		break;

	case PVR_405GP_RD:
		puts("GP Rev. D");
		break;

#ifdef CONFIG_405GP
	case PVR_405GP_RE: /* 405GP rev E and 405CR rev C have same PVR */
		puts("GP Rev. E");
		break;
#endif

	case PVR_405CR_RA:
		puts("CR Rev. A");
		break;

	case PVR_405CR_RB:
		puts("CR Rev. B");
		break;

#ifdef CONFIG_405CR
	case PVR_405CR_RC: /* 405GP rev E and 405CR rev C have same PVR */
		puts("CR Rev. C");
		break;
#endif

	case PVR_405GPR_RB:
		puts("GPr Rev. B");
		break;

	case PVR_405EP_RB:
		puts("EP Rev. B");
		break;

#if defined(CONFIG_440)
	case PVR_440GP_RB:
		puts("GP Rev. B");
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
		puts("GP Rev. C");
		break;

	case PVR_440GX_RA:
		puts("GX Rev. A");
		break;

	case PVR_440GX_RB:
		puts("GX Rev. B");
		break;

	case PVR_440GX_RC:
		puts("GX Rev. C");
		break;

	case PVR_440GX_RF:
		puts("GX Rev. F");
		break;

	case PVR_440EP_RA:
		puts("EP Rev. A");
		break;

#ifdef CONFIG_440EP
	case PVR_440EP_RB: /* 440EP rev B and 440GR rev A have same PVR */
		puts("EP Rev. B");
		break;
#endif /*  CONFIG_440EP */

#ifdef CONFIG_440GR
	case PVR_440GR_RA: /* 440EP rev B and 440GR rev A have same PVR */
		puts("GR Rev. A");
		break;
#endif /* CONFIG_440GR */
#endif /* CONFIG_440 */

	default:
		printf (" UNKNOWN (PVR=%08x)", pvr);
		break;
	}

	printf (" at %s MHz (PLB=%lu, OPB=%lu, EBC=%lu MHz)\n", strmhz(buf, clock),
	       sys_info.freqPLB / 1000000,
	       sys_info.freqPLB / sys_info.pllOpbDiv / 1000000,
	       FREQ_EBC / 1000000);

#if defined(I2C_BOOTROM_ENABLED)
	printf ("       IIC Boot EEPROM %sabled\n", I2C_BOOTROM_ENABLED ? "en" : "dis");
#endif

#if defined(PCI_ARBITER_ENABLED)
	printf ("       %sternal PCI arbiter enabled",
		(PCI_ARBITER_ENABLED) ? "In" : "Ex");
#endif

#if defined(PCI_ASYNC_ENABLED)
	if (PCI_ASYNC_ENABLED) {
		printf (", PCI async ext clock used");
	} else {
		printf (", PCI sync clock at %lu MHz",
		       sys_info.freqPLB / sys_info.pllPciDiv / 1000000);
	}
#endif

#if defined(PCI_ARBITER_ENABLED) || defined(PCI_ASYNC_ENABLED)
	putc('\n');
#endif

#if defined(CONFIG_405EP)
	printf ("       16 kB I-Cache 16 kB D-Cache");
#elif defined(CONFIG_440)
	printf ("       32 kB I-Cache 32 kB D-Cache");
#else
	printf ("       16 kB I-Cache %d kB D-Cache",
		((pvr | 0x00000001) == PVR_405GPR_RB) ? 16 : 8);
#endif
#endif /* !defined(CONFIG_IOP480) */

#if defined(CONFIG_IOP480)
	printf ("PLX IOP480 (PVR=%08x)", pvr);
	printf (" at %s MHz:", strmhz(buf, clock));
	printf (" %u kB I-Cache", 4);
	printf (" %u kB D-Cache", 2);
#endif

#endif /* !defined(CONFIG_405) */

	putc ('\n');

	return 0;
}


/* ------------------------------------------------------------------------- */

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#if defined(CONFIG_YOSEMITE) || defined(CONFIG_YELLOWSTONE)
	/*give reset to BCSR*/
	*(unsigned char*)(CFG_BCSR_BASE | 0x06) = 0x09;

#else

	/*
	 * Initiate system reset in debug control register DBCR
	 */
	__asm__ __volatile__("lis   3, 0x3000" ::: "r3");
#if defined(CONFIG_440)
	__asm__ __volatile__("mtspr 0x134, 3");
#else
	__asm__ __volatile__("mtspr 0x3f2, 3");
#endif

#endif/* defined(CONFIG_YOSEMITE) || defined(CONFIG_YELLOWSTONE)*/
	return 1;
}

#if defined(CONFIG_440)
static int do_chip_reset (unsigned long sys0, unsigned long sys1)
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
#if !defined(CONFIG_IOP480)
	sys_info_t  sys_info;

	get_sys_info(&sys_info);
	return (sys_info.freqProcessor);
#else
	return (66000000);
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

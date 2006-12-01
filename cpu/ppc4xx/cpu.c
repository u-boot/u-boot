/*
 * (C) Copyright 2000-2006
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

#if !defined(CONFIG_405)
DECLARE_GLOBAL_DATA_PTR;
#endif

#if defined(CONFIG_BOARD_RESET)
void board_reset(void);
#endif

#if defined(CONFIG_440)
#define FREQ_EBC		(sys_info.freqEPB)
#else
#define FREQ_EBC		(sys_info.freqPLB / sys_info.pllExtBusDiv)
#endif

#if defined(CONFIG_405GP) || \
    defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)

#define PCI_ASYNC

int pci_async_enabled(void)
{
#if defined(CONFIG_405GP)
	return (mfdcr(strap) & PSR_PCI_ASYNC_EN);
#endif

#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	unsigned long val;

	mfsdr(sdr_sdstp1, val);
	return (val & SDR0_SDSTP1_PAME_MASK);
#endif
}
#endif

#if defined(CONFIG_PCI) && !defined(CONFIG_IOP480) && !defined(CONFIG_405)
int pci_arbiter_enabled(void)
{
#if defined(CONFIG_405GP)
	return (mfdcr(strap) & PSR_PCI_ARBIT_EN);
#endif

#if defined(CONFIG_405EP)
	return (mfdcr(cpc0_pci) & CPC0_PCI_ARBIT_EN);
#endif

#if defined(CONFIG_440GP)
	return (mfdcr(cpc0_strp1) & CPC0_STRP1_PAE_MASK);
#endif

#if defined(CONFIG_440GX) || \
    defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE)
	unsigned long val;

	mfsdr(sdr_sdstp1, val);
	return (val & SDR0_SDSTP1_PAE_MASK);
#endif
}
#endif

#if defined(CONFIG_405EP) || defined(CONFIG_440GX) || \
    defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE)

#define I2C_BOOTROM

int i2c_bootrom_enabled(void)
{
#if defined(CONFIG_405EP)
	return (mfdcr(cpc0_boot) & CPC0_BOOT_SEP);
#else
	unsigned long val;

	mfsdr(sdr_sdcs, val);
	return (val & SDR0_SDCS_SDD);
#endif
}

#if defined(CONFIG_440GX)
#define SDR0_PINSTP_SHIFT	29
static char *bootstrap_str[] = {
	"EBC (16 bits)",
	"EBC (8 bits)",
	"EBC (32 bits)",
	"EBC (8 bits)",
	"PCI",
	"I2C (Addr 0x54)",
	"Reserved",
	"I2C (Addr 0x50)",
};
#endif

#if defined(CONFIG_440SP) || defined(CONFIG_440SPE)
#define SDR0_PINSTP_SHIFT	30
static char *bootstrap_str[] = {
	"EBC (8 bits)",
	"PCI",
	"I2C (Addr 0x54)",
	"I2C (Addr 0x50)",
};
#endif

#if defined(CONFIG_440EP) || defined(CONFIG_440GR)
#define SDR0_PINSTP_SHIFT	29
static char *bootstrap_str[] = {
	"EBC (8 bits)",
	"PCI",
	"NAND (8 bits)",
	"EBC (16 bits)",
	"EBC (16 bits)",
	"I2C (Addr 0x54)",
	"PCI",
	"I2C (Addr 0x52)",
};
#endif

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define SDR0_PINSTP_SHIFT	29
static char *bootstrap_str[] = {
	"EBC (8 bits)",
	"EBC (16 bits)",
	"EBC (16 bits)",
	"NAND (8 bits)",
	"PCI",
	"I2C (Addr 0x54)",
	"PCI",
	"I2C (Addr 0x52)",
};
#endif

#if defined(SDR0_PINSTP_SHIFT)
static int bootstrap_option(void)
{
	unsigned long val;

	mfsdr(sdr_pinstp, val);
	return ((val & 0xe0000000) >> SDR0_PINSTP_SHIFT);
}
#endif /* SDR0_PINSTP_SHIFT */
#endif


#if defined(CONFIG_440)
static int do_chip_reset(unsigned long sys0, unsigned long sys1);
#endif


int checkcpu (void)
{
#if !defined(CONFIG_405)	/* not used on Xilinx 405 FPGA implementations */
	uint pvr = get_pvr();
	ulong clock = gd->cpu_clk;
	char buf[32];

#if !defined(CONFIG_IOP480)
	char addstr[64] = "";
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

	case PVR_440EP_RC: /* 440EP rev C and 440GR rev B have same PVR */
		puts("EP Rev. C");
		break;
#endif /*  CONFIG_440EP */

#ifdef CONFIG_440GR
	case PVR_440GR_RA: /* 440EP rev B and 440GR rev A have same PVR */
		puts("GR Rev. A");
		break;

	case PVR_440GR_RB: /* 440EP rev C and 440GR rev B have same PVR */
		puts("GR Rev. B");
		break;
#endif /* CONFIG_440GR */
#endif /* CONFIG_440 */

	case PVR_440EPX1_RA:
		puts("EPx Rev. A");
		strcpy(addstr, "Security/Kasumi support");
		break;

	case PVR_440EPX2_RA:
		puts("EPx Rev. A");
		strcpy(addstr, "No Security/Kasumi support");
		break;

	case PVR_440GRX1_RA:
		puts("GRx Rev. A");
		strcpy(addstr, "Security/Kasumi support");
		break;

	case PVR_440GRX2_RA:
		puts("GRx Rev. A");
		strcpy(addstr, "No Security/Kasumi support");
		break;

	case PVR_440SP_RA:
		puts("SP Rev. A");
		break;

	case PVR_440SP_RB:
		puts("SP Rev. B");
		break;

	case PVR_440SP_RC:
		puts("SP Rev. C");
		break;

	case PVR_440SPe_RA:
		puts("SPe Rev. A");
		break;

	case PVR_440SPe_RB:
		puts("SPe Rev. B");
		break;

	default:
		printf (" UNKNOWN (PVR=%08x)", pvr);
		break;
	}

	printf (" at %s MHz (PLB=%lu, OPB=%lu, EBC=%lu MHz)\n", strmhz(buf, clock),
	       sys_info.freqPLB / 1000000,
	       sys_info.freqPLB / sys_info.pllOpbDiv / 1000000,
	       FREQ_EBC / 1000000);

	if (addstr[0] != 0)
		printf("       %s\n", addstr);

#if defined(I2C_BOOTROM)
	printf ("       I2C boot EEPROM %sabled\n", i2c_bootrom_enabled() ? "en" : "dis");
#if defined(SDR0_PINSTP_SHIFT)
	printf ("       Bootstrap Option %c - ", (char)bootstrap_option() + 'A');
	printf ("Boot ROM Location %s\n", bootstrap_str[bootstrap_option()]);
#endif	/* SDR0_PINSTP_SHIFT */
#endif	/* I2C_BOOTROM */

#if defined(CONFIG_PCI)
	printf ("       Internal PCI arbiter %sabled", pci_arbiter_enabled() ? "en" : "dis");
#endif

#if defined(PCI_ASYNC)
	if (pci_async_enabled()) {
		printf (", PCI async ext clock used");
	} else {
		printf (", PCI sync clock at %lu MHz",
		       sys_info.freqPLB / sys_info.pllPciDiv / 1000000);
	}
#endif

#if defined(CONFIG_PCI)
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

#if defined (CONFIG_440SPE)
int ppc440spe_revB() {
	unsigned int pvr;

	pvr = get_pvr();
	if (pvr == PVR_440SPe_RB)
		return 1;
	else
		return 0;
}
#endif

/* ------------------------------------------------------------------------- */

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#if defined(CONFIG_BOARD_RESET)
	board_reset();
#else
#if defined(CFG_4xx_RESET_TYPE)
	mtspr(dbcr0, CFG_4xx_RESET_TYPE << 28);
#else
	/*
	 * Initiate system reset in debug control register DBCR
	 */
	mtspr(dbcr0, 0x30000000);
#endif /* defined(CFG_4xx_RESET_TYPE) */
#endif /* defined(CONFIG_BOARD_RESET) */

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

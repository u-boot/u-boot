/*
 * (C) Copyright 2000-2002
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
 * m8xx.c
 *
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
#include <mpc8xx.h>
#include <asm/cache.h>

static char *cpu_warning = "\n         " \
	"*** Warning: CPU Core has Silicon Bugs -- Check the Errata ***";

#if ((defined(CONFIG_MPC860) || defined(CONFIG_MPC855)) && \
     !defined(CONFIG_MPC862))
# ifdef	CONFIG_MPC855
#  define	ID_STR	"PC855"
# else
#  define	ID_STR	"PC860"
# endif

static int check_CPU (long clock, uint pvr, uint immr)
{
	volatile immap_t *immap = (immap_t *) (immr & 0xFFFF0000);
	uint k, m;
	char buf[32];
	char pre = 'X';
	char *mid = "xx";
	char *suf;

	/* the highest 16 bits should be 0x0050 for a 860 */

	if ((pvr >> 16) != 0x0050)
		return -1;

	k = (immr << 16) | *((ushort *) & immap->im_cpm.cp_dparam[0xB0]);
	m = 0;

	switch (k) {
	case 0x00020001: pre = 'p'; suf = ""; break;
	case 0x00030001: suf = ""; break;
	case 0x00120003: suf = "A"; break;
	case 0x00130003: suf = "A3"; break;

	case 0x00200004: suf = "B"; break;

	case 0x00300004: suf = "C"; break;
	case 0x00310004: suf = "C1"; m = 1;
		break;

	case 0x00200064: mid = "SR"; suf = "B"; break;
	case 0x00300065: mid = "SR"; suf = "C"; break;
	case 0x00310065: mid = "SR"; suf = "C1"; m = 1; break;
	case 0x05010000: suf = "D3"; m = 1; break;
	case 0x05020000: suf = "D4"; m = 1; break;

		/* this value is not documented anywhere */
	case 0x40000000: pre = 'P'; suf = "D"; m = 1; break;

	default: suf = NULL; break;
	}

	if (suf)
		printf ("%c" ID_STR "%sZPnn%s", pre, mid, suf);
	else
		printf ("unknown M" ID_STR " (0x%08x)", k);

	printf (" at %s MHz:", strmhz (buf, clock));

	printf (" %u kB I-Cache", checkicache () >> 10);
	printf (" %u kB D-Cache", checkdcache () >> 10);

	/* lets check and see if we're running on a 860T (or P?) */

	immap->im_cpm.cp_fec.fec_addr_low = 0x12345678;
	if (immap->im_cpm.cp_fec.fec_addr_low == 0x12345678) {
		printf (" FEC present");
	}

	if (!m) {
		puts (cpu_warning);
	}

	putc ('\n');

	return 0;
}

#elif defined(CONFIG_MPC862)

static int check_CPU (long clock, uint pvr, uint immr)
{
	volatile immap_t *immap = (immap_t *) (immr & 0xFFFF0000);
	uint k, m;
	char buf[32];
	char pre = 'X';
	char *mid = "xx";
	char *suf;

	/* the highest 16 bits should be 0x0050 for a 8xx */

	if ((pvr >> 16) != 0x0050)
		return -1;

	k = (immr << 16) | *((ushort *) & immap->im_cpm.cp_dparam[0xB0]);
	m = 0;

	switch (k) {

		/* this value is not documented anywhere */
	case 0x06000000: mid = "P"; suf = "0"; break;
	case 0x06010001: mid = "P"; suf = "A"; m = 1; break;
	case 0x07000003: mid = "P"; suf = "B"; m = 1; break;
	default: suf = NULL; break;
	}

	if (suf)
		printf ("%cPC862%sZPnn%s", pre, mid, suf);
	else
		printf ("unknown MPC862 (0x%08x)", k);

	printf (" at %s MHz:", strmhz (buf, clock));

	printf (" %u kB I-Cache", checkicache () >> 10);
	printf (" %u kB D-Cache", checkdcache () >> 10);

	/* lets check and see if we're running on a 862T (or P?) */

	immap->im_cpm.cp_fec.fec_addr_low = 0x12345678;
	if (immap->im_cpm.cp_fec.fec_addr_low == 0x12345678) {
		printf (" FEC present");
	}

	if (!m) {
		puts (cpu_warning);
	}

	putc ('\n');

	return 0;
}

#elif defined(CONFIG_MPC823)

static int check_CPU (long clock, uint pvr, uint immr)
{
	volatile immap_t *immap = (immap_t *) (immr & 0xFFFF0000);
	uint k, m;
	char buf[32];
	char *suf;

	/* the highest 16 bits should be 0x0050 for a 8xx */

	if ((pvr >> 16) != 0x0050)
		return -1;

	k = (immr << 16) | *((ushort *) & immap->im_cpm.cp_dparam[0xB0]);
	m = 0;

	switch (k) {
		/* MPC823 */
	case 0x20000000: suf = "0"; break;
	case 0x20010000: suf = "0.1"; break;
	case 0x20020000: suf = "Z2/3"; break;
	case 0x20020001: suf = "Z3"; break;
	case 0x21000000: suf = "A"; break;
	case 0x21010000: suf = "B"; m = 1; break;
	case 0x21010001: suf = "B2"; m = 1; break;
		/* MPC823E */
	case 0x24010000: suf = NULL;
			puts ("PPC823EZTnnB2");
			m = 1;
			break;
	default:
			suf = NULL;
			printf ("unknown MPC823 (0x%08x)", k);
			break;
	}
	if (suf)
		printf ("PPC823ZTnn%s", suf);

	printf (" at %s MHz:", strmhz (buf, clock));

	printf (" %u kB I-Cache", checkicache () >> 10);
	printf (" %u kB D-Cache", checkdcache () >> 10);

	/* lets check and see if we're running on a 860T (or P?) */

	immap->im_cpm.cp_fec.fec_addr_low = 0x12345678;
	if (immap->im_cpm.cp_fec.fec_addr_low == 0x12345678) {
		puts (" FEC present");
	}

	if (!m) {
		puts (cpu_warning);
	}

	putc ('\n');

	return 0;
}

#elif defined(CONFIG_MPC850)

static int check_CPU (long clock, uint pvr, uint immr)
{
	volatile immap_t *immap = (immap_t *) (immr & 0xFFFF0000);
	uint k, m;
	char buf[32];

	/* the highest 16 bits should be 0x0050 for a 8xx */

	if ((pvr >> 16) != 0x0050)
		return -1;

	k = (immr << 16) | *((ushort *) & immap->im_cpm.cp_dparam[0xB0]);
	m = 0;

	switch (k) {
	case 0x20020001:
		printf ("XPC850xxZT");
		break;
	case 0x21000065:
		printf ("XPC850xxZTA");
		break;
	case 0x21010067:
		printf ("XPC850xxZTB");
		m = 1;
		break;
	case 0x21020068:
		printf ("XPC850xxZTC");
		m = 1;
		break;
	default:
		printf ("unknown MPC850 (0x%08x)", k);
	}
	printf (" at %s MHz:", strmhz (buf, clock));

	printf (" %u kB I-Cache", checkicache () >> 10);
	printf (" %u kB D-Cache", checkdcache () >> 10);

	/* lets check and see if we're running on a 850T (or P?) */

	immap->im_cpm.cp_fec.fec_addr_low = 0x12345678;
	if (immap->im_cpm.cp_fec.fec_addr_low == 0x12345678) {
		printf (" FEC present");
	}

	if (!m) {
		puts (cpu_warning);
	}

	putc ('\n');

	return 0;
}
#else
#error CPU undefined
#endif
/* ------------------------------------------------------------------------- */

int checkcpu (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	ulong clock = gd->cpu_clk;
	uint immr = get_immr (0);	/* Return full IMMR contents */
	uint pvr = get_pvr ();

	puts ("CPU:   ");

	/* 850 has PARTNUM 20 */
	/* 801 has PARTNUM 10 */
	return check_CPU (clock, pvr, immr);
}

/* ------------------------------------------------------------------------- */
/* L1 i-cache                                                                */
/* the standard 860 has 128 sets of 16 bytes in 2 ways (= 4 kB)              */
/* the 860 P (plus) has 256 sets of 16 bytes in 4 ways (= 16 kB)             */

int checkicache (void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	u32 cacheon = rd_ic_cst () & IDC_ENABLED;

#ifdef CONFIG_IP860
	u32 k = memctl->memc_br1 & ~0x00007fff;	/* probe in flash memoryarea */
#else
	u32 k = memctl->memc_br0 & ~0x00007fff;	/* probe in flash memoryarea */
#endif
	u32 m;
	u32 lines = -1;

	wr_ic_cst (IDC_UNALL);
	wr_ic_cst (IDC_INVALL);
	wr_ic_cst (IDC_DISABLE);
	__asm__ volatile ("isync");

	while (!((m = rd_ic_cst ()) & IDC_CERR2)) {
		wr_ic_adr (k);
		wr_ic_cst (IDC_LDLCK);
		__asm__ volatile ("isync");

		lines++;
		k += 0x10;				/* the number of bytes in a cacheline */
	}

	wr_ic_cst (IDC_UNALL);
	wr_ic_cst (IDC_INVALL);

	if (cacheon)
		wr_ic_cst (IDC_ENABLE);
	else
		wr_ic_cst (IDC_DISABLE);

	__asm__ volatile ("isync");

	return lines << 4;
};

/* ------------------------------------------------------------------------- */
/* L1 d-cache                                                                */
/* the standard 860 has 128 sets of 16 bytes in 2 ways (= 4 kB)              */
/* the 860 P (plus) has 256 sets of 16 bytes in 2 ways (= 8 kB)              */
/* call with cache disabled                                                  */

int checkdcache (void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	u32 cacheon = rd_dc_cst () & IDC_ENABLED;

#ifdef CONFIG_IP860
	u32 k = memctl->memc_br1 & ~0x00007fff;	/* probe in flash memoryarea */
#else
	u32 k = memctl->memc_br0 & ~0x00007fff;	/* probe in flash memoryarea */
#endif
	u32 m;
	u32 lines = -1;

	wr_dc_cst (IDC_UNALL);
	wr_dc_cst (IDC_INVALL);
	wr_dc_cst (IDC_DISABLE);

	while (!((m = rd_dc_cst ()) & IDC_CERR2)) {
		wr_dc_adr (k);
		wr_dc_cst (IDC_LDLCK);
		lines++;
		k += 0x10;	/* the number of bytes in a cacheline */
	}

	wr_dc_cst (IDC_UNALL);
	wr_dc_cst (IDC_INVALL);

	if (cacheon)
		wr_dc_cst (IDC_ENABLE);
	else
		wr_dc_cst (IDC_DISABLE);

	return lines << 4;
};

/* ------------------------------------------------------------------------- */

void upmconfig (uint upm, uint * table, uint size)
{
	uint i;
	uint addr = 0;
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	for (i = 0; i < size; i++) {
		memctl->memc_mdr = table[i];	/* (16-15) */
		memctl->memc_mcr = addr | upm;	/* (16-16) */
		addr++;
	}
}

/* ------------------------------------------------------------------------- */

int do_reset (cmd_tbl_t * cmdtp, bd_t * bd, int flag, int argc,
			  char *argv[])
{
	ulong msr, addr;

	volatile immap_t *immap = (immap_t *) CFG_IMMR;

	immap->im_clkrst.car_plprcr |= PLPRCR_CSR;	/* Checkstop Reset enable */

	/* Interrupts and MMU off */
	__asm__ volatile ("mtspr    81, 0");
	__asm__ volatile ("mfmsr    %0":"=r" (msr));

	msr &= ~0x1030;
	__asm__ volatile ("mtmsr    %0"::"r" (msr));

	/*
	 * Trying to execute the next instruction at a non-existing address
	 * should cause a machine check, resulting in reset
	 */
#ifdef CFG_RESET_ADDRESS
	addr = CFG_RESET_ADDRESS;
#else
	/*
	 * note: when CFG_MONITOR_BASE points to a RAM address, CFG_MONITOR_BASE
	 * - sizeof (ulong) is usually a valid address. Better pick an address
	 * known to be invalid on your system and assign it to CFG_RESET_ADDRESS.
	 * "(ulong)-1" used to be a good choice for many systems...
	 */
	addr = CFG_MONITOR_BASE - sizeof (ulong);
#endif
	((void (*)(void)) addr) ();
	return 1;
}

/* ------------------------------------------------------------------------- */

/*
 * Get timebase clock frequency (like cpu_clk in Hz)
 *
 * See table 15-5 pp. 15-16, and SCCR[RTSEL] pp. 15-27.
 */
unsigned long get_tbclk (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	volatile immap_t *immr = (volatile immap_t *) CFG_IMMR;
	ulong oscclk, factor;

	if (immr->im_clkrst.car_sccr & SCCR_TBS) {
		return (gd->cpu_clk / 16);
	}

	factor = (((CFG_PLPRCR) & PLPRCR_MF_MSK) >> PLPRCR_MF_SHIFT) + 1;

	oscclk = gd->cpu_clk / factor;

	if ((immr->im_clkrst.car_sccr & SCCR_RTSEL) == 0 || factor > 2) {
		return (oscclk / 4);
	}
	return (oscclk / 16);
}

/* ------------------------------------------------------------------------- */

#if defined(CONFIG_WATCHDOG)
void watchdog_reset (void)
{
	int re_enable = disable_interrupts ();

	reset_8xx_watchdog ((immap_t *) CFG_IMMR);
	if (re_enable)
		enable_interrupts ();
}

void reset_8xx_watchdog (volatile immap_t * immr)
{
# if defined(CONFIG_LWMON)
	/*
	 * The LWMON board uses a MAX6301 Watchdog
	 * with the trigger pin connected to port PA.7
	 *
	 * (The old board version used a MAX706TESA Watchdog, which
	 * had to be handled exactly the same.)
	 */
# define WATCHDOG_BIT	0x0100
	immr->im_ioport.iop_papar &= ~(WATCHDOG_BIT);	/* GPIO     */
	immr->im_ioport.iop_padir |= WATCHDOG_BIT;	/* Output   */
	immr->im_ioport.iop_paodr &= ~(WATCHDOG_BIT);	/* active output */

	immr->im_ioport.iop_padat ^= WATCHDOG_BIT;	/* Toggle WDI   */
# else
	/*
	 * All other boards use the MPC8xx Internal Watchdog
	 */
	immr->im_siu_conf.sc_swsr = 0x556c;	/* write magic1 */
	immr->im_siu_conf.sc_swsr = 0xaa39;	/* write magic2 */
# endif /* CONFIG_LWMON */
}

#endif /* CONFIG_WATCHDOG */

/* ------------------------------------------------------------------------- */

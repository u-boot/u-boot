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

#include <common.h>
#include <mpc8260.h>
#include <asm/cpm_8260.h>
#include <ioports.h>

static void config_8260_ioports (volatile immap_t * immr)
{
	int portnum;

	for (portnum = 0; portnum < 4; portnum++) {
		uint pmsk = 0,
		     ppar = 0,
		     psor = 0,
		     pdir = 0,
		     podr = 0,
		     pdat = 0;
		iop_conf_t *iopc = (iop_conf_t *) & iop_conf_tab[portnum][0];
		iop_conf_t *eiopc = iopc + 32;
		uint msk = 1;

		/*
		 * NOTE:
		 * index 0 refers to pin 31,
		 * index 31 refers to pin 0
		 */
		while (iopc < eiopc) {
			if (iopc->conf) {
				pmsk |= msk;
				if (iopc->ppar)
					ppar |= msk;
				if (iopc->psor)
					psor |= msk;
				if (iopc->pdir)
					pdir |= msk;
				if (iopc->podr)
					podr |= msk;
				if (iopc->pdat)
					pdat |= msk;
			}

			msk <<= 1;
			iopc++;
		}

		if (pmsk != 0) {
			volatile ioport_t *iop = ioport_addr (immr, portnum);
			uint tpmsk = ~pmsk;

			/*
			 * the (somewhat confused) paragraph at the
			 * bottom of page 35-5 warns that there might
			 * be "unknown behaviour" when programming
			 * PSORx and PDIRx, if PPARx = 1, so I
			 * decided this meant I had to disable the
			 * dedicated function first, and enable it
			 * last.
			 */
			iop->ppar &= tpmsk;
			iop->psor = (iop->psor & tpmsk) | psor;
			iop->podr = (iop->podr & tpmsk) | podr;
			iop->pdat = (iop->pdat & tpmsk) | pdat;
			iop->pdir = (iop->pdir & tpmsk) | pdir;
			iop->ppar |= ppar;
		}
	}
}

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f (volatile immap_t * immr)
{
	DECLARE_GLOBAL_DATA_PTR;
#if !defined(CONFIG_COGENT)		/* done in start.S for the cogent */
	uint sccr;
#endif
	volatile memctl8260_t *memctl = &immr->im_memctl;
	extern void m8260_cpm_reset (void);

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CFG_INIT_RAM_ADDR + CFG_GBL_DATA_OFFSET);

	/* Clear initial global data */
	memset ((void *) gd, 0, sizeof (gd_t));

	/* RSR - Reset Status Register - clear all status (5-4) */
	gd->reset_status = immr->im_clkrst.car_rsr;
	immr->im_clkrst.car_rsr = RSR_ALLBITS;

	/* RMR - Reset Mode Register - contains checkstop reset enable (5-5) */
	immr->im_clkrst.car_rmr = CFG_RMR;

	/* BCR - Bus Configuration Register (4-25) */
	immr->im_siu_conf.sc_bcr = CFG_BCR;

	/* SIUMCR - contains debug pin configuration (4-31) */
	immr->im_siu_conf.sc_siumcr = CFG_SIUMCR;

	config_8260_ioports (immr);

	/* initialize time counter status and control register (4-40) */
	immr->im_sit.sit_tmcntsc = CFG_TMCNTSC;

	/* initialize the PIT (4-42) */
	immr->im_sit.sit_piscr = CFG_PISCR;

#if !defined(CONFIG_COGENT)		/* done in start.S for the cogent */
	/* System clock control register (9-8) */
	sccr = immr->im_clkrst.car_sccr &
		(SCCR_PCI_MODE | SCCR_PCI_MODCK | SCCR_PCIDF_MSK);
	immr->im_clkrst.car_sccr = sccr |
		(CFG_SCCR & ~(SCCR_PCI_MODE | SCCR_PCI_MODCK | SCCR_PCIDF_MSK) );
#endif /* !CONFIG_COGENT */

	/*
	 * Memory Controller:
	 */

	/* Map banks 0 and 1 to the FLASH banks 0 and 1 at preliminary
	 * addresses - these have to be modified later when FLASH size
	 * has been determined
	 */

#if defined(CFG_OR0_REMAP)
	memctl->memc_or0 = CFG_OR0_REMAP;
#endif
#if defined(CFG_OR1_REMAP)
	memctl->memc_or1 = CFG_OR1_REMAP;
#endif

	/* now restrict to preliminary range */
	memctl->memc_br0 = CFG_BR0_PRELIM;
	memctl->memc_or0 = CFG_OR0_PRELIM;

#if defined(CFG_BR1_PRELIM) && defined(CFG_OR1_PRELIM)
	memctl->memc_or1 = CFG_OR1_PRELIM;
	memctl->memc_br1 = CFG_BR1_PRELIM;
#endif

#if defined(CFG_BR2_PRELIM) && defined(CFG_OR2_PRELIM)
	memctl->memc_or2 = CFG_OR2_PRELIM;
	memctl->memc_br2 = CFG_BR2_PRELIM;
#endif

#if defined(CFG_BR3_PRELIM) && defined(CFG_OR3_PRELIM)
	memctl->memc_or3 = CFG_OR3_PRELIM;
	memctl->memc_br3 = CFG_BR3_PRELIM;
#endif

#if defined(CFG_BR4_PRELIM) && defined(CFG_OR4_PRELIM)
	memctl->memc_or4 = CFG_OR4_PRELIM;
	memctl->memc_br4 = CFG_BR4_PRELIM;
#endif

#if defined(CFG_BR5_PRELIM) && defined(CFG_OR5_PRELIM)
	memctl->memc_or5 = CFG_OR5_PRELIM;
	memctl->memc_br5 = CFG_BR5_PRELIM;
#endif

#if defined(CFG_BR6_PRELIM) && defined(CFG_OR6_PRELIM)
	memctl->memc_or6 = CFG_OR6_PRELIM;
	memctl->memc_br6 = CFG_BR6_PRELIM;
#endif

#if defined(CFG_BR7_PRELIM) && defined(CFG_OR7_PRELIM)
	memctl->memc_or7 = CFG_OR7_PRELIM;
	memctl->memc_br7 = CFG_BR7_PRELIM;
#endif

#if defined(CFG_BR8_PRELIM) && defined(CFG_OR8_PRELIM)
	memctl->memc_or8 = CFG_OR8_PRELIM;
	memctl->memc_br8 = CFG_BR8_PRELIM;
#endif

#if defined(CFG_BR9_PRELIM) && defined(CFG_OR9_PRELIM)
	memctl->memc_or9 = CFG_OR9_PRELIM;
	memctl->memc_br9 = CFG_BR9_PRELIM;
#endif

#if defined(CFG_BR10_PRELIM) && defined(CFG_OR10_PRELIM)
	memctl->memc_or10 = CFG_OR10_PRELIM;
	memctl->memc_br10 = CFG_BR10_PRELIM;
#endif

#if defined(CFG_BR11_PRELIM) && defined(CFG_OR11_PRELIM)
	memctl->memc_or11 = CFG_OR11_PRELIM;
	memctl->memc_br11 = CFG_BR11_PRELIM;
#endif

	m8260_cpm_reset ();
}

/*
 * initialize higher level parts of CPU like time base and timers
 */
int cpu_init_r (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	volatile immap_t *immr = (immap_t *) gd->bd->bi_immr_base;

	immr->im_cpm.cp_rccr = CFG_RCCR;

	return (0);
}

/*
 * print out the reason for the reset
 */
int prt_8260_rsr (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	static struct {
		ulong mask;
		char *desc;
	} bits[] = {
		{
		RSR_JTRS, "JTAG"}, {
		RSR_CSRS, "Check Stop"}, {
		RSR_SWRS, "Software Watchdog"}, {
		RSR_BMRS, "Bus Monitor"}, {
		RSR_ESRS, "External Soft"}, {
		RSR_EHRS, "External Hard"}
	};
	static int n = sizeof bits / sizeof bits[0];
	ulong rsr = gd->reset_status;
	int i;
	char *sep;

	puts (CPU_ID_STR " Reset Status:");

	sep = " ";
	for (i = 0; i < n; i++)
		if (rsr & bits[i].mask) {
			printf ("%s%s", sep, bits[i].desc);
			sep = ", ";
		}

	puts ("\n\n");
	return (0);
}

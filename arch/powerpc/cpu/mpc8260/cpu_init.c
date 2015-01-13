/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc8260.h>
#include <asm/cpm_8260.h>
#include <ioports.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_BOARD_GET_CPU_CLK_F)
extern unsigned long board_get_cpu_clk_f (void);
#endif

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

#define SET_VAL_MASK(a, b, mask) ((a & mask) | (b & ~mask))
/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f (volatile immap_t * immr)
{
	uint sccr;
#if defined(CONFIG_BOARD_GET_CPU_CLK_F)
	unsigned long cpu_clk;
#endif
	volatile memctl8260_t *memctl = &immr->im_memctl;
	extern void m8260_cpm_reset (void);

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET);

	/* Clear initial global data */
	memset ((void *) gd, 0, sizeof (gd_t));

	/* RSR - Reset Status Register - clear all status (5-4) */
	gd->arch.reset_status = immr->im_clkrst.car_rsr;
	immr->im_clkrst.car_rsr = RSR_ALLBITS;

	/* RMR - Reset Mode Register - contains checkstop reset enable (5-5) */
	immr->im_clkrst.car_rmr = CONFIG_SYS_RMR;

	/* BCR - Bus Configuration Register (4-25) */
#if defined(CONFIG_SYS_BCR_60x) && (CONFIG_SYS_BCR_SINGLE)
	if (immr->im_siu_conf.sc_bcr & BCR_EBM) {
		immr->im_siu_conf.sc_bcr = SET_VAL_MASK(immr->im_siu_conf.sc_bcr, CONFIG_SYS_BCR_60x, 0x80000010);
	} else {
		immr->im_siu_conf.sc_bcr = SET_VAL_MASK(immr->im_siu_conf.sc_bcr, CONFIG_SYS_BCR_SINGLE, 0x80000010);
	}
#else
	immr->im_siu_conf.sc_bcr = CONFIG_SYS_BCR;
#endif

	/* SIUMCR - contains debug pin configuration (4-31) */
#if defined(CONFIG_SYS_SIUMCR_LOW) && (CONFIG_SYS_SIUMCR_HIGH)
	cpu_clk = board_get_cpu_clk_f ();
	if (cpu_clk >= 100000000) {
		immr->im_siu_conf.sc_siumcr = SET_VAL_MASK(immr->im_siu_conf.sc_siumcr, CONFIG_SYS_SIUMCR_HIGH, 0x9f3cc000);
	} else {
		immr->im_siu_conf.sc_siumcr = SET_VAL_MASK(immr->im_siu_conf.sc_siumcr, CONFIG_SYS_SIUMCR_LOW, 0x9f3cc000);
	}
#else
	immr->im_siu_conf.sc_siumcr = CONFIG_SYS_SIUMCR;
#endif

	config_8260_ioports (immr);

	/* initialize time counter status and control register (4-40) */
	immr->im_sit.sit_tmcntsc = CONFIG_SYS_TMCNTSC;

	/* initialize the PIT (4-42) */
	immr->im_sit.sit_piscr = CONFIG_SYS_PISCR;

	/* System clock control register (9-8) */
	sccr = immr->im_clkrst.car_sccr &
		(SCCR_PCI_MODE | SCCR_PCI_MODCK | SCCR_PCIDF_MSK);
	immr->im_clkrst.car_sccr = sccr |
		(CONFIG_SYS_SCCR & ~(SCCR_PCI_MODE | SCCR_PCI_MODCK | SCCR_PCIDF_MSK) );

	/*
	 * Memory Controller:
	 */

	/* Map banks 0 and 1 to the FLASH banks 0 and 1 at preliminary
	 * addresses - these have to be modified later when FLASH size
	 * has been determined
	 */

#if defined(CONFIG_SYS_OR0_REMAP)
	memctl->memc_or0 = CONFIG_SYS_OR0_REMAP;
#endif
#if defined(CONFIG_SYS_OR1_REMAP)
	memctl->memc_or1 = CONFIG_SYS_OR1_REMAP;
#endif

	/* now restrict to preliminary range */
	/* the PS came from the HRCW, don't change it */
	memctl->memc_br0 = SET_VAL_MASK(memctl->memc_br0 , CONFIG_SYS_BR0_PRELIM, BRx_PS_MSK);
	memctl->memc_or0 = CONFIG_SYS_OR0_PRELIM;

#if defined(CONFIG_SYS_BR1_PRELIM) && defined(CONFIG_SYS_OR1_PRELIM)
	memctl->memc_or1 = CONFIG_SYS_OR1_PRELIM;
	memctl->memc_br1 = CONFIG_SYS_BR1_PRELIM;
#endif

#if defined(CONFIG_SYS_BR2_PRELIM) && defined(CONFIG_SYS_OR2_PRELIM)
	memctl->memc_or2 = CONFIG_SYS_OR2_PRELIM;
	memctl->memc_br2 = CONFIG_SYS_BR2_PRELIM;
#endif

#if defined(CONFIG_SYS_BR3_PRELIM) && defined(CONFIG_SYS_OR3_PRELIM)
	memctl->memc_or3 = CONFIG_SYS_OR3_PRELIM;
	memctl->memc_br3 = CONFIG_SYS_BR3_PRELIM;
#endif

#if defined(CONFIG_SYS_BR4_PRELIM) && defined(CONFIG_SYS_OR4_PRELIM)
	memctl->memc_or4 = CONFIG_SYS_OR4_PRELIM;
	memctl->memc_br4 = CONFIG_SYS_BR4_PRELIM;
#endif

#if defined(CONFIG_SYS_BR5_PRELIM) && defined(CONFIG_SYS_OR5_PRELIM)
	memctl->memc_or5 = CONFIG_SYS_OR5_PRELIM;
	memctl->memc_br5 = CONFIG_SYS_BR5_PRELIM;
#endif

#if defined(CONFIG_SYS_BR6_PRELIM) && defined(CONFIG_SYS_OR6_PRELIM)
	memctl->memc_or6 = CONFIG_SYS_OR6_PRELIM;
	memctl->memc_br6 = CONFIG_SYS_BR6_PRELIM;
#endif

#if defined(CONFIG_SYS_BR7_PRELIM) && defined(CONFIG_SYS_OR7_PRELIM)
	memctl->memc_or7 = CONFIG_SYS_OR7_PRELIM;
	memctl->memc_br7 = CONFIG_SYS_BR7_PRELIM;
#endif

#if defined(CONFIG_SYS_BR8_PRELIM) && defined(CONFIG_SYS_OR8_PRELIM)
	memctl->memc_or8 = CONFIG_SYS_OR8_PRELIM;
	memctl->memc_br8 = CONFIG_SYS_BR8_PRELIM;
#endif

#if defined(CONFIG_SYS_BR9_PRELIM) && defined(CONFIG_SYS_OR9_PRELIM)
	memctl->memc_or9 = CONFIG_SYS_OR9_PRELIM;
	memctl->memc_br9 = CONFIG_SYS_BR9_PRELIM;
#endif

#if defined(CONFIG_SYS_BR10_PRELIM) && defined(CONFIG_SYS_OR10_PRELIM)
	memctl->memc_or10 = CONFIG_SYS_OR10_PRELIM;
	memctl->memc_br10 = CONFIG_SYS_BR10_PRELIM;
#endif

#if defined(CONFIG_SYS_BR11_PRELIM) && defined(CONFIG_SYS_OR11_PRELIM)
	memctl->memc_or11 = CONFIG_SYS_OR11_PRELIM;
	memctl->memc_br11 = CONFIG_SYS_BR11_PRELIM;
#endif

	m8260_cpm_reset ();
}

/*
 * initialize higher level parts of CPU like time base and timers
 */
int cpu_init_r (void)
{
	volatile immap_t *immr = (immap_t *) gd->bd->bi_immr_base;

	immr->im_cpm.cp_rccr = CONFIG_SYS_RCCR;

	return (0);
}

/*
 * print out the reason for the reset
 */
int prt_8260_rsr (void)
{
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
	ulong rsr = gd->arch.reset_status;
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

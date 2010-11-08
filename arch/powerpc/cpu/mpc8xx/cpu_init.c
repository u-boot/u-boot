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
#include <watchdog.h>

#include <mpc8xx.h>
#include <commproc.h>

#if defined(CONFIG_SYS_RTCSC) || defined(CONFIG_SYS_RMDS)
DECLARE_GLOBAL_DATA_PTR;
#endif

#if defined(CONFIG_SYS_I2C_UCODE_PATCH) || defined(CONFIG_SYS_SPI_UCODE_PATCH) || \
    defined(CONFIG_SYS_SMC_UCODE_PATCH)
void cpm_load_patch (volatile immap_t * immr);
#endif

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f (volatile immap_t * immr)
{
#ifndef CONFIG_MBX
	volatile memctl8xx_t *memctl = &immr->im_memctl;
# ifdef CONFIG_SYS_PLPRCR
	ulong mfmask;
# endif
#endif
	ulong reg;

	/* SYPCR - contains watchdog control (11-9) */

	immr->im_siu_conf.sc_sypcr = CONFIG_SYS_SYPCR;

#if defined(CONFIG_WATCHDOG)
	reset_8xx_watchdog (immr);
#endif /* CONFIG_WATCHDOG */

	/* SIUMCR - contains debug pin configuration (11-6) */
#ifndef CONFIG_SVM_SC8xx
	immr->im_siu_conf.sc_siumcr |= CONFIG_SYS_SIUMCR;
#else
	immr->im_siu_conf.sc_siumcr = CONFIG_SYS_SIUMCR;
#endif
	/* initialize timebase status and control register (11-26) */
	/* unlock TBSCRK */

	immr->im_sitk.sitk_tbscrk = KAPWR_KEY;
	immr->im_sit.sit_tbscr = CONFIG_SYS_TBSCR;

	/* initialize the PIT (11-31) */

	immr->im_sitk.sitk_piscrk = KAPWR_KEY;
	immr->im_sit.sit_piscr = CONFIG_SYS_PISCR;

	/* System integration timers. Don't change EBDF! (15-27) */

	immr->im_clkrstk.cark_sccrk = KAPWR_KEY;
	reg = immr->im_clkrst.car_sccr;
	reg &= SCCR_MASK;
	reg |= CONFIG_SYS_SCCR;
	immr->im_clkrst.car_sccr = reg;

	/* PLL (CPU clock) settings (15-30) */

	immr->im_clkrstk.cark_plprcrk = KAPWR_KEY;

#ifndef CONFIG_MBX		/* MBX board does things different */

	/* If CONFIG_SYS_PLPRCR (set in the various *_config.h files) tries to
	 * set the MF field, then just copy CONFIG_SYS_PLPRCR over car_plprcr,
	 * otherwise OR in CONFIG_SYS_PLPRCR so we do not change the current MF
	 * field value.
	 *
	 * For newer (starting MPC866) chips PLPRCR layout is different.
	 */
#ifdef CONFIG_SYS_PLPRCR
	if (get_immr(0xFFFF) >= MPC8xx_NEW_CLK)
	   mfmask = PLPRCR_MFACT_MSK;
	else
	   mfmask = PLPRCR_MF_MSK;

	if ((CONFIG_SYS_PLPRCR & mfmask) != 0)
	   reg = CONFIG_SYS_PLPRCR;			/* reset control bits   */
	else {
	   reg = immr->im_clkrst.car_plprcr;
	   reg &= mfmask;			/* isolate MF-related fields */
	   reg |= CONFIG_SYS_PLPRCR;			/* reset control bits   */
	}
	immr->im_clkrst.car_plprcr = reg;
#endif

	/*
	 * Memory Controller:
	 */

	/* perform BR0 reset that MPC850 Rev. A can't guarantee */
	reg = memctl->memc_br0;
	reg &= BR_PS_MSK;	/* Clear everything except Port Size bits */
	reg |= BR_V;		/* then add just the "Bank Valid" bit     */
	memctl->memc_br0 = reg;

	/* Map banks 0 (and maybe 1) to the FLASH banks 0 (and 1) at
	 * preliminary addresses - these have to be modified later
	 * when FLASH size has been determined
	 *
	 * Depending on the size of the memory region defined by
	 * CONFIG_SYS_OR0_REMAP some boards (wide address mask) allow to map the
	 * CONFIG_SYS_MONITOR_BASE, while others (narrower address mask) can't
	 * map CONFIG_SYS_MONITOR_BASE.
	 *
	 * For example, for CONFIG_IVMS8, the CONFIG_SYS_MONITOR_BASE is
	 * 0xff000000, but CONFIG_SYS_OR0_REMAP's address mask is 0xfff80000.
	 *
	 * If BR0 wasn't loaded with address base 0xff000000, then BR0's
	 * base address remains as 0x00000000. However, the address mask
	 * have been narrowed to 512Kb, so CONFIG_SYS_MONITOR_BASE wasn't mapped
	 * into the Bank0.
	 *
	 * This is why CONFIG_IVMS8 and similar boards must load BR0 with
	 * CONFIG_SYS_BR0_PRELIM in advance.
	 *
	 * [Thanks to Michael Liao for this explanation.
	 *  I owe him a free beer. - wd]
	 */

#if defined(CONFIG_HERMES)	|| \
    defined(CONFIG_ICU862)	|| \
    defined(CONFIG_IP860)	|| \
    defined(CONFIG_IVML24)	|| \
    defined(CONFIG_IVMS8)	|| \
    defined(CONFIG_LWMON)	|| \
    defined(CONFIG_MHPC)	|| \
    defined(CONFIG_PCU_E)	|| \
    defined(CONFIG_R360MPI)	|| \
    defined(CONFIG_RMU)		|| \
    defined(CONFIG_RPXCLASSIC)	|| \
    defined(CONFIG_RPXLITE)	|| \
    defined(CONFIG_SPC1920)	|| \
    defined(CONFIG_SPD823TS)

	memctl->memc_br0 = CONFIG_SYS_BR0_PRELIM;
#endif

#if defined(CONFIG_SYS_OR0_REMAP)
	memctl->memc_or0 = CONFIG_SYS_OR0_REMAP;
#endif
#if defined(CONFIG_SYS_OR1_REMAP)
	memctl->memc_or1 = CONFIG_SYS_OR1_REMAP;
#endif
#if defined(CONFIG_SYS_OR5_REMAP)
	memctl->memc_or5 = CONFIG_SYS_OR5_REMAP;
#endif

	/* now restrict to preliminary range */
	memctl->memc_br0 = CONFIG_SYS_BR0_PRELIM;
	memctl->memc_or0 = CONFIG_SYS_OR0_PRELIM;

#if (defined(CONFIG_SYS_OR1_PRELIM) && defined(CONFIG_SYS_BR1_PRELIM))
	memctl->memc_or1 = CONFIG_SYS_OR1_PRELIM;
	memctl->memc_br1 = CONFIG_SYS_BR1_PRELIM;
#endif

#if defined(CONFIG_IP860) /* disable CS0 now that Flash is mapped on CS1 */
	memctl->memc_br0 = 0;
#endif

#if defined(CONFIG_SYS_OR2_PRELIM) && defined(CONFIG_SYS_BR2_PRELIM)
	memctl->memc_or2 = CONFIG_SYS_OR2_PRELIM;
	memctl->memc_br2 = CONFIG_SYS_BR2_PRELIM;
#endif

#if defined(CONFIG_SYS_OR3_PRELIM) && defined(CONFIG_SYS_BR3_PRELIM)
	memctl->memc_or3 = CONFIG_SYS_OR3_PRELIM;
	memctl->memc_br3 = CONFIG_SYS_BR3_PRELIM;
#endif

#if defined(CONFIG_SYS_OR4_PRELIM) && defined(CONFIG_SYS_BR4_PRELIM)
	memctl->memc_or4 = CONFIG_SYS_OR4_PRELIM;
	memctl->memc_br4 = CONFIG_SYS_BR4_PRELIM;
#endif

#if defined(CONFIG_SYS_OR5_PRELIM) && defined(CONFIG_SYS_BR5_PRELIM)
	memctl->memc_or5 = CONFIG_SYS_OR5_PRELIM;
	memctl->memc_br5 = CONFIG_SYS_BR5_PRELIM;
#endif

#if defined(CONFIG_SYS_OR6_PRELIM) && defined(CONFIG_SYS_BR6_PRELIM)
	memctl->memc_or6 = CONFIG_SYS_OR6_PRELIM;
	memctl->memc_br6 = CONFIG_SYS_BR6_PRELIM;
#endif

#if defined(CONFIG_SYS_OR7_PRELIM) && defined(CONFIG_SYS_BR7_PRELIM)
	memctl->memc_or7 = CONFIG_SYS_OR7_PRELIM;
	memctl->memc_br7 = CONFIG_SYS_BR7_PRELIM;
#endif

#endif /* ! CONFIG_MBX */

	/*
	 * Reset CPM
	 */
	immr->im_cpm.cp_cpcr = CPM_CR_RST | CPM_CR_FLG;
	do {			/* Spin until command processed     */
		__asm__ ("eieio");
	} while (immr->im_cpm.cp_cpcr & CPM_CR_FLG);

#ifdef CONFIG_MBX
	/*
	 * on the MBX, things are a little bit different:
	 * - we need to read the VPD to get board information
	 * - the plprcr is set up dynamically
	 * - the memory controller is set up dynamically
	 */
	mbx_init ();
#endif /* CONFIG_MBX */

#ifdef CONFIG_RPXCLASSIC
	rpxclassic_init ();
#endif

#if defined(CONFIG_RPXLITE) && defined(CONFIG_ENV_IS_IN_NVRAM)
	rpxlite_init ();
#endif

#ifdef CONFIG_SYS_RCCR			/* must be done before cpm_load_patch() */
	/* write config value */
	immr->im_cpm.cp_rccr = CONFIG_SYS_RCCR;
#endif

#if defined(CONFIG_SYS_I2C_UCODE_PATCH) || defined(CONFIG_SYS_SPI_UCODE_PATCH) || \
    defined(CONFIG_SYS_SMC_UCODE_PATCH)
	cpm_load_patch (immr);	/* load mpc8xx  microcode patch */
#endif
}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r (void)
{
#if defined(CONFIG_SYS_RTCSC) || defined(CONFIG_SYS_RMDS)
	bd_t *bd = gd->bd;
	volatile immap_t *immr = (volatile immap_t *) (bd->bi_immr_base);
#endif

#ifdef CONFIG_SYS_RTCSC
	/* Unlock RTSC register */
	immr->im_sitk.sitk_rtcsck = KAPWR_KEY;
	/* write config value */
	immr->im_sit.sit_rtcsc = CONFIG_SYS_RTCSC;
#endif

#ifdef CONFIG_SYS_RMDS
	/* write config value */
	immr->im_cpm.cp_rmds = CONFIG_SYS_RMDS;
#endif
	return (0);
}

/*
 * Copyright 2007 Freescale Semiconductor.
 *
 * (C) Copyright 2003 Motorola Inc.
 * Modified by Xianghua Xiao, X.Xiao@motorola.com
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
#include <asm/processor.h>
#include <ioports.h>
#include <asm/io.h>
#include <asm/mmu.h>
#include <asm/fsl_law.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_QE
extern qe_iop_conf_t qe_iop_conf_tab[];
extern void qe_config_iopin(u8 port, u8 pin, int dir,
				int open_drain, int assign);
extern void qe_init(uint qe_base);
extern void qe_reset(void);

static void config_qe_ioports(void)
{
	u8      port, pin;
	int     dir, open_drain, assign;
	int     i;

	for (i = 0; qe_iop_conf_tab[i].assign != QE_IOP_TAB_END; i++) {
		port		= qe_iop_conf_tab[i].port;
		pin		= qe_iop_conf_tab[i].pin;
		dir		= qe_iop_conf_tab[i].dir;
		open_drain	= qe_iop_conf_tab[i].open_drain;
		assign		= qe_iop_conf_tab[i].assign;
		qe_config_iopin(port, pin, dir, open_drain, assign);
	}
}
#endif

#ifdef CONFIG_CPM2
void config_8560_ioports (volatile ccsr_cpm_t * cpm)
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
			volatile ioport_t *iop = ioport_addr (cpm, portnum);
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
#endif

/* We run cpu_init_early_f in AS = 1 */
void cpu_init_early_f(void)
{
	set_tlb(0, CFG_CCSRBAR, CFG_CCSRBAR,
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		1, 0, BOOKE_PAGESZ_4K, 0);

	/* set up CCSR if we want it moved */
#if (CFG_CCSRBAR_DEFAULT != CFG_CCSRBAR)
	{
		u32 temp;

		set_tlb(0, CFG_CCSRBAR_DEFAULT, CFG_CCSRBAR_DEFAULT,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			1, 1, BOOKE_PAGESZ_4K, 0);

		temp = in_be32((volatile u32 *)CFG_CCSRBAR_DEFAULT);
		out_be32((volatile u32 *)CFG_CCSRBAR_DEFAULT, CFG_CCSRBAR >> 12);

		temp = in_be32((volatile u32 *)CFG_CCSRBAR);
	}
#endif

	init_laws();
	invalidate_tlb(0);
#ifdef CONFIG_FSL_INIT_TLBS
	init_tlbs();
#else
	{
		extern u32 tlb1_entry;
		u32 *tmp = &tlb1_entry;
		int i;
		int num = tmp[2];

		/* skip to actual table */
		tmp += 3;

		for (i = 0; i < num; i++, tmp += 4) {
			mtspr(MAS0, tmp[0]);
			mtspr(MAS1, tmp[1]);
			mtspr(MAS2, tmp[2]);
			mtspr(MAS3, tmp[3]);
			asm volatile("isync;msync;tlbwe;isync");
		}
	}
#endif
}

/*
 * Breathe some life into the CPU...
 *
 * Set up the memory map
 * initialize a bunch of registers
 */

void cpu_init_f (void)
{
	volatile ccsr_lbc_t *memctl = (void *)(CFG_MPC85xx_LBC_ADDR);
	extern void m8560_cpm_reset (void);

	disable_tlb(14);
	disable_tlb(15);

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CFG_INIT_RAM_ADDR + CFG_GBL_DATA_OFFSET);

	/* Clear initial global data */
	memset ((void *) gd, 0, sizeof (gd_t));

#ifdef CONFIG_CPM2
	config_8560_ioports((ccsr_cpm_t *)CFG_MPC85xx_CPM_ADDR);
#endif

	/* Map banks 0 and 1 to the FLASH banks 0 and 1 at preliminary
	 * addresses - these have to be modified later when FLASH size
	 * has been determined
	 */
#if defined(CFG_OR0_REMAP)
	memctl->or0 = CFG_OR0_REMAP;
#endif
#if defined(CFG_OR1_REMAP)
	memctl->or1 = CFG_OR1_REMAP;
#endif

	/* now restrict to preliminary range */
	/* if cs1 is already set via debugger, leave cs0/cs1 alone */
	if (! memctl->br1 & 1) {
#if defined(CFG_BR0_PRELIM) && defined(CFG_OR0_PRELIM)
		memctl->br0 = CFG_BR0_PRELIM;
		memctl->or0 = CFG_OR0_PRELIM;
#endif

#if defined(CFG_BR1_PRELIM) && defined(CFG_OR1_PRELIM)
		memctl->or1 = CFG_OR1_PRELIM;
		memctl->br1 = CFG_BR1_PRELIM;
#endif
	}

#if defined(CFG_BR2_PRELIM) && defined(CFG_OR2_PRELIM)
	memctl->or2 = CFG_OR2_PRELIM;
	memctl->br2 = CFG_BR2_PRELIM;
#endif

#if defined(CFG_BR3_PRELIM) && defined(CFG_OR3_PRELIM)
	memctl->or3 = CFG_OR3_PRELIM;
	memctl->br3 = CFG_BR3_PRELIM;
#endif

#if defined(CFG_BR4_PRELIM) && defined(CFG_OR4_PRELIM)
	memctl->or4 = CFG_OR4_PRELIM;
	memctl->br4 = CFG_BR4_PRELIM;
#endif

#if defined(CFG_BR5_PRELIM) && defined(CFG_OR5_PRELIM)
	memctl->or5 = CFG_OR5_PRELIM;
	memctl->br5 = CFG_BR5_PRELIM;
#endif

#if defined(CFG_BR6_PRELIM) && defined(CFG_OR6_PRELIM)
	memctl->or6 = CFG_OR6_PRELIM;
	memctl->br6 = CFG_BR6_PRELIM;
#endif

#if defined(CFG_BR7_PRELIM) && defined(CFG_OR7_PRELIM)
	memctl->or7 = CFG_OR7_PRELIM;
	memctl->br7 = CFG_BR7_PRELIM;
#endif

#if defined(CONFIG_CPM2)
	m8560_cpm_reset();
#endif
#ifdef CONFIG_QE
	/* Config QE ioports */
	config_qe_ioports();
#endif

}


/*
 * Initialize L2 as cache.
 *
 * The newer 8548, etc, parts have twice as much cache, but
 * use the same bit-encoding as the older 8555, etc, parts.
 *
 */

int cpu_init_r(void)
{
#ifdef CONFIG_CLEAR_LAW0
#ifdef CONFIG_FSL_LAW
	disable_law(0);
#else
	volatile ccsr_local_ecm_t *ecm = (void *)(CFG_MPC85xx_ECM_ADDR);

	/* clear alternate boot location LAW (used for sdram, or ddr bank) */
	ecm->lawar0 = 0;
#endif
#endif

#if defined(CONFIG_L2_CACHE)
	volatile ccsr_l2cache_t *l2cache = (void *)CFG_MPC85xx_L2_ADDR;
	volatile uint cache_ctl;
	uint svr, ver;
	uint l2srbar;

	svr = get_svr();
	ver = SVR_VER(svr);

	asm("msync;isync");
	cache_ctl = l2cache->l2ctl;

	switch (cache_ctl & 0x30000000) {
	case 0x20000000:
		if (ver == SVR_8548 || ver == SVR_8548_E ||
		    ver == SVR_8544 || ver == SVR_8568_E) {
			printf ("L2 cache 512KB:");
			/* set L2E=1, L2I=1, & L2SRAM=0 */
			cache_ctl = 0xc0000000;
		} else {
			printf ("L2 cache 256KB:");
			/* set L2E=1, L2I=1, & L2BLKSZ=2 (256 Kbyte) */
			cache_ctl = 0xc8000000;
		}
		break;
	case 0x10000000:
		printf ("L2 cache 256KB:");
		if (ver == SVR_8544 || ver == SVR_8544_E) {
			cache_ctl = 0xc0000000; /* set L2E=1, L2I=1, & L2SRAM=0 */
		}
		break;
	case 0x30000000:
	case 0x00000000:
	default:
		printf ("L2 cache unknown size (0x%08x)\n", cache_ctl);
		return -1;
	}

	if (l2cache->l2ctl & 0x80000000) {
		printf(" already enabled.");
		l2srbar = l2cache->l2srbar0;
#ifdef CFG_INIT_L2_ADDR
		if (l2cache->l2ctl & 0x00010000 && l2srbar >= CFG_FLASH_BASE) {
			l2srbar = CFG_INIT_L2_ADDR;
			l2cache->l2srbar0 = l2srbar;
			printf("  Moving to 0x%08x", CFG_INIT_L2_ADDR);
		}
#endif /* CFG_INIT_L2_ADDR */
		puts("\n");
	} else {
		asm("msync;isync");
		l2cache->l2ctl = cache_ctl; /* invalidate & enable */
		asm("msync;isync");
		printf(" enabled\n");
	}
#else
	printf("L2 cache: disabled\n");
#endif
#ifdef CONFIG_QE
	uint qe_base = CFG_IMMR + 0x00080000; /* QE immr base */
	qe_init(qe_base);
	qe_reset();
#endif

	return 0;
}

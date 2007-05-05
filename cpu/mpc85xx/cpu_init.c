/*
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

DECLARE_GLOBAL_DATA_PTR;


#ifdef CONFIG_CPM2
static void config_8560_ioports (volatile immap_t * immr)
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
#endif

/*
 * Breathe some life into the CPU...
 *
 * Set up the memory map
 * initialize a bunch of registers
 */

void cpu_init_f (void)
{
	volatile immap_t    *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_lbc_t *memctl = &immap->im_lbc;
	extern void m8560_cpm_reset (void);

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CFG_INIT_RAM_ADDR + CFG_GBL_DATA_OFFSET);

	/* Clear initial global data */
	memset ((void *) gd, 0, sizeof (gd_t));


#ifdef CONFIG_CPM2
	config_8560_ioports(immap);
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
#if defined(CFG_BR0_PRELIM) && defined(CFG_OR0_PRELIM)
	memctl->br0 = CFG_BR0_PRELIM;
	memctl->or0 = CFG_OR0_PRELIM;
#endif

#if defined(CFG_BR1_PRELIM) && defined(CFG_OR1_PRELIM)
	memctl->or1 = CFG_OR1_PRELIM;
	memctl->br1 = CFG_BR1_PRELIM;
#endif

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
}


/*
 * Initialize L2 as cache.
 *
 * The newer 8548, etc, parts have twice as much cache, but
 * use the same bit-encoding as the older 8555, etc, parts.
 *
 * FIXME: Use PVR_VER(pvr) == 1 test here instead of SVR_VER()?
 */

int cpu_init_r(void)
{
#if defined(CONFIG_L2_CACHE)
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_l2cache_t *l2cache = &immap->im_l2cache;
	volatile uint cache_ctl;
	uint svr, ver;

	svr = get_svr();
	ver = SVR_VER(svr);

	asm("msync;isync");
	cache_ctl = l2cache->l2ctl;

	switch (cache_ctl & 0x30000000) {
	case 0x20000000:
		if (ver == SVR_8548 || ver == SVR_8548_E) {
			printf ("L2 cache 512KB:");
		} else {
			printf ("L2 cache 256KB:");
		}
		break;
	case 0x00000000:
	case 0x10000000:
	case 0x30000000:
	default:
		printf ("L2 cache unknown size (0x%08x)\n", cache_ctl);
		return -1;
	}

	asm("msync;isync");
	l2cache->l2ctl = 0x68000000; /* invalidate */
	cache_ctl = l2cache->l2ctl;
	asm("msync;isync");

	l2cache->l2ctl = 0xa8000000; /* enable 256KB L2 cache */
	cache_ctl = l2cache->l2ctl;
	asm("msync;isync");

	printf(" enabled\n");
#else
	printf("L2 cache: disabled\n");
#endif

	return 0;
}

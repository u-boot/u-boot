/*
 * Copyright (C) 2004-2006 Freescale Semiconductor, Inc.
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
#include <mpc83xx.h>
#include <ioports.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_QE
extern qe_iop_conf_t qe_iop_conf_tab[];
extern void qe_config_iopin(u8 port, u8 pin, int dir,
			 int open_drain, int assign);
extern void qe_init(uint qe_base);
extern void qe_reset(void);

static void config_qe_ioports(void)
{
	u8	port, pin;
	int	dir, open_drain, assign;
	int	i;

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

/*
 * Breathe some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f (volatile immap_t * im)
{
	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CFG_INIT_RAM_ADDR + CFG_GBL_DATA_OFFSET);

	/* Clear initial global data */
	memset ((void *) gd, 0, sizeof (gd_t));

	/* system performance tweaking */

#ifdef CFG_ACR_PIPE_DEP
	/* Arbiter pipeline depth */
	im->arbiter.acr = (im->arbiter.acr & ~ACR_PIPE_DEP) |
			  (CFG_ACR_PIPE_DEP << ACR_PIPE_DEP_SHIFT);
#endif

#ifdef CFG_SPCR_TSEC1EP
	/* TSEC1 Emergency priority */
	im->sysconf.spcr = (im->sysconf.spcr & ~SPCR_TSEC1EP) | (CFG_SPCR_TSEC1EP << SPCR_TSEC1EP_SHIFT);
#endif

#ifdef CFG_SPCR_TSEC2EP
	/* TSEC2 Emergency priority */
	im->sysconf.spcr = (im->sysconf.spcr & ~SPCR_TSEC2EP) | (CFG_SPCR_TSEC2EP << SPCR_TSEC2EP_SHIFT);
#endif

#ifdef CONFIG_MPC834X
#ifdef CFG_SCCR_TSEC1CM
	/* TSEC1 clock mode */
	im->clk.sccr = (im->clk.sccr & ~SCCR_TSEC1CM) | (CFG_SCCR_TSEC1CM << SCCR_TSEC1CM_SHIFT);
#endif
#ifdef CFG_SCCR_TSEC2CM
	/* TSEC2 & I2C1 clock mode */
	im->clk.sccr = (im->clk.sccr & ~SCCR_TSEC2CM) | (CFG_SCCR_TSEC2CM << SCCR_TSEC2CM_SHIFT);
#endif
#ifdef CFG_SCCR_USBMPHCM
	/* USB MPH clock mode */
	im->clk.sccr = (im->clk.sccr & ~SCCR_USBMPHCM) | (CFG_SCCR_USBMPHCM << SCCR_USBMPHCM_SHIFT);
#endif
#endif /* CONFIG_MPC834X */

#ifdef CFG_SCCR_PCICM
	/* PCI & DMA clock mode */
	im->clk.sccr = (im->clk.sccr & ~SCCR_PCICM) | (CFG_SCCR_PCICM << SCCR_PCICM_SHIFT);
#endif

#ifdef CFG_SCCR_USBDRCM
	/* USB DR clock mode */
	im->clk.sccr = (im->clk.sccr & ~SCCR_USBDRCM) | (CFG_SCCR_USBDRCM << SCCR_USBDRCM_SHIFT);
#endif

#ifdef CFG_SCCR_ENCCM
	/* Encryption clock mode */
	im->clk.sccr = (im->clk.sccr & ~SCCR_ENCCM) | (CFG_SCCR_ENCCM << SCCR_PCICM_SHIFT);
#endif

#ifdef CFG_ACR_RPTCNT
	/* Arbiter repeat count */
	im->arbiter.acr = ((im->arbiter.acr & ~(ACR_RPTCNT)) | (CFG_ACR_RPTCNT << ACR_RPTCNT_SHIFT));
#endif

	/* RSR - Reset Status Register - clear all status (4.6.1.3) */
	gd->reset_status = im->reset.rsr;
	im->reset.rsr = ~(RSR_RES);

	/*
	 * RMR - Reset Mode Register
	 * contains checkstop reset enable (4.6.1.4)
	 */
	im->reset.rmr = (RMR_CSRE & (1<<RMR_CSRE_SHIFT));

	/* LCRR - Clock Ratio Register (10.3.1.16) */
	im->lbus.lcrr = CFG_LCRR;

	/* Enable Time Base & Decrimenter ( so we will have udelay() )*/
	im->sysconf.spcr |= SPCR_TBEN;

	/* System General Purpose Register */
#ifdef CFG_SICRH
	im->sysconf.sicrh = CFG_SICRH;
#endif
#ifdef CFG_SICRL
	im->sysconf.sicrl = CFG_SICRL;
#endif
	/* DDR control driver register */
#ifdef CFG_DDRCDR
	im->sysconf.ddrcdr = CFG_DDRCDR;
#endif

#ifdef CONFIG_QE
	/* Config QE ioports */
	config_qe_ioports();
#endif

	/*
	 * Memory Controller:
	 */

	/* Map banks 0 and 1 to the FLASH banks 0 and 1 at preliminary
	 * addresses - these have to be modified later when FLASH size
	 * has been determined
	 */

#if defined(CFG_BR0_PRELIM)  \
	&& defined(CFG_OR0_PRELIM) \
	&& defined(CFG_LBLAWBAR0_PRELIM) \
	&& defined(CFG_LBLAWAR0_PRELIM)
	im->lbus.bank[0].br = CFG_BR0_PRELIM;
	im->lbus.bank[0].or = CFG_OR0_PRELIM;
	im->sysconf.lblaw[0].bar = CFG_LBLAWBAR0_PRELIM;
	im->sysconf.lblaw[0].ar = CFG_LBLAWAR0_PRELIM;
#else
#error 	CFG_BR0_PRELIM, CFG_OR0_PRELIM, CFG_LBLAWBAR0_PRELIM & CFG_LBLAWAR0_PRELIM must be defined
#endif

#if defined(CFG_BR1_PRELIM) && defined(CFG_OR1_PRELIM)
	im->lbus.bank[1].br = CFG_BR1_PRELIM;
	im->lbus.bank[1].or = CFG_OR1_PRELIM;
#endif
#if defined(CFG_LBLAWBAR1_PRELIM) && defined(CFG_LBLAWAR1_PRELIM)
	im->sysconf.lblaw[1].bar = CFG_LBLAWBAR1_PRELIM;
	im->sysconf.lblaw[1].ar = CFG_LBLAWAR1_PRELIM;
#endif
#if defined(CFG_BR2_PRELIM) && defined(CFG_OR2_PRELIM)
	im->lbus.bank[2].br = CFG_BR2_PRELIM;
	im->lbus.bank[2].or = CFG_OR2_PRELIM;
#endif
#if defined(CFG_LBLAWBAR2_PRELIM) && defined(CFG_LBLAWAR2_PRELIM)
	im->sysconf.lblaw[2].bar = CFG_LBLAWBAR2_PRELIM;
	im->sysconf.lblaw[2].ar = CFG_LBLAWAR2_PRELIM;
#endif
#if defined(CFG_BR3_PRELIM) && defined(CFG_OR3_PRELIM)
	im->lbus.bank[3].br = CFG_BR3_PRELIM;
	im->lbus.bank[3].or = CFG_OR3_PRELIM;
#endif
#if defined(CFG_LBLAWBAR3_PRELIM) && defined(CFG_LBLAWAR3_PRELIM)
	im->sysconf.lblaw[3].bar = CFG_LBLAWBAR3_PRELIM;
	im->sysconf.lblaw[3].ar = CFG_LBLAWAR3_PRELIM;
#endif
#if defined(CFG_BR4_PRELIM) && defined(CFG_OR4_PRELIM)
	im->lbus.bank[4].br = CFG_BR4_PRELIM;
	im->lbus.bank[4].or = CFG_OR4_PRELIM;
#endif
#if defined(CFG_LBLAWBAR4_PRELIM) && defined(CFG_LBLAWAR4_PRELIM)
	im->sysconf.lblaw[4].bar = CFG_LBLAWBAR4_PRELIM;
	im->sysconf.lblaw[4].ar = CFG_LBLAWAR4_PRELIM;
#endif
#if defined(CFG_BR5_PRELIM) && defined(CFG_OR5_PRELIM)
	im->lbus.bank[5].br = CFG_BR5_PRELIM;
	im->lbus.bank[5].or = CFG_OR5_PRELIM;
#endif
#if defined(CFG_LBLAWBAR5_PRELIM) && defined(CFG_LBLAWAR5_PRELIM)
	im->sysconf.lblaw[5].bar = CFG_LBLAWBAR5_PRELIM;
	im->sysconf.lblaw[5].ar = CFG_LBLAWAR5_PRELIM;
#endif
#if defined(CFG_BR6_PRELIM) && defined(CFG_OR6_PRELIM)
	im->lbus.bank[6].br = CFG_BR6_PRELIM;
	im->lbus.bank[6].or = CFG_OR6_PRELIM;
#endif
#if defined(CFG_LBLAWBAR6_PRELIM) && defined(CFG_LBLAWAR6_PRELIM)
	im->sysconf.lblaw[6].bar = CFG_LBLAWBAR6_PRELIM;
	im->sysconf.lblaw[6].ar = CFG_LBLAWAR6_PRELIM;
#endif
#if defined(CFG_BR7_PRELIM) && defined(CFG_OR7_PRELIM)
	im->lbus.bank[7].br = CFG_BR7_PRELIM;
	im->lbus.bank[7].or = CFG_OR7_PRELIM;
#endif
#if defined(CFG_LBLAWBAR7_PRELIM) && defined(CFG_LBLAWAR7_PRELIM)
	im->sysconf.lblaw[7].bar = CFG_LBLAWBAR7_PRELIM;
	im->sysconf.lblaw[7].ar = CFG_LBLAWAR7_PRELIM;
#endif
#ifdef CFG_GPIO1_PRELIM
	im->gpio[0].dir = CFG_GPIO1_DIR;
	im->gpio[0].dat = CFG_GPIO1_DAT;
#endif
#ifdef CFG_GPIO2_PRELIM
	im->gpio[1].dir = CFG_GPIO2_DIR;
	im->gpio[1].dat = CFG_GPIO2_DAT;
#endif
}

int cpu_init_r (void)
{
#ifdef CONFIG_QE
	uint qe_base = CFG_IMMR + 0x00100000; /* QE immr base */
	qe_init(qe_base);
	qe_reset();
#endif
	return 0;
}

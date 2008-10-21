/*
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
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

DECLARE_GLOBAL_DATA_PTR;

/*
 * Breathe some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f (volatile immap_t * im)
{
	int i;

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET);

	/* Clear initial global data */
	for (i = 0; i < sizeof(gd_t); i++)
		((char *)gd)[i] = 0;

	/* system performance tweaking */

#ifdef CONFIG_SYS_ACR_PIPE_DEP
	/* Arbiter pipeline depth */
	im->arbiter.acr = (im->arbiter.acr & ~ACR_PIPE_DEP) |
			  (CONFIG_SYS_ACR_PIPE_DEP << ACR_PIPE_DEP_SHIFT);
#endif

#ifdef CONFIG_SYS_ACR_RPTCNT
	/* Arbiter repeat count */
	im->arbiter.acr = (im->arbiter.acr & ~(ACR_RPTCNT)) |
			  (CONFIG_SYS_ACR_RPTCNT << ACR_RPTCNT_SHIFT);
#endif

#ifdef CONFIG_SYS_SPCR_OPT
	/* Optimize transactions between CSB and other devices */
	im->sysconf.spcr = (im->sysconf.spcr & ~SPCR_OPT) |
			   (CONFIG_SYS_SPCR_OPT << SPCR_OPT_SHIFT);
#endif

	/* Enable Time Base & Decrimenter (so we will have udelay()) */
	im->sysconf.spcr |= SPCR_TBEN;

	/* DDR control driver register */
#ifdef CONFIG_SYS_DDRCDR
	im->sysconf.ddrcdr = CONFIG_SYS_DDRCDR;
#endif
	/* Output buffer impedance register */
#ifdef CONFIG_SYS_OBIR
	im->sysconf.obir = CONFIG_SYS_OBIR;
#endif

	/*
	 * Memory Controller:
	 */

	/* Map banks 0 and 1 to the FLASH banks 0 and 1 at preliminary
	 * addresses - these have to be modified later when FLASH size
	 * has been determined
	 */

#if defined(CONFIG_SYS_NAND_BR_PRELIM)  \
	&& defined(CONFIG_SYS_NAND_OR_PRELIM) \
	&& defined(CONFIG_SYS_NAND_LBLAWBAR_PRELIM) \
	&& defined(CONFIG_SYS_NAND_LBLAWAR_PRELIM)
	im->lbus.bank[0].br = CONFIG_SYS_NAND_BR_PRELIM;
	im->lbus.bank[0].or = CONFIG_SYS_NAND_OR_PRELIM;
	im->sysconf.lblaw[0].bar = CONFIG_SYS_NAND_LBLAWBAR_PRELIM;
	im->sysconf.lblaw[0].ar = CONFIG_SYS_NAND_LBLAWAR_PRELIM;
#else
#error CONFIG_SYS_NAND_BR_PRELIM, CONFIG_SYS_NAND_OR_PRELIM, CONFIG_SYS_NAND_LBLAWBAR_PRELIM & CONFIG_SYS_NAND_LBLAWAR_PRELIM must be defined
#endif
}

/*
 * Get timebase clock frequency (like cpu_clk in Hz)
 */
unsigned long get_tbclk(void)
{
	return (gd->bus_clk + 3L) / 4L;
}

void puts(const char *str)
{
	while (*str)
		putc(*str++);
}

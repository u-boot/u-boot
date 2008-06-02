/*
 *    Copyright (c) 2008 Nuovation System Designs, LLC
 *      Grant Erickson <gerickson@nuovations.com>
 *
 *    (C) Copyright 2005-2007
 *    Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 *    (C) Copyright 2002
 *    Jun Gu, Artesyn Technology, jung@artesyncp.com
 *
 *    (C) Copyright 2001
 *    Bill Hunter, Wave 7 Optics, williamhunter@attbi.com
 *
 *    See file CREDITS for list of people who contributed to this
 *    project.
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License as
 *    published by the Free Software Foundation; either version 2 of
 *    the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will abe useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA 02111-1307 USA
 *
 *    Description:
 *	This file implements generic DRAM ECC initialization for
 *	PowerPC processors using a SDRAM DDR/DDR2 controller,
 *	including the 405EX(r), 440GP/GX/EP/GR, 440SP(E), and
 *	460EX/GT.
 */

#include <common.h>
#include <ppc4xx.h>
#include <ppc_asm.tmpl>
#include <ppc_defs.h>
#include <asm/processor.h>
#include <asm/io.h>

#include "ecc.h"

#if defined(CONFIG_SDRAM_PPC4xx_IBM_DDR) || \
    defined(CONFIG_SDRAM_PPC4xx_IBM_DDR2)
#if defined(CONFIG_DDR_ECC) || defined(CONFIG_SDRAM_ECC)
/*
 *  void ecc_init()
 *
 *  Description:
 *    This routine initializes a range of DRAM ECC memory with known
 *    data and enables ECC checking.
 *
 *  TO DO:
 *    - Improve performance by utilizing cache.
 *    - Further generalize to make usable by other 4xx variants (e.g.
 *      440EPx, et al).
 *
 *  Input(s):
 *    start - A pointer to the start of memory covered by ECC requiring
 *	      initialization.
 *    size  - The size, in bytes, of the memory covered by ECC requiring
 *	      initialization.
 *
 *  Output(s):
 *    start - A pointer to the start of memory covered by ECC with
 *	      CFG_ECC_PATTERN written to all locations and ECC data
 *	      primed.
 *
 *  Returns:
 *    N/A
 */
void ecc_init(unsigned long * const start, unsigned long size)
{
	const unsigned long pattern = CFG_ECC_PATTERN;
	unsigned long * const end = (unsigned long * const)((long)start + size);
	unsigned long * current = start;
	unsigned long mcopt1;
	long increment;

	if (start >= end)
		return;

	mfsdram(SDRAM_ECC_CFG, mcopt1);

	/* Enable ECC generation without checking or reporting */

	mtsdram(SDRAM_ECC_CFG, ((mcopt1 & ~SDRAM_ECC_CFG_MCHK_MASK) |
				SDRAM_ECC_CFG_MCHK_GEN));

	increment = sizeof(u32);

#if defined(CONFIG_440)
	/*
	 * Look at the geometry of SDRAM (data width) to determine whether we
	 * can skip words when writing.
	 */

	if ((mcopt1 & SDRAM_ECC_CFG_DMWD_MASK) != SDRAM_ECC_CFG_DMWD_32)
		increment = sizeof(u64);
#endif /* defined(CONFIG_440) */

	while (current < end) {
		*current = pattern;
		 current = (unsigned long *)((long)current + increment);
	}

	/* Wait until the writes are finished. */

	sync();

	/* Enable ECC generation with checking and no reporting */

	mtsdram(SDRAM_ECC_CFG, ((mcopt1 & ~SDRAM_ECC_CFG_MCHK_MASK) |
				SDRAM_ECC_CFG_MCHK_CHK));
}
#endif /* defined(CONFIG_DDR_ECC) || defined(CONFIG_SDRAM_ECC) */
#endif /* defined(CONFIG_SDRAM_PPC4xx_IBM_DDR)... */

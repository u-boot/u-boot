/*
 *    Copyright (c) 2008 Nuovation System Designs, LLC
 *      Grant Erickson <gerickson@nuovations.com>
 *
 *    (C) Copyright 2005-2009
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
#include <asm/mmu.h>
#include <asm/cache.h>

#include "ecc.h"

#if defined(CONFIG_SDRAM_PPC4xx_IBM_DDR) || \
    defined(CONFIG_SDRAM_PPC4xx_IBM_DDR2)
#if defined(CONFIG_DDR_ECC) || defined(CONFIG_SDRAM_ECC)

#if defined(CONFIG_405EX)
/*
 * Currently only 405EX uses 16bit data bus width as an alternative
 * option to 32bit data width (SDRAM0_MCOPT1_WDTH)
 */
#define SDRAM_DATA_ALT_WIDTH	2
#else
#define SDRAM_DATA_ALT_WIDTH	8
#endif

static void wait_ddr_idle(void)
{
	u32 val;

	do {
		mfsdram(SDRAM_MCSTAT, val);
	} while ((val & SDRAM_MCSTAT_IDLE_MASK) == SDRAM_MCSTAT_IDLE_NOT);
}

static void program_ecc_addr(unsigned long start_address,
			     unsigned long num_bytes,
			     unsigned long tlb_word2_i_value)
{
	unsigned long current_address;
	unsigned long end_address;
	unsigned long address_increment;
	unsigned long mcopt1;
	char str[] = "ECC generation -";
	char slash[] = "\\|/-\\|/-";
	int loop = 0;
	int loopi = 0;

	current_address = start_address;
	mfsdram(SDRAM_MCOPT1, mcopt1);
	if ((mcopt1 & SDRAM_MCOPT1_MCHK_MASK) != SDRAM_MCOPT1_MCHK_NON) {
		mtsdram(SDRAM_MCOPT1,
			(mcopt1 & ~SDRAM_MCOPT1_MCHK_MASK) | SDRAM_MCOPT1_MCHK_GEN);
		sync();
		eieio();
		wait_ddr_idle();

		puts(str);

#ifdef CONFIG_440
		if (tlb_word2_i_value == TLB_WORD2_I_ENABLE) {
#endif
			/* ECC bit set method for non-cached memory */
			if ((mcopt1 & SDRAM_MCOPT1_DMWD_MASK) == SDRAM_MCOPT1_DMWD_32)
				address_increment = 4;
			else
				address_increment = SDRAM_DATA_ALT_WIDTH;
			end_address = current_address + num_bytes;

			while (current_address < end_address) {
				*((unsigned long *)current_address) = 0;
				current_address += address_increment;

				if ((loop++ % (2 << 20)) == 0) {
					putc('\b');
					putc(slash[loopi++ % 8]);
				}
			}
#ifdef CONFIG_440
		} else {
			/* ECC bit set method for cached memory */
			dcbz_area(start_address, num_bytes);
			/* Write modified dcache lines back to memory */
			clean_dcache_range(start_address, start_address + num_bytes);
		}
#endif /* CONFIG_440 */

		blank_string(strlen(str));

		sync();
		eieio();
		wait_ddr_idle();

		/* clear ECC error repoting registers */
		mtsdram(SDRAM_ECCES, 0xffffffff);
		mtdcr(0x4c, 0xffffffff);

		mtsdram(SDRAM_MCOPT1,
			(mcopt1 & ~SDRAM_MCOPT1_MCHK_MASK) | SDRAM_MCOPT1_MCHK_CHK_REP);
		sync();
		eieio();
		wait_ddr_idle();
	}
}

#if defined(CONFIG_SDRAM_PPC4xx_IBM_DDR)
void ecc_init(unsigned long * const start, unsigned long size)
{
	/*
	 * Init ECC with cache disabled (on PPC's with IBM DDR
	 * controller (non DDR2), not tested with cache enabled yet
	 */
	program_ecc_addr((u32)start, size, TLB_WORD2_I_ENABLE);
}
#endif

#if defined(CONFIG_SDRAM_PPC4xx_IBM_DDR2)
void do_program_ecc(unsigned long tlb_word2_i_value)
{
	unsigned long mcopt1;
	unsigned long mcopt2;
	unsigned long mcstat;
	phys_size_t memsize = sdram_memsize();

	if (memsize > CONFIG_MAX_MEM_MAPPED) {
		printf("\nWarning: Can't enable ECC on systems with more than 2GB of SDRAM!\n");
		return;
	}

	mfsdram(SDRAM_MCOPT1, mcopt1);
	mfsdram(SDRAM_MCOPT2, mcopt2);

	if ((mcopt1 & SDRAM_MCOPT1_MCHK_MASK) != SDRAM_MCOPT1_MCHK_NON) {
		/* DDR controller must be enabled and not in self-refresh. */
		mfsdram(SDRAM_MCSTAT, mcstat);
		if (((mcopt2 & SDRAM_MCOPT2_DCEN_MASK) == SDRAM_MCOPT2_DCEN_ENABLE)
		    && ((mcopt2 & SDRAM_MCOPT2_SREN_MASK) == SDRAM_MCOPT2_SREN_EXIT)
		    && ((mcstat & (SDRAM_MCSTAT_MIC_MASK | SDRAM_MCSTAT_SRMS_MASK))
			== (SDRAM_MCSTAT_MIC_COMP | SDRAM_MCSTAT_SRMS_NOT_SF))) {

			program_ecc_addr(0, memsize, tlb_word2_i_value);
		}
	}
}
#endif

#endif /* defined(CONFIG_DDR_ECC) || defined(CONFIG_SDRAM_ECC) */
#endif /* defined(CONFIG_SDRAM_PPC4xx_IBM_DDR)... */

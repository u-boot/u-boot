/*
 * (C) Copyright 2003
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
#include <command.h>
#include <asm/addrspace.h>
#include <asm/inca-ip.h>


extern uint incaip_get_cpuclk(void);

static ulong max_sdram_size(void)
{
	/* The only supported SDRAM data width is 16bit.
	 */
#define CFG_DW	2

	/* The only supported number of SDRAM banks is 4.
	 */
#define CFG_NB	4

	ulong cfgpb0 = *INCA_IP_SDRAM_MC_CFGPB0;
	int   cols   = cfgpb0 & 0xF;
	int   rows   = (cfgpb0 & 0xF0) >> 4;
	ulong size   = (1 << (rows + cols)) * CFG_DW * CFG_NB;

	return size;
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */

static long int dram_size(long int *base, long int maxsize)
{
	volatile long int *addr;
	ulong cnt, val;
	ulong save[32];			/* to make test non-destructive */
	unsigned char i = 0;

	for (cnt = (maxsize / sizeof (long)) >> 1; cnt > 0; cnt >>= 1) {
		addr = base + cnt;		/* pointer arith! */

		save[i++] = *addr;
		*addr = ~cnt;
	}

	/* write 0 to base address */
	addr = base;
	save[i] = *addr;
	*addr = 0;

	/* check at base address */
	if ((val = *addr) != 0) {
		*addr = save[i];
		return (0);
	}

	for (cnt = 1; cnt < maxsize / sizeof (long); cnt <<= 1) {
		addr = base + cnt;		/* pointer arith! */

		val = *addr;
		*addr = save[--i];

		if (val != (~cnt)) {
			return (cnt * sizeof (long));
		}
	}
	return (maxsize);
}

long int initdram(int board_type)
{
	int   rows, cols, best_val = *INCA_IP_SDRAM_MC_CFGPB0;
	ulong size, max_size       = 0;
	ulong our_address;

	asm volatile ("move %0, $25" : "=r" (our_address) :);

		/* Can't probe for RAM size unless we are running from Flash.
		 */
	if (PHYSADDR(our_address) < PHYSADDR(PHYS_FLASH_1))
	{
		return max_sdram_size();
	}

	for (cols = 0x8; cols <= 0xC; cols++)
	{
		for (rows = 0xB; rows <= 0xD; rows++)
		{
			*INCA_IP_SDRAM_MC_CFGPB0 = (0x14 << 8) |
			                           (rows << 4) | cols;
			size = dram_size((ulong *)CFG_SDRAM_BASE,
			                                     max_sdram_size());

			if (size > max_size)
			{
				best_val = *INCA_IP_SDRAM_MC_CFGPB0;
				max_size = size;
			}
		}
	}

	*INCA_IP_SDRAM_MC_CFGPB0 = best_val;
	return max_size;
}

int checkboard (void)
{

	unsigned long chipid = *INCA_IP_WDT_CHIPID;
	int part_num;

	puts ("Board: INCA-IP ");
	part_num = (chipid >> 12) & 0xffff;
	switch (part_num) {
	case 0xc0:
		printf ("Standard Version, ");
		break;
	case 0xc1:
		printf ("Basic Version, ");
		break;
	default:
		printf ("Unknown Part Number 0x%x ", part_num);
		break;
	}

	printf ("Chip V1.%ld, ", (chipid >> 28));

	printf("CPU Speed %d MHz\n", incaip_get_cpuclk()/1000000);

	return 0;
}

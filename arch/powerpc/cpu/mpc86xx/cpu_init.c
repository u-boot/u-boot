/*
 * Copyright 2004,2009-2011 Freescale Semiconductor, Inc.
 * Jeff Brown
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
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

/*
 * cpu_init.c - low level cpu init
 */

#include <config.h>
#include <common.h>
#include <mpc86xx.h>
#include <asm/mmu.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/mp.h>

extern void srio_init(void);

DECLARE_GLOBAL_DATA_PTR;

/*
 * Breathe some life into the CPU...
 *
 * Set up the memory map
 * initialize a bunch of registers
 */

void cpu_init_f(void)
{
	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET);

	/* Clear initial global data */
	memset ((void *) gd, 0, sizeof (gd_t));

#ifdef CONFIG_FSL_LAW
	init_laws();
#endif

	setup_bats();

	init_early_memctl_regs();

#if defined(CONFIG_FSL_DMA)
	dma_init();
#endif

	/* enable the timebase bit in HID0 */
	set_hid0(get_hid0() | 0x4000000);

	/* enable EMCP, SYNCBE | ABE bits in HID1 */
	set_hid1(get_hid1() | 0x80000C00);
}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r(void)
{
	/* needs to be in ram since code uses global static vars */
	fsl_serdes_init();

#ifdef CONFIG_SYS_SRIO
	srio_init();
#endif

#if defined(CONFIG_MP)
	setup_mp();
#endif
	return 0;
}

#ifdef CONFIG_ADDR_MAP
/* Initialize address mapping array */
void init_addr_map(void)
{
	int i;
	ppc_bat_t bat = DBAT0;
	phys_size_t size;
	unsigned long upper, lower;

	for (i = 0; i < CONFIG_SYS_NUM_ADDR_MAP; i++, bat++) {
		if (read_bat(bat, &upper, &lower) != -1) {
			if (!BATU_VALID(upper))
				size = 0;
			else
				size = BATU_SIZE(upper);
			addrmap_set_entry(BATU_VADDR(upper), BATL_PADDR(lower),
					  size, i);
		}
#ifdef CONFIG_HIGH_BATS
		/* High bats are not contiguous with low BAT numbers */
		if (bat == DBAT3)
			bat = DBAT4 - 1;
#endif
	}
}
#endif

/*
 * (C) Copyright 2008-2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
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
#include <asm/io.h>
#include <asm/processor-flags.h>
#include <asm/ic/sc520.h>

DECLARE_GLOBAL_DATA_PTR;

sc520_mmcr_t *sc520_mmcr = (sc520_mmcr_t *)SC520_MMCR_BASE;

int cpu_init_f(void)
{
	if (CONFIG_SYS_SC520_HIGH_SPEED) {
		/* set it to 133 MHz and write back */
		writeb(0x02, &sc520_mmcr->cpuctl);
		gd->cpu_clk = 133000000;
	} else {
		/* set it to 100 MHz and write back */
		writeb(0x01, &sc520_mmcr->cpuctl);
		gd->cpu_clk = 100000000;
	}

	/* wait at least one millisecond */
	asm("movl	$0x2000, %%ecx\n"
	    "0:		pushl %%ecx\n"
	    "popl	%%ecx\n"
	    "loop 0b\n": : : "ecx");

	return x86_cpu_init_f();
}

int cpu_init_r(void)
{
	/* Disable the PAR used for CAR */
	writel(0x0000000, &sc520_mmcr->par[2]);

	/* turn on the SDRAM write buffer */
	writeb(0x11, &sc520_mmcr->dbctl);

	return x86_cpu_init_r();
}

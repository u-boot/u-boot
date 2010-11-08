/* CPU specific code for the LEON3 CPU
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com
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
#include <command.h>
#include <netdev.h>

#include <asm/io.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

extern void _reset_reloc(void);

int checkcpu(void)
{
	/* check LEON version here */
	printf("CPU: LEON3\n");
	return 0;
}

/* ------------------------------------------------------------------------- */

void cpu_reset(void)
{
	/* Interrupts off */
	disable_interrupts();

	/* jump to restart in flash */
	_reset_reloc();
}

int do_reset(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	cpu_reset();

	return 1;

}

u64 flash_read64(void *addr)
{
	return __raw_readq(addr);
}

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_GRETH
int cpu_eth_init(bd_t *bis)
{
	return greth_initialize(bis);
}
#endif

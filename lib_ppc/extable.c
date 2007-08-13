/*
 * Copyright (C) 1999  Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
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

/*
 * The exception table consists of pairs of addresses: the first is the
 * address of an instruction that is allowed to fault, and the second is
 * the address at which the program should continue.  No registers are
 * modified, so it is entirely up to the continuation code to figure out
 * what to do.
 *
 * All the routines below use bits of fixup code that are out of line
 * with the main instruction path.  This means when everything is well,
 * we don't even have to jump over them.  Further, they do not intrude
 * on our cache or tlb entries.
 */

DECLARE_GLOBAL_DATA_PTR;

struct exception_table_entry
{
	unsigned long insn, fixup;
};

extern const struct exception_table_entry __start___ex_table[];
extern const struct exception_table_entry __stop___ex_table[];

static inline unsigned long
search_one_table(const struct exception_table_entry *first,
		 const struct exception_table_entry *last,
		 unsigned long value)
{
	long diff;
	if ((ulong) first > CFG_MONITOR_BASE) {
		/* exception occurs in FLASH, before u-boot relocation.
		 * No relocation offset is needed.
		 */
		while (first <= last) {
			diff = first->insn - value;
			if (diff == 0)
				return first->fixup;
			first++;
		}
	} else {
		/* exception occurs in RAM, after u-boot relocation.
		 * A relocation offset should be added.
		 */
		while (first <= last) {
			diff = (first->insn + gd->reloc_off) - value;
			if (diff == 0)
				return (first->fixup + gd->reloc_off);
			first++;
		}
	}
	return 0;
}

int	ex_tab_message = 1;

unsigned long
search_exception_table(unsigned long addr)
{
	unsigned long ret;

	/* There is only the kernel to search.  */
	ret = search_one_table(__start___ex_table, __stop___ex_table-1, addr);
	/* if the serial port does not hang in exception, printf can be used */
#if !defined(CFG_SERIAL_HANG_IN_EXCEPTION)
	if (ex_tab_message)
		printf("Bus Fault @ 0x%08lx, fixup 0x%08lx\n", addr, ret);
#endif
	if (ret) return ret;

	return 0;
}

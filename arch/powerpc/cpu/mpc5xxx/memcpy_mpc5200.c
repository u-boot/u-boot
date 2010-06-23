/*
 * (C) Copyright 2010
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

/*
 * This is a workaround for issues on the MPC5200, where unaligned
 * 32-bit-accesses to the local bus will deliver corrupted data. This
 * happens for example when trying to use memcpy() from an odd NOR
 * flash address; the behaviour can be also seen when using "md" on an
 * odd NOR flash address (but there it is not a bug in U-Boot, which
 * only shows the behaviour of this processor).
 *
 * For memcpy(), we test if either the source or the target address
 * are not 32 bit aligned, and - if so - if the source address is in
 * NOR flash: in this case we perform a byte-wise (slow) then; for
 * aligned operations of non-flash areas we use the optimized (fast)
 * real __memcpy().  This way we minimize the performance impact of
 * this workaround.
 *
 */

#include <common.h>
#include <flash.h>
#include <linux/types.h>

void *memcpy(void *trg, const void *src, size_t len)
{
	extern void* __memcpy(void *, const void *, size_t);
	char *s = (char *)src;
	char *t = (char *)trg;
	void *dest = (void *)src;

	/*
	 * Check is source address is in flash:
	 * If not, we use the fast assembler code
	 */
	if (((((unsigned long)s & 3) == 0)	/* source aligned  */
		&&				/*	AND	   */
	     (((unsigned long)t & 3) == 0))	/* target aligned, */
		||				/*	or	   */
	    (addr2info((ulong)s) == NULL)) {	/* source not in flash */
		return __memcpy(trg, src, len);
	}

	/*
	 * Copying from flash, perform byte by byte copy.
	 */
	while (len-- > 0)
		*t++ = *s++;

	return dest;
}

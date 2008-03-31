/*
 * (C) Copyright 2002
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
 * CPU test
 * Load/store multiple word instructions:	lmw, stmw
 *
 * 26 consecutive words are loaded from a source memory buffer
 * into GPRs r6 through r31. After that, 26 consecutive words are stored
 * from the GPRs r6 through r31 into a target memory buffer. The contents
 * of the source and target buffers are then compared.
 */

#include <post.h>
#include "cpu_asm.h"

#if CONFIG_POST & CFG_POST_CPU

extern void cpu_post_exec_02 (ulong *code, ulong op1, ulong op2);

int cpu_post_test_multi (void)
{
    int ret = 0;
    unsigned int i;

    if (ret == 0)
    {
	ulong src [26], dst [26];

	ulong code[] =
	{
	    ASM_LMW(5, 3, 0),
	    ASM_STMW(5, 4, 0),
	    ASM_BLR,
	};

	for (i = 0; i < sizeof(src) / sizeof(src[0]); i ++)
	{
	    src[i] = i;
	    dst[i] = 0;
	}

	cpu_post_exec_02(code, (ulong)src, (ulong)dst);

	ret = memcmp(src, dst, sizeof(dst)) == 0 ? 0 : -1;
    }

    if (ret != 0)
    {
	post_log ("Error at multi test !\n");
    }

    return ret;
}

#endif

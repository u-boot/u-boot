/*
 * (C) Copyright 2002
 * Hyperion Entertainment, ThomasF@hyperion-entertainment.com
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include "articiaS.h"
#include "memio.h"
#include "via686.h"

__asm(" .globl send_kb                                      \n
	send_kb:                                            \n
		lis     r9, 0xfe00                          \n
							    \n
		li      r4, 0x10        # retries           \n
		mtctr   r4                                  \n
							    \n
	idle:                                               \n
		lbz     r4, 0x64(r9)                        \n
		andi.   r4, r4, 0x02                        \n
		bne     idle                                \n
							    \n
	ready:                                              \n
		stb     r3, 0x60(r9)                        \n
							    \n
	check:                                              \n
		lbz     r4, 0x64(r9)                        \n
		andi.   r4, r4, 0x01                        \n
		beq     check                               \n
							    \n
		lbz     r4, 0x60(r9)                        \n
		cmpwi   r4, 0xfa                            \n
		beq     done                                \n
							    \n
		bdnz    idle                                \n
							    \n
		li      r3, 0                               \n
		blr                                         \n
							    \n
	done:                                               \n
		li      r3, 1                               \n
		blr                                         \n
							    \n
	.globl test_kb                                      \n
	test_kb:                                            \n
		mflr    r10                                 \n
		li      r3, 0xed                            \n
		bl      send_kb                             \n
		li      r3, 0x01                            \n
		bl      send_kb                             \n
		mtlr    r10                                 \n
		blr                                         \n
");


int checkboard (void)
{
	printf ("Board: AmigaOneG3SE\n");
	return 0;
}

long initdram (int board_type)
{
	return articiaS_ram_init ();
}


void after_reloc (ulong dest_addr, gd_t *gd)
{
/* HJF:	DECLARE_GLOBAL_DATA_PTR; */

	board_init_r (gd, dest_addr);
}


int misc_init_r (void)
{
	extern pci_dev_t video_dev;
	extern void drv_video_init (void);

	if (video_dev != ~0)
		drv_video_init ();

	return (0);
}


void pci_init_board (void)
{
#ifndef CONFIG_RAMBOOT
	articiaS_pci_init ();
#endif
}

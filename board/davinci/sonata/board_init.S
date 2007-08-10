/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Board-specific low level initialization code. Called at the very end
 * of cpu/arm926ejs/davinci/lowlevel_init.S. Just returns if there is no
 * initialization required.
 *
 * For _OLDER_ Sonata boards sets up GPIO4 to control NAND WP line. Newer
 * Sonata boards, AFAIK, don't use this so it's just return by default. Ask
 * Visioneering if they reinvented the wheel once again to make sure :)
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

#include <config.h>

.globl	dv_board_init
dv_board_init:
#ifdef SONATA_BOARD_GPIOWP
	/* Set PINMUX0 to enable GPIO4 */
	ldr	r0, _PINMUX0
	ldr	r1, GPIO4_EN_MASK
	ldr	r2, [r0]
	and	r2, r2, r1
	str	r2, [r0]

	/* Enable GPIO LPSC module */
	ldr	r0, PTSTAT

gpio_ptstat_loop1:
	ldr	r2, [r0]
	tst	r2, $0x00000001
	bne	gpio_ptstat_loop1

	ldr	r1, MDCTL_GPIO
	ldr	r2, [r1]
	and	r2, r2, $0xfffffff8
	orr	r2, r2, $0x00000003
	str	r2, [r1]

	orr	r2, r2, $0x00000200
	str	r2, [r1]

	ldr	r1, PTCMD
	mov	r2, $0x00000001
	str	r2, [r1]

gpio_ptstat_loop2:
	ldr	r2, [r0]
	tst	r2, $0x00000001
	bne	gpio_ptstat_loop2

	ldr	r0, MDSTAT_GPIO
gpio_mdstat_loop:
	ldr	r2, [r0]
	and	r2, r2, $0x0000001f
	teq	r2, $0x00000003
	bne	gpio_mdstat_loop

	/* GPIO4 -> output */
	ldr	r0, GPIO_DIR01
	mov	r1, $0x10
	ldr	r2, [r0]
	bic	r2, r2, r0
	str	r2, [r0]

	/* Set it to 0 (Write Protect) */
	ldr	r0, GPIO_CLR_DATA01
	str	r1, [r0]
#endif

	mov	pc, lr

#ifdef SONATA_BOARD_GPIOWP
.ltorg

GPIO4_EN_MASK:
	.word	0xf77fffff
MDCTL_GPIO:
	.word	0x01c41a68
MDSTAT_GPIO:
	.word	0x01c41868
GPIO_DIR01:
	.word	0x01c67010
GPIO_CLR_DATA01:
	.word	0x01c6701c
#endif

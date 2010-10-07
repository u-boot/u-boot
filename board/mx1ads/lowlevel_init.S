/*
 * board/mx1ads/lowlevel_init.S
 *
 * (c) Copyright 2004
 * Techware Information Technology, Inc.
 * http://www.techware.com.tw/
 *
 * Ming-Len Wu <minglen_wu@techware.com.tw>
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
#include <version.h>

#define SDCTL0			0x221000
#define SDCTL1			0x221004


_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE

.globl lowlevel_init
lowlevel_init:
/* memory controller init		*/

	ldr  r1, =SDCTL0

/*  Set Precharge Command		*/

	ldr  r3, =0x92120200
/*	ldr  r3, =0x92120251
*/
	str  r3, [r1]

/* Issue Precharge All Commad		*/
	ldr  r3, =0x8200000
	ldr  r2, [r3]

/* Set AutoRefresh Command		*/
	ldr  r3, =0xA2120200
	str  r3, [r1]

/* Issue AutoRefresh Command		*/
	ldr  r3, =0x8000000
	ldr  r2, [r3]
	ldr  r2, [r3]
	ldr  r2, [r3]
	ldr  r2, [r3]
	ldr  r2, [r3]
	ldr  r2, [r3]
	ldr  r2, [r3]
	ldr  r2, [r3]

/* Set Mode Register			*/
	ldr  r3, =0xB2120200
	str  r3, [r1]

/* Issue Mode Register Command		*/
	ldr  r3, =0x08111800	/* Mode Register Value		*/
	ldr  r2, [r3]

/* Set Normal Mode			*/
	ldr  r3, =0x82124200
	str  r3, [r1]

/* everything is fine now		*/
	mov	pc, lr

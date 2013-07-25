/*
 * board/mx1ads/lowlevel_init.S
 *
 * (c) Copyright 2004
 * Techware Information Technology, Inc.
 * http://www.techware.com.tw/
 *
 * Ming-Len Wu <minglen_wu@techware.com.tw>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

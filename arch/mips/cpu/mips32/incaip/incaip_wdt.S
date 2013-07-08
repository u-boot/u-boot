/*
 *  INCA-IP Watchdog timer management code.
 *
 *  Copyright (c) 2003	Wolfgang Denk <wd@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#include <config.h>
#include <asm/regdef.h>


#define WD_BASE            0xb8000000
#define WD_CON0(value)     0x0020(value)
#define WD_CON1(value)     0x0024(value)
#define WD_DISABLE         0x00000008
#define WD_ENABLE          0x00000000
#define WD_WRITE_PW        0xFFFC00F8
#define WD_WRITE_ENDINIT   0xFFFC00F3
#define WD_WRITE_INIT      0xFFFC00F2


	.globl	disable_incaip_wdt
disable_incaip_wdt:
	li	t0, WD_BASE

	/* Calculate password.
	 */
	lw	t2, WD_CON1(t0)
	and	t2, 0xC

	lw	t3, WD_CON0(t0)
	and	t3, 0xFFFFFF01

	or	t3, t2
	or	t3, 0xF0

	sw	t3, WD_CON0(t0)		/* write password */

	/* Clear ENDINIT.
	 */
	li	t1, WD_WRITE_INIT
	sw	t1, WD_CON0(t0)


	li	t1, WD_DISABLE
	sw	t1, WD_CON1(t0)		/* disable watchdog */
	li	t1, WD_WRITE_PW
	sw	t1, WD_CON0(t0)		/* write password */
	li	t1, WD_WRITE_ENDINIT
	sw	t1, WD_CON0(t0)		/* end command */

	jr	ra
	nop

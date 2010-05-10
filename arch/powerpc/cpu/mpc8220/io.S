/*
 *  Copyright (C) 1998	Dan Malek <dmalek@jlc.net>
 *  Copyright (C) 1999	Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *  Copyright (C) 2001	Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 *			Andreas Heppel <aheppel@sysgo.de>
 *  Copyright (C) 2003	Wolfgang Denk <wd@denx.de>
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

#include <config.h>
#include <ppc_asm.tmpl>

/* ------------------------------------------------------------------------------- */
/*  Function:	  in8 */
/*  Description:  Input 8 bits */
/* ------------------------------------------------------------------------------- */
	.globl	in8
in8:
	lbz	r3,0(r3)
	sync
	blr

/* ------------------------------------------------------------------------------- */
/*  Function:	  in16 */
/*  Description:  Input 16 bits */
/* ------------------------------------------------------------------------------- */
	.globl	in16
in16:
	lhz	r3,0(r3)
	sync
	blr

/* ------------------------------------------------------------------------------- */
/*  Function:	  in16r */
/*  Description:  Input 16 bits and byte reverse */
/* ------------------------------------------------------------------------------- */
	.globl	in16r
in16r:
	lhbrx	r3,0,r3
	sync
	blr

/* ------------------------------------------------------------------------------- */
/*  Function:	  in32 */
/*  Description:  Input 32 bits */
/* ------------------------------------------------------------------------------- */
	.globl	in32
in32:
	lwz	3,0(3)
	sync
	blr

/* ------------------------------------------------------------------------------- */
/*  Function:	  in32r */
/*  Description:  Input 32 bits and byte reverse */
/* ------------------------------------------------------------------------------- */
	.globl	in32r
in32r:
	lwbrx	r3,0,r3
	sync
	blr

/* ------------------------------------------------------------------------------- */
/*  Function:	  out8 */
/*  Description:  Output 8 bits */
/* ------------------------------------------------------------------------------- */
	.globl	out8
out8:
	stb	r4,0(r3)
	sync
	blr

/* ------------------------------------------------------------------------------- */
/*  Function:	  out16 */
/*  Description:  Output 16 bits */
/* ------------------------------------------------------------------------------- */
	.globl	out16
out16:
	sth	r4,0(r3)
	sync
	blr

/* ------------------------------------------------------------------------------- */
/*  Function:	  out16r */
/*  Description:  Byte reverse and output 16 bits */
/* ------------------------------------------------------------------------------- */
	.globl	out16r
out16r:
	sthbrx	r4,0,r3
	sync
	blr

/* ------------------------------------------------------------------------------- */
/*  Function:	  out32 */
/*  Description:  Output 32 bits */
/* ------------------------------------------------------------------------------- */
	.globl	out32
out32:
	stw	r4,0(r3)
	sync
	blr

/* ------------------------------------------------------------------------------- */
/*  Function:	  out32r */
/*  Description:  Byte reverse and output 32 bits */
/* ------------------------------------------------------------------------------- */
	.globl	out32r
out32r:
	stwbrx	r4,0,r3
	sync
	blr

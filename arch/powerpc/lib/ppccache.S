/*
 * Copyright (C) 1998  Dan Malek <dmalek@jlc.net>
 * Copyright (C) 1999  Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 * Copyright (C) 2000, 2001,2002 Wolfgang Denk <wd@denx.de>
 * Copyright Freescale Semiconductor, Inc. 2004, 2006.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <ppc_asm.tmpl>

/*------------------------------------------------------------------------------- */
/* Function:	 ppcDcbf */
/* Description:	 Data Cache block flush */
/* Input:	 r3 = effective address */
/* Output:	 none. */
/*------------------------------------------------------------------------------- */
	.globl	ppcDcbf
ppcDcbf:
	dcbf	r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 ppcDcbi */
/* Description:	 Data Cache block Invalidate */
/* Input:	 r3 = effective address */
/* Output:	 none. */
/*------------------------------------------------------------------------------- */
	.globl	ppcDcbi
ppcDcbi:
	dcbi	r0,r3
	blr

/*--------------------------------------------------------------------------
 * Function:	 ppcDcbz
 * Description:	 Data Cache block zero.
 * Input:	 r3 = effective address
 * Output:	 none.
 *-------------------------------------------------------------------------- */

	.globl	ppcDcbz
ppcDcbz:
	dcbz	r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 ppcSync */
/* Description:	 Processor Synchronize */
/* Input:	 none. */
/* Output:	 none. */
/*------------------------------------------------------------------------------- */
	.globl	ppcSync
ppcSync:
	sync
	blr

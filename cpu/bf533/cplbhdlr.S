/* Copyright (C) 2003 Analog Devices, Inc. All Rights Reserved.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.
 *
 * Blackfin BF533/2.6 support : LG Soft India
 */


/* Include an exception handler to invoke the CPLB manager
 */

#include <asm-blackfin/linkage.h>
#include <asm/cplb.h>
#include <asm/entry.h>


.text

.globl _cplb_hdr;
.type _cplb_hdr, STT_FUNC;
.extern _cplb_mgr;
.type _cplb_mgr, STT_FUNC;
.extern __unknown_exception_occurred;
.type __unknown_exception_occurred, STT_FUNC;
.extern __cplb_miss_all_locked;
.type __cplb_miss_all_locked, STT_FUNC;
.extern __cplb_miss_without_replacement;
.type __cplb_miss_without_replacement, STT_FUNC;
.extern __cplb_protection_violation;
.type __cplb_protection_violation, STT_FUNC;
.extern panic_pv;

.align 2;

ENTRY(_cplb_hdr)
	SSYNC;
	[--SP] = ( R7:0, P5:0 );
	[--SP] = ASTAT;
	[--SP] = SEQSTAT;
	[--SP] = I0;
	[--SP] = I1;
	[--SP] = I2;
	[--SP] = I3;
	[--SP] = LT0;
	[--SP] = LB0;
	[--SP] = LC0;
	[--SP] = LT1;
	[--SP] = LB1;
	[--SP] = LC1;
	R2 = SEQSTAT;

	/*Mask the contents of SEQSTAT and leave only EXCAUSE in R2*/
	R2 <<= 26;
	R2 >>= 26;

	R1 = 0x23; /* Data access CPLB protection violation */
	CC = R2 == R1;
	IF !CC JUMP not_data_write;
	R0 = 2;		/* is a write to data space*/
	JUMP is_icplb_miss;

not_data_write:
	R1 = 0x2C; /* CPLB miss on an instruction fetch */
	CC = R2 == R1;
	R0 = 0;		/* is_data_miss == False*/
	IF CC JUMP is_icplb_miss;

	R1 = 0x26;
	CC = R2 == R1;
	IF !CC JUMP unknown;

	R0 = 1;		/* is_data_miss == True*/

is_icplb_miss:

#if ( defined (CONFIG_BLKFIN_CACHE) || defined (CONFIG_BLKFIN_DCACHE))
#if ( defined (CONFIG_BLKFIN_CACHE) && !defined (CONFIG_BLKFIN_DCACHE))
	R1 = CPLB_ENABLE_ICACHE;
#endif
#if ( !defined (CONFIG_BLKFIN_CACHE) && defined (CONFIG_BLKFIN_DCACHE))
	R1 = CPLB_ENABLE_DCACHE;
#endif
#if ( defined (CONFIG_BLKFIN_CACHE) && defined (CONFIG_BLKFIN_DCACHE))
	R1 = CPLB_ENABLE_DCACHE | CPLB_ENABLE_ICACHE;
#endif
#else
	R1 = 0;
#endif

	[--SP] = RETS;
	CALL _cplb_mgr;
	RETS = [SP++];
	CC = R0 == 0;
	IF !CC JUMP not_replaced;
	LC1 = [SP++];
	LB1 = [SP++];
	LT1 = [SP++];
	LC0 = [SP++];
	LB0 = [SP++];
	LT0 = [SP++];
	I3 = [SP++];
	I2 = [SP++];
	I1 = [SP++];
	I0 = [SP++];
	SEQSTAT = [SP++];
	ASTAT = [SP++];
	( R7:0, P5:0 ) = [SP++];
	RTS;

unknown:
	[--SP] = RETS;
	CALL __unknown_exception_occurred;
	RETS = [SP++];
	JUMP unknown;
not_replaced:
	CC = R0 == CPLB_NO_UNLOCKED;
	IF !CC JUMP next_check;
	[--SP] = RETS;
	CALL __cplb_miss_all_locked;
	RETS = [SP++];
next_check:
	CC = R0 == CPLB_NO_ADDR_MATCH;
	IF !CC JUMP next_check2;
	[--SP] = RETS;
	CALL __cplb_miss_without_replacement;
	RETS = [SP++];
	JUMP not_replaced;
next_check2:
	CC = R0 == CPLB_PROT_VIOL;
	IF !CC JUMP strange_return_from_cplb_mgr;
	[--SP] = RETS;
	CALL __cplb_protection_violation;
	RETS = [SP++];
	JUMP not_replaced;
strange_return_from_cplb_mgr:
	IDLE;
	CSYNC;
	JUMP strange_return_from_cplb_mgr;

/************************************
 * Diagnostic exception handlers
 */

__cplb_miss_all_locked:
	sp += -12;
	R0 = CPLB_NO_UNLOCKED;
	call panic_bfin;
	SP += 12;
	RTS;

 __cplb_miss_without_replacement:
	sp += -12;
	R0 = CPLB_NO_ADDR_MATCH;
	call panic_bfin;
	SP += 12;
	RTS;

__cplb_protection_violation:
	sp += -12;
	R0 = CPLB_PROT_VIOL;
	call panic_bfin;
	SP += 12;
	RTS;

__unknown_exception_occurred:

	/* This function is invoked by the default exception
	 * handler, if it does not recognise the kind of
	 * exception that has occurred. In other words, the
	 * default handler only handles some of the system's
	 * exception types, and it does not expect any others
	 * to occur. If your application is going to be using
	 * other kinds of exceptions, you must replace the
	 * default handler with your own, that handles all the
	 * exceptions you will use.
	 *
	 * Since there's nothing we can do, we just loop here
	 * at what we hope is a suitably informative label.
	 */

	IDLE;
do_not_know_what_to_do:
	CSYNC;
	JUMP __unknown_exception_occurred;

	RTS;
.__unknown_exception_occurred.end:
.global __unknown_exception_occurred;
.type __unknown_exception_occurred, STT_FUNC;

panic_bfin:
	RTS;

/*This file is subject to the terms and conditions of the GNU General Public
 * License.
 *
 * Blackfin BF533/2.6 support : LG Soft India
 * Modification: Dec 07 2004
 *	1. Correction in icheck_lock.  Valid lock entries were
 *	   geting victimized, for instruction cplb replacement.
 *	2. Setup loop's are modified as now toolchain support's P Indexed
 *	   addressing
 *	   :LG Soft India
 *
 */

/* Usage: int _cplb_mgr(is_data_miss,int enable_cache)
 * is_data_miss==2 => Mark as Dirty, write to the clean data page
 * is_data_miss==1 => Replace a data CPLB.
 * is_data_miss==0 => Replace an instruction CPLB.
 *
 * Returns:
 * CPLB_RELOADED	=> Successfully updated CPLB table.
 * CPLB_NO_UNLOCKED	=> All CPLBs are locked, so cannot be evicted.This indicates
 *				that the CPLBs in the configuration tablei are badly
 *				configured, as this should never occur.
 * CPLB_NO_ADDR_MATCH	=> The address being accessed, that triggered the exception,
 *				is not covered by any of the CPLBs in the configuration
 *				table. The application isi presumably misbehaving.
 * CPLB_PROT_VIOL	=> The address being accessed, that triggered thei exception,
 *				was not a first-write to a clean Write Back Data page,
 *				and so presumably is a genuine violation of the page's
 *				protection attributes. The application is misbehaving.
 */
#define ASSEMBLY

#include <asm-blackfin/linkage.h>
#include <asm-blackfin/blackfin.h>
#include <asm-blackfin/cplbtab.h>
#include <asm-blackfin/cplb.h>

.text

.align 2;
ENTRY(_cplb_mgr)

	[--SP]=( R7:0,P5:0 );

	CC = R0 == 2;
	IF CC JUMP dcplb_write;

	CC = R0 == 0;
	IF !CC JUMP dcplb_miss_compare;

	/* ICPLB Miss Exception. We need to choose one of the
	* currently-installed CPLBs, and replace it with one
	* from the configuration table.
	*/

	P4.L = (ICPLB_FAULT_ADDR & 0xFFFF);
	P4.H = (ICPLB_FAULT_ADDR >> 16);

	P1 = 16;
	P5.L = page_size_table;
	P5.H = page_size_table;

	P0.L = (ICPLB_DATA0 & 0xFFFF);
	P0.H = (ICPLB_DATA0 >> 16);
	R4 = [P4];		/* Get faulting address*/
	R6 = 64;		/* Advance past the fault address, which*/
	R6 = R6 + R4;		/* we'll use if we find a match*/
	R3 = ((16 << 8) | 2);	/* Extract mask, bits 16 and 17.*/

	R5 = 0;
isearch:

	R1 = [P0-0x100];	/* Address for this CPLB */

	R0 = [P0++];		/* Info for this CPLB*/
	CC = BITTST(R0,0);	/* Is the CPLB valid?*/
	IF !CC JUMP nomatch;	/* Skip it, if not.*/
	CC = R4 < R1(IU);	/* If fault address less than page start*/
	IF CC JUMP nomatch;	/* then skip this one.*/
	R2 = EXTRACT(R0,R3.L) (Z);	/* Get page size*/
	P1 = R2;
	P1 = P5 + (P1<<2);	/* index into page-size table*/
	R2 = [P1];		/* Get the page size*/
	R1 = R1 + R2;		/* and add to page start, to get page end*/
	CC = R4 < R1(IU);	/* and see whether fault addr is in page.*/
	IF !CC R4 = R6;		/* If so, advance the address and finish loop.*/
	IF !CC JUMP isearch_done;
nomatch:
	/* Go around again*/
	R5 += 1;
	CC = BITTST(R5, 4);	/* i.e CC = R5 >= 16*/
	IF !CC JUMP isearch;

isearch_done:
	I0 = R4;		/* Fault address we'll search for*/

	/* set up pointers */
	P0.L = (ICPLB_DATA0 & 0xFFFF);
	P0.H = (ICPLB_DATA0 >> 16);

	/* The replacement procedure for ICPLBs */

	P4.L = (IMEM_CONTROL & 0xFFFF);
	P4.H = (IMEM_CONTROL >> 16);

	/* disable cplbs */
	R5 = [P4];		/* Control Register*/
	BITCLR(R5,ENICPLB_P);
	CLI R1;
	SSYNC;		/* SSYNC required before writing to IMEM_CONTROL. */
	.align 8;
	[P4] = R5;
	SSYNC;
	STI R1;

	R1 = -1;		/* end point comparison */
	R3 = 16;		/* counter */

	/* Search through CPLBs for first non-locked entry */
	/* Overwrite it by moving everyone else up by 1 */
icheck_lock:
	R0 = [P0++];
	R3 = R3 + R1;
	CC = R3 == R1;
	IF CC JUMP all_locked;
	CC = BITTST(R0, 0);		/* an invalid entry is good */
	IF !CC JUMP ifound_victim;
	CC = BITTST(R0,1);		/* but a locked entry isn't */
	IF CC JUMP icheck_lock;

ifound_victim:
#ifdef CONFIG_CPLB_INFO
	R7 = [P0 - 0x104];
	P2.L = ipdt_table;
	P2.H = ipdt_table;
	P3.L = ipdt_swapcount_table;
	P3.H = ipdt_swapcount_table;
	P3 += -4;
icount:
	R2 = [P2];	/* address from config table */
	P2 += 8;
	P3 += 8;
	CC = R2==-1;
	IF CC JUMP icount_done;
	CC = R7==R2;
	IF !CC JUMP icount;
	R7 = [P3];
	R7 += 1;
	[P3] = R7;
	CSYNC;
icount_done:
#endif
	LC0=R3;
	LSETUP(is_move,ie_move) LC0;
is_move:
	R0 = [P0];
	[P0 - 4] = R0;
	R0 = [P0 - 0x100];
	[P0-0x104] = R0;
ie_move:P0+=4;

	/* We've made space in the ICPLB table, so that ICPLB15
	 * is now free to be overwritten. Next, we have to determine
	 * which CPLB we need to install, from the configuration
	 * table. This is a matter of getting the start-of-page
	 * addresses and page-lengths from the config table, and
	 * determining whether the fault address falls within that
	 * range.
	 */

	P2.L = ipdt_table;
	P2.H = ipdt_table;
#ifdef	CONFIG_CPLB_INFO
	P3.L = ipdt_swapcount_table;
	P3.H = ipdt_swapcount_table;
	P3 += -8;
#endif
	P0.L = page_size_table;
	P0.H = page_size_table;

	/* Retrieve our fault address (which may have been advanced
	 * because the faulting instruction crossed a page boundary).
	 */

	R0 = I0;

	/* An extraction pattern, to get the page-size bits from
	 * the CPLB data entry. Bits 16-17, so two bits at posn 16.
	 */

	R1 = ((16<<8)|2);
inext:	R4 = [P2++];	/* address from config table */
	R2 = [P2++];	/* data from config table */
#ifdef	CONFIG_CPLB_INFO
	P3 += 8;
#endif

	CC = R4 == -1;	/* End of config table*/
	IF CC JUMP no_page_in_table;

	/* See if failed address > start address */
	CC = R4 <= R0(IU);
	IF !CC JUMP inext;

	/* extract page size (17:16)*/
	R3 = EXTRACT(R2, R1.L) (Z);

	/* add page size to addr to get range */

	P5 = R3;
	P5 = P0 + (P5 << 2);	/* scaled, for int access*/
	R3 = [P5];
	R3 = R3 + R4;

	/* See if failed address < (start address + page size) */
	CC = R0 < R3(IU);
	IF !CC JUMP inext;

	/* We've found a CPLB in the config table that covers
	 * the faulting address, so install this CPLB into the
	 * last entry of the table.
	 */

	P1.L = (ICPLB_DATA15 & 0xFFFF);		/*ICPLB_DATA15*/
	P1.H = (ICPLB_DATA15 >> 16);
	[P1] = R2;
	[P1-0x100] = R4;
#ifdef	CONFIG_CPLB_INFO
	R3 = [P3];
	R3 += 1;
	[P3] = R3;
#endif

	/* P4 points to IMEM_CONTROL, and R5 contains its old
	 * value, after we disabled ICPLBS. Re-enable them.
	 */

	BITSET(R5,ENICPLB_P);
	CLI R2;
	SSYNC;		/* SSYNC required before writing to IMEM_CONTROL. */
	.align 8;
	[P4] = R5;
	SSYNC;
	STI R2;

	( R7:0,P5:0 ) = [SP++];
	R0 = CPLB_RELOADED;
	RTS;

/* FAILED CASES*/
no_page_in_table:
	( R7:0,P5:0 ) = [SP++];
	R0 = CPLB_NO_ADDR_MATCH;
	RTS;
all_locked:
	( R7:0,P5:0 ) = [SP++];
	R0 = CPLB_NO_UNLOCKED;
	RTS;
prot_violation:
	( R7:0,P5:0 ) = [SP++];
	R0 = CPLB_PROT_VIOL;
	RTS;

dcplb_write:

	/* if a DCPLB is marked as write-back (CPLB_WT==0), and
	 * it is clean (CPLB_DIRTY==0), then a write to the
	 * CPLB's page triggers a protection violation. We have to
	 * mark the CPLB as dirty, to indicate that there are
	 * pending writes associated with the CPLB.
	 */

	P4.L = (DCPLB_STATUS & 0xFFFF);
	P4.H = (DCPLB_STATUS >> 16);
	P3.L = (DCPLB_DATA0 & 0xFFFF);
	P3.H = (DCPLB_DATA0 >> 16);
	R5 = [P4];

	/* A protection violation can be caused by more than just writes
	 * to a clean WB page, so we have to ensure that:
	 * - It's a write
	 * - to a clean WB page
	 * - and is allowed in the mode the access occurred.
	 */

	CC = BITTST(R5, 16);	/* ensure it was a write*/
	IF !CC JUMP prot_violation;

	/* to check the rest, we have to retrieve the DCPLB.*/

	/* The low half of DCPLB_STATUS is a bit mask*/

	R2 = R5.L (Z);	/* indicating which CPLB triggered the event.*/
	R3 = 30;	/* so we can use this to determine the offset*/
	R2.L = SIGNBITS R2;
	R2 = R2.L (Z);	/* into the DCPLB table.*/
	R3 = R3 - R2;
	P4 = R3;
	P3 = P3 + (P4<<2);
	R3 = [P3];	/* Retrieve the CPLB*/

	/* Now we can check whether it's a clean WB page*/

	CC = BITTST(R3, 14);	/* 0==WB, 1==WT*/
	IF CC JUMP prot_violation;
	CC = BITTST(R3, 7);	/* 0 == clean, 1 == dirty*/
	IF CC JUMP prot_violation;

	/* Check whether the write is allowed in the mode that was active.*/

	R2 = 1<<3;		/* checking write in user mode*/
	CC = BITTST(R5, 17);	/* 0==was user, 1==was super*/
	R5 = CC;
	R2 <<= R5;		/* if was super, check write in super mode*/
	R2 = R3 & R2;
	CC = R2 == 0;
	IF CC JUMP prot_violation;

	/* It's a genuine write-to-clean-page.*/

	BITSET(R3, 7);		/* mark as dirty*/
	[P3] = R3;		/* and write back.*/
	CSYNC;
	( R7:0,P5:0 ) = [SP++];
	R0 = CPLB_RELOADED;
	RTS;

dcplb_miss_compare:

	/* Data CPLB Miss event. We need to choose a CPLB to
	 * evict, and then locate a new CPLB to install from the
	 * config table, that covers the faulting address.
	 */

	P1.L = (DCPLB_DATA15 & 0xFFFF);
	P1.H = (DCPLB_DATA15 >> 16);

	P4.L = (DCPLB_FAULT_ADDR & 0xFFFF);
	P4.H = (DCPLB_FAULT_ADDR >> 16);
	R4 = [P4];
	I0 = R4;

	/* The replacement procedure for DCPLBs*/

	R6 = R1;	/* Save for later*/

	/* Turn off CPLBs while we work.*/
	P4.L = (DMEM_CONTROL & 0xFFFF);
	P4.H = (DMEM_CONTROL >> 16);
	R5 = [P4];
	BITCLR(R5,ENDCPLB_P);
	CLI R0;
	SSYNC;		/* SSYNC required before writing to DMEM_CONTROL. */
	.align 8;
	[P4] = R5;
	SSYNC;
	STI R0;

	/* Start looking for a CPLB to evict. Our order of preference
	 * is: invalid CPLBs, clean CPLBs, dirty CPLBs. Locked CPLBs
	 * are no good.
	 */

	I1.L = (DCPLB_DATA0 & 0xFFFF);
	I1.H = (DCPLB_DATA0 >> 16);
	P1 = 3;
	P2 = 16;
	I2.L = dcplb_preference;
	I2.H = dcplb_preference;
	LSETUP(sdsearch1, edsearch1) LC0 = P1;
sdsearch1:
	R0 = [I2++];		/* Get the bits we're interested in*/
	P0 = I1;		/* Go back to start of table*/
	LSETUP (sdsearch2, edsearch2) LC1 = P2;
sdsearch2:
	R1 = [P0++];		/* Fetch each installed CPLB in turn*/
	R2 = R1 & R0;		/* and test for interesting bits.*/
	CC = R2 == 0;		/* If none are set, it'll do.*/
	IF !CC JUMP skip_stack_check;

	R2 = [P0 - 0x104];	/* R2 - PageStart */
	P3.L = page_size_table; /* retrive end address */
	P3.H = page_size_table; /* retrive end address */
	R3 = 0x2;		/* 0th - position, 2 bits -length */
	nop;			/*Anamoly 05000209*/
	R7 = EXTRACT(R1,R3.l);
	R7 = R7 << 2;		/* Page size index offset */
	P5 = R7;
	P3 = P3 + P5;
	R7 = [P3];		/* page size in 1K bytes */

	R7 = R7 << 0xA;		/* in bytes * 1024*/
	R7 = R2 + R7;		/* R7 - PageEnd */
	R4 = SP;		/* Test SP is in range */

	CC = R7 < R4;		/* if PageEnd < SP */
	IF CC JUMP dfound_victim;
	R3 = 0x284;		/* stack length from start of trap till the point */
				/* 20 stack locations for future modifications */
	R4 = R4 + R3;
	CC = R4 < R2;		/* if SP + stacklen < PageStart */
	IF CC JUMP dfound_victim;
skip_stack_check:

edsearch2: NOP;
edsearch1: NOP;

	/* If we got here, we didn't find a DCPLB we considered
	 * replacable, which means all of them were locked.
	 */

	JUMP all_locked;
dfound_victim:

#ifdef CONFIG_CPLB_INFO
	R1 = [P0 - 0x104];
	P2.L = dpdt_table;
	P2.H = dpdt_table;
	P3.L = dpdt_swapcount_table;
	P3.H = dpdt_swapcount_table;
	P3 += -4;
dicount:
	R2 = [P2];
	P2 += 8;
	P3 += 8;
	CC = R2==-1;
	IF CC JUMP dicount_done;
	CC = R1==R2;
	IF !CC JUMP dicount;
	R1 = [P3];
	R1 += 1;
	[P3] = R1;
	CSYNC;
dicount_done:
#endif

	/* Clean down the hardware loops*/
	R2 = 0;
	LC1 = R2;
	LC0 = R2;

	/* There's a suitable victim in [P0-4] (because we've
	 * advanced already). If it's a valid dirty write-back
	 * CPLB, we need to flush the pending writes first.
	 */

	CC = BITTST(R1, 0);	/* Is it valid?*/
	IF !CC JUMP Ddoverwrite;/* nope.*/
	CC = BITTST(R1, 7);	/* Is it dirty?*/
	IF !CC JUMP Ddoverwrite (BP);	/* Nope.*/
	CC = BITTST(R1, 14);	/* Is it Write-Through?*/
	IF CC JUMP Ddoverwrite; /* Yep*/

	/* This is a dirty page, so we need to flush all writes
	 * that are pending on the page.
	 */

	/* Retrieve the page start address*/
	R0 = [P0 - 0x104];
	[--sp] = rets;
	CALL dcplb_flush;	/* R0==CPLB addr, R1==CPLB data*/
	rets = [sp++];
Ddoverwrite:

	/* [P0-4] is a suitable victim CPLB, so we want to
	 * overwrite it by moving all the following CPLBs
	 * one space closer to the start.
	 */

	R1.L = ((DCPLB_DATA15+4) & 0xFFFF);		/*DCPLB_DATA15+4*/
	R1.H = ((DCPLB_DATA15+4) >> 16);
	R0 = P0;

	/* If the victim happens to be in DCPLB15,
	 * we don't need to move anything.
	 */

	CC = R1 == R0;
	IF CC JUMP de_moved;
	R1 = R1 - R0;
	R1 >>= 2;
	P1 = R1;
	LSETUP(ds_move, de_move) LC0=P1;
ds_move:
	 R0 = [P0++];	/* move data */
	[P0 - 8] = R0;
	R0 = [P0-0x104] /* move address */
de_move: [P0-0x108] = R0;

	/* We've now made space in DCPLB15 for the new CPLB to be
	 * installed. The next stage is to locate a CPLB in the
	 * config table that covers the faulting address.
	 */

de_moved:NOP;
	R0 = I0;		/* Our faulting address */

	P2.L = dpdt_table;
	P2.H = dpdt_table;
#ifdef	CONFIG_CPLB_INFO
	P3.L = dpdt_swapcount_table;
	P3.H = dpdt_swapcount_table;
	P3 += -8;
#endif

	P1.L = page_size_table;
	P1.H = page_size_table;

	/* An extraction pattern, to retrieve bits 17:16.*/

	R1 = (16<<8)|2;
dnext:	R4 = [P2++];	/* address */
	R2 = [P2++];	/* data */
#ifdef	CONFIG_CPLB_INFO
	P3 += 8;
#endif

	CC = R4 == -1;
	IF CC JUMP no_page_in_table;

	/* See if failed address > start address */
	CC = R4 <= R0(IU);
	IF !CC JUMP dnext;

	/* extract page size (17:16)*/
	R3 = EXTRACT(R2, R1.L) (Z);

	/* add page size to addr to get range */

	P5 = R3;
	P5 = P1 + (P5 << 2);
	R3 = [P5];
	R3 = R3 + R4;

	/* See if failed address < (start address + page size) */
	CC = R0 < R3(IU);
	IF !CC JUMP dnext;

	/* We've found the CPLB that should be installed, so
	 * write it into CPLB15, masking off any caching bits
	 * if necessary.
	 */

	P1.L = (DCPLB_DATA15 & 0xFFFF);
	P1.H = (DCPLB_DATA15 >> 16);

	/* If the DCPLB has cache bits set, but caching hasn't
	 * been enabled, then we want to mask off the cache-in-L1
	 * bit before installing. Moreover, if caching is off, we
	 * also want to ensure that the DCPLB has WT mode set, rather
	 * than WB, since WB pages still trigger first-write exceptions
	 * even when not caching is off, and the page isn't marked as
	 * cachable. Finally, we could mark the page as clean, not dirty,
	 * but we choose to leave that decision to the user; if the user
	 * chooses to have a CPLB pre-defined as dirty, then they always
	 * pay the cost of flushing during eviction, but don't pay the
	 * cost of first-write exceptions to mark the page as dirty.
	 */

#ifdef CONFIG_BLKFIN_WT
	BITSET(R6, 14);		/* Set WT*/
#endif

	[P1] = R2;
	[P1-0x100] = R4;
#ifdef	CONFIG_CPLB_INFO
	R3 = [P3];
	R3 += 1;
	[P3] = R3;
#endif

	/* We've installed the CPLB, so re-enable CPLBs. P4
	 * points to DMEM_CONTROL, and R5 is the value we
	 * last wrote to it, when we were disabling CPLBs.
	 */

	BITSET(R5,ENDCPLB_P);
	CLI R2;
	.align 8;
	[P4] = R5;
	SSYNC;
	STI R2;

	( R7:0,P5:0 ) = [SP++];
	R0 = CPLB_RELOADED;
	RTS;

.data
.align 4;
page_size_table:
.byte4	0x00000400;	/* 1K */
.byte4	0x00001000;	/* 4K */
.byte4	0x00100000;	/* 1M */
.byte4	0x00400000;	/* 4M */

.align 4;
dcplb_preference:
.byte4	0x00000001;	/* valid bit */
.byte4	0x00000082;	/* dirty+lock bits */
.byte4	0x00000002;	/* lock bit */

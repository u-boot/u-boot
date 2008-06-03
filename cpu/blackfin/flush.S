/* flush.S - low level cache flushing routines
 * Copyright (C) 2003-2007 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

#include <config.h>
#include <asm/blackfin.h>
#include <asm/cplb.h>
#include <asm/mach-common/bits/mpu.h>

.text

/* This is an external function being called by the user
 * application through __flush_cache_all. Currently this function
 * serves the purpose of flushing all the pending writes in
 * in the data cache.
 */

ENTRY(_flush_data_cache)
	[--SP] = ( R7:6, P5:4 );
	LINK 12;
	SP += -12;
	P5.H = HI(DCPLB_ADDR0);
	P5.L = LO(DCPLB_ADDR0);
	P4.H = HI(DCPLB_DATA0);
	P4.L = LO(DCPLB_DATA0);
	R7 = CPLB_VALID | CPLB_L1_CHBL | CPLB_DIRTY (Z);
	R6 = 16;
.Lnext:	R0 = [P5++];
	R1 = [P4++];
	CC = BITTST(R1, 14);	/* Is it write-through?*/
	IF CC JUMP .Lskip;	/* If so, ignore it.*/
	R2 = R1 & R7;		/* Is it a dirty, cached page?*/
	CC = R2;
	IF !CC JUMP .Lskip;	/* If not, ignore it.*/
	[--SP] = RETS;
	CALL _dcplb_flush;	/* R0 = page, R1 = data*/
	RETS = [SP++];
.Lskip:	R6 += -1;
	CC = R6;
	IF CC JUMP .Lnext;
	SSYNC;
	SP += 12;
	UNLINK;
	( R7:6, P5:4 ) = [SP++];
	RTS;
ENDPROC(_flush_data_cache)

/* This is an internal function to flush all pending
 * writes in the cache associated with a particular DCPLB.
 *
 * R0 -  page's start address
 * R1 -  CPLB's data field.
 */

.align 2
ENTRY(_dcplb_flush)
	[--SP] = ( R7:0, P5:0 );
	[--SP] = LC0;
	[--SP] = LT0;
	[--SP] = LB0;
	[--SP] = LC1;
	[--SP] = LT1;
	[--SP] = LB1;

	/* If it's a 1K or 4K page, then it's quickest to
	 * just systematically flush all the addresses in
	 * the page, regardless of whether they're in the
	 * cache, or dirty. If it's a 1M or 4M page, there
	 * are too many addresses, and we have to search the
	 * cache for lines corresponding to the page.
	 */

	CC = BITTST(R1, 17);	/* 1MB or 4MB */
	IF !CC JUMP .Ldflush_whole_page;

	/* We're only interested in the page's size, so extract
	 * this from the CPLB (bits 17:16), and scale to give an
	 * offset into the page_size and page_prefix tables.
	 */

	R1 <<= 14;
	R1 >>= 30;
	R1 <<= 2;

	/* The page could be mapped into Bank A or Bank B, depending
	 * on (a) whether both banks are configured as cache, and
	 * (b) on whether address bit A[x] is set. x is determined
	 * by DCBS in DMEM_CONTROL
	 */

	R2 = 0;			/* Default to Bank A (Bank B would be 1)*/

	P0.L = LO(DMEM_CONTROL);
	P0.H = HI(DMEM_CONTROL);

	R3 = [P0];		/* If Bank B is not enabled as cache*/
	CC = BITTST(R3, 2);	/* then Bank A is our only option.*/
	IF CC JUMP .Lbank_chosen;

	R4 = 1<<14;		/* If DCBS==0, use A[14].*/
	R5 = R4 << 7;		/* If DCBS==1, use A[23];*/
	CC = BITTST(R3, 4);
	IF CC R4 = R5;		/* R4 now has either bit 14 or bit 23 set.*/
	R5 = R0 & R4;		/* Use it to test the Page address*/
	CC = R5;		/* and if that bit is set, we use Bank B,*/
	R2 = CC;		/* else we use Bank A.*/
	R2 <<= 23;		/* The Bank selection's at posn 23.*/

.Lbank_chosen:

	/* We can also determine the sub-bank used, because this is
	 * taken from bits 13:12 of the address.
	 */

	R3 = ((12<<8)|2);		/* Extraction pattern */
	nop;				/*Anamoly 05000209*/
	R4 = EXTRACT(R0, R3.L) (Z);	/* Extract bits*/
	/* Save in extraction pattern for later deposit.*/
	R3.H = R4.L << 0;

	/* So:
	 * R0 = Page start
	 * R1 = Page length (actually, offset into size/prefix tables)
	 * R2 = Bank select mask
	 * R3 = sub-bank deposit values
	 *
	 * The cache has 2 Ways, and 64 sets, so we iterate through
	 * the sets, accessing the tag for each Way, for our Bank and
	 * sub-bank, looking for dirty, valid tags that match our
	 * address prefix.
	 */

	P5.L = LO(DTEST_COMMAND);
	P5.H = HI(DTEST_COMMAND);
	P4.L = LO(DTEST_DATA0);
	P4.H = HI(DTEST_DATA0);

	P0.L = page_prefix_table;
	P0.H = page_prefix_table;
	P1 = R1;
	R5 = 0;			/* Set counter*/
	P0 = P1 + P0;
	R4 = [P0];		/* This is the address prefix*/


	/* We're reading (bit 1==0) the tag (bit 2==0), and we
	 * don't care about which double-word, since we're only
	 * fetching tags, so we only have to set Set, Bank,
	 * Sub-bank and Way.
	 */

	P2 = 2;
	LSETUP (.Lfs1, .Lfe1) LC1 = P2;
.Lfs1:	P0 = 64;		/* iterate over all sets*/
	LSETUP (.Lfs0, .Lfe0) LC0 = P0;
.Lfs0:	R6 = R5 << 5;		/* Combine set*/
	R6.H = R3.H << 0 ;	/* and sub-bank*/
	R6 = R6 | R2;		/* and Bank. Leave Way==0 at first.*/
	BITSET(R6,14);
	[P5] = R6;		/* Issue Command*/
	SSYNC;
	R7 = [P4];		/* and read Tag.*/
	CC = BITTST(R7, 0);	/* Check if valid*/
	IF !CC JUMP .Lfskip;	/* and skip if not.*/
	CC = BITTST(R7, 1);	/* Check if dirty*/
	IF !CC JUMP .Lfskip;	/* and skip if not.*/

	/* Compare against the page address. First, plant bits 13:12
	 * into the tag, since those aren't part of the returned data.
	 */

	R7 = DEPOSIT(R7, R3);	/* set 13:12*/
	R1 = R7 & R4;		/* Mask off lower bits*/
	CC = R1 == R0;		/* Compare against page start.*/
	IF !CC JUMP .Lfskip;	/* Skip it if it doesn't match.*/

	/* Tag address matches against page, so this is an entry
	 * we must flush.
	 */

	R7 >>= 10;		/* Mask off the non-address bits*/
	R7 <<= 10;
	P3 = R7;
	SSYNC;
	FLUSHINV [P3];		/* And flush the entry*/
.Lfskip:
.Lfe0:	R5 += 1;		/* Advance to next Set*/
.Lfe1:	BITSET(R2, 26);		/* Go to next Way.*/

.Ldfinished:
	SSYNC;			/* Ensure the data gets out to mem.*/

	/*Finished. Restore context.*/
	LB1 = [SP++];
	LT1 = [SP++];
	LC1 = [SP++];
	LB0 = [SP++];
	LT0 = [SP++];
	LC0 = [SP++];
	( R7:0, P5:0 ) = [SP++];
	RTS;

.Ldflush_whole_page:

	/* It's a 1K or 4K page, so quicker to just flush the
	 * entire page.
	 */

	P1 = 32;		/* For 1K pages*/
	P2 = P1 << 2;		/* For 4K pages*/
	P0 = R0;		/* Start of page*/
	CC = BITTST(R1, 16);	/* Whether 1K or 4K*/
	IF CC P1 = P2;
	P1 += -1;		/* Unroll one iteration*/
	SSYNC;
	FLUSHINV [P0++];	/* because CSYNC can't end loops.*/
	LSETUP (.Leall, .Leall) LC0 = P1;
.Leall:	FLUSHINV [P0++];
	SSYNC;
	JUMP .Ldfinished;
ENDPROC(_dcplb_flush)

.align 4;
page_prefix_table:
.byte4	0xFFFFFC00;	/* 1K */
.byte4	0xFFFFF000;	/* 4K */
.byte4	0xFFF00000;	/* 1M */
.byte4	0xFFC00000;	/* 4M */
.page_prefix_table.end:

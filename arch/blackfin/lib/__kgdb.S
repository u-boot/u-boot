
#include <linux/linkage.h>

/* save stack context for non-local goto
 * int kgdb_setjmp(long *buf)
 */

ENTRY(_kgdb_setjmp)
	[--SP] = p0; 	/* Save P0 */
	p0 = r0;
	r0 = [SP++];	/* Load P0 into R0 */

	[p0 + 0x00] = r0;       /* GP address registers */
	[p0 + 0x04] = p1;
	[p0 + 0x08] = p2;
	[p0 + 0x0C] = p3;
	[p0 + 0x10] = p4;
	[p0 + 0x14] = p5;
	[p0 + 0x18] = FP;       /* frame pointer */
	[p0 + 0x1C] = SP;       /* stack pointer */

	[p0 + 0x20] = p0;       /* data regs */
	[p0 + 0x24] = r1;
	[p0 + 0x28] = r2;
	[p0 + 0x2C] = r3;
	[p0 + 0x30] = r4;
	[p0 + 0x34] = r5;
	[p0 + 0x38] = r6;
	[p0 + 0x3C] = r7;

	r0 = ASTAT;	[p0 + 0x40] = r0;

	/* loop counters */
	r0 = LC0;	[p0 + 0x44] = r0;
	r0 = LC1;	[p0 + 0x48] = r0;

	/* Accumulator */
	r0 = A0.w;	[p0 + 0x4C] = r0;
	r0.l = A0.x;	[p0 + 0x50] = r0;
	r0 = A1.w;	[p0 + 0x54] = r0;
	r0.l = A1.x;	[p0 + 0x58] = r0;

	/* index registers */
	r0 = i0;	[p0 + 0x5C] = r0;
	r0 = i1;	[p0 + 0x60] = r0;
	r0 = i2;	[p0 + 0x64] = r0;
	r0 = i3;	[p0 + 0x68] = r0;

	/* modifier registers */
	r0 = m0;	[p0 + 0x6C] = r0;
	r0 = m1;	[p0 + 0x70] = r0;
	r0 = m2;	[p0 + 0x74] = r0;
	r0 = m3;	[p0 + 0x78] = r0;

	/* length registers */
	r0 = l0;	[p0 + 0x7C] = r0;
	r0 = l1;	[p0 + 0x80] = r0;
	r0 = l2;	[p0 + 0x84] = r0;
	r0 = l3;	[p0 + 0x88] = r0;

	/* base registers */
	r0 = b0;	[p0 + 0x8C] = r0;
	r0 = b1;	[p0 + 0x90] = r0;
	r0 = b2;	[p0 + 0x94] = r0;
	r0 = b3;	[p0 + 0x98] = r0;

	/* store return address */
	r0 = RETS;	[p0 + 0x9C] = r0;

	R0 = 0;
	RTS;
ENDPROC(_kgdb_setjmp)

/*
 * non-local jump to a saved stack context
 * longjmp(long *buf, int val)
 */

ENTRY(_kgdb_longjmp)
	p0 = r0;
	r0 = [p0 + 0x00];
	[--sp] = r0;

	/* GP address registers - skip p0 for now*/
	p1 = [p0 + 0x04];
	p2 = [p0 + 0x08];
	p3 = [p0 + 0x0C];
	p4 = [p0 + 0x10];
	p5 = [p0 + 0x14];
	/* frame pointer */
	fp = [p0 + 0x18];
	/* stack pointer */
	r0 = [sp++];
	sp = [p0 + 0x1C];
	[--sp] = r0;
	[--sp] = r1;

	/* data regs */
	r0 = [p0 + 0x20];
	r1 = [p0 + 0x24];
	r2 = [p0 + 0x28];
	r3 = [p0 + 0x2C];
	r4 = [p0 + 0x30];
	r5 = [p0 + 0x34];
	r6 = [p0 + 0x38];
	r7 = [p0 + 0x3C];

	r0 = [p0 + 0x40];	ASTAT = r0;

	/* loop counters */
	r0 = [p0 + 0x44];	LC0 = r0;
	r0 = [p0 + 0x48];	LC1 = r0;

	/* Accumulator */
	r0 = [p0 + 0x4C];	A0.w = r0;
	r0 = [p0 + 0x50];	A0.x = r0;
	r0 = [p0 + 0x54];	A1.w = r0;
	r0 = [p0 + 0x58];	A1.x = r0;

	/* index registers */
	r0 = [p0 + 0x5C];	i0 = r0;
	r0 = [p0 + 0x60];	i1 = r0;
	r0 = [p0 + 0x64];	i2 = r0;
	r0 = [p0 + 0x68];	i3 = r0;

	/* modifier registers */
	r0 = [p0 + 0x6C];	m0 = r0;
	r0 = [p0 + 0x70];	m1 = r0;
	r0 = [p0 + 0x74];	m2 = r0;
	r0 = [p0 + 0x78];	m3 = r0;

	/* length registers */
	r0 = [p0 + 0x7C];	l0 = r0;
	r0 = [p0 + 0x80];	l1 = r0;
	r0 = [p0 + 0x84];	l2 = r0;
	r0 = [p0 + 0x88];	l3 = r0;

	/* base registers */
	r0 = [p0 + 0x8C];	b0 = r0;
	r0 = [p0 + 0x90];	b1 = r0;
	r0 = [p0 + 0x94];	b2 = r0;
	r0 = [p0 + 0x98];	b3 = r0;

	/* store return address */
	r0 = [p0 + 0x9C];	RETS = r0;

	/* fixup R0 & P0 */
	r0 = [sp++];
	p0 = [sp++];
	CC = R0 == 0;
	IF !CC JUMP .Lfinished;
	R0 = 1;
.Lfinished:
	RTS;
ENDPROC(_kgdb_longjmp)

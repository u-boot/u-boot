/* cache.S - low level cache handling routines
 * Copyright (C) 2003-2007 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

#include <asm/linkage.h>
#include <config.h>
#include <asm/blackfin.h>

.text
.align 2
ENTRY(_blackfin_icache_flush_range)
	R2 = -32;
	R2 = R0 & R2;
	P0 = R2;
	P1 = R1;
	CSYNC;
1:
	IFLUSH[P0++];
	CC = P0 < P1(iu);
	IF CC JUMP 1b(bp);
	IFLUSH[P0];
	SSYNC;
	RTS;
ENDPROC(_blackfin_icache_flush_range)

ENTRY(_blackfin_dcache_flush_range)
	R2 = -32;
	R2 = R0 & R2;
	P0 = R2;
	P1 = R1;
	CSYNC;
1:
	FLUSH[P0++];
	CC = P0 < P1(iu);
	IF CC JUMP 1b(bp);
	FLUSH[P0];
	SSYNC;
	RTS;
ENDPROC(_blackfin_dcache_flush_range)

ENTRY(_blackfin_dcache_invalidate_range)
	R2 = -32;
	R2 = R0 & R2;
	P0 = R2;
	P1 = R1;
	CSYNC;
1:
	FLUSHINV[P0++];
	CC = P0 < P1(iu);
	IF CC JUMP 1b(bp);

	/*
	 * If the data crosses a cache line, then we'll be pointing to
	 * the last cache line, but won't have flushed/invalidated it yet, so do
	 * one more.
	 */
	FLUSHINV[P0];
	SSYNC;
	RTS;
ENDPROC(_blackfin_dcache_invalidate_range)

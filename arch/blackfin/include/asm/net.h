/*
 * net.h - misc Blackfin network helpers
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __ASM_BFIN_RAND_MAC__
#define __ASM_BFIN_RAND_MAC__

/* If the board does not have a real MAC assigned to it, then generate a
 * locally administrated pseudo-random one based on CYCLES and compile date.
 */
static inline void bfin_gen_rand_mac(uchar *mac_addr)
{
	/* make something up */
	const char s[] = __DATE__;
	size_t i;
	u32 cycles;
	for (i = 0; i < 6; ++i) {
		asm("%0 = CYCLES;" : "=r" (cycles));
		mac_addr[i] = cycles ^ s[i];
	}
	mac_addr[0] = (mac_addr[0] | 0x02) & ~0x01; /* make it local unicast */
}

#endif

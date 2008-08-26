/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#include <common.h>
#include <ddr_spd.h>

/* used for ddr1 and ddr2 spd */
static int
spd_check(const u8 *buf, u8 spd_rev, u8 spd_cksum)
{
	unsigned int cksum = 0;
	unsigned int i;

	/*
	 * Check SPD revision supported
	 * Rev 1.2 or less supported by this code
	 */
	if (spd_rev > 0x12) {
		printf("SPD revision %02X not supported by this code\n",
		       spd_rev);
		return 1;
	}

	/*
	 * Calculate checksum
	 */
	for (i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	if (cksum != spd_cksum) {
		printf("SPD checksum unexpected. "
			"Checksum in SPD = %02X, computed SPD = %02X\n",
			spd_cksum, cksum);
		return 1;
	}

	return 0;
}

unsigned int
ddr1_spd_check(const ddr1_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	return spd_check(p, spd->spd_rev, spd->cksum);
}

unsigned int
ddr2_spd_check(const ddr2_spd_eeprom_t *spd)
{
	const u8 *p = (const u8 *)spd;

	return spd_check(p, spd->spd_rev, spd->cksum);
}

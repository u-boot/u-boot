/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#ifndef COMMON_TIMING_PARAMS_H
#define COMMON_TIMING_PARAMS_H

typedef struct {
	/* parameters to constrict */

	unsigned int tCKmin_X_ps;
	unsigned int tCKmax_ps;
	unsigned int tCKmax_max_ps;
	unsigned int tRCD_ps;
	unsigned int tRP_ps;
	unsigned int tRAS_ps;

	unsigned int tWR_ps;	/* maximum = 63750 ps */
	unsigned int tWTR_ps;	/* maximum = 63750 ps */
	unsigned int tRFC_ps;	/* maximum = 255 ns + 256 ns + .75 ns
					   = 511750 ps */

	unsigned int tRRD_ps;	/* maximum = 63750 ps */
	unsigned int tRC_ps;	/* maximum = 254 ns + .75 ns = 254750 ps */

	unsigned int refresh_rate_ps;

	unsigned int tIS_ps;	/* byte 32, spd->ca_setup */
	unsigned int tIH_ps;	/* byte 33, spd->ca_hold */
	unsigned int tDS_ps;	/* byte 34, spd->data_setup */
	unsigned int tDH_ps;	/* byte 35, spd->data_hold */
	unsigned int tRTP_ps;	/* byte 38, spd->trtp */
	unsigned int tDQSQ_max_ps;	/* byte 44, spd->tdqsq */
	unsigned int tQHS_ps;	/* byte 45, spd->tqhs */

	unsigned int ndimms_present;
	unsigned int lowest_common_SPD_caslat;
	unsigned int highest_common_derated_caslat;
	unsigned int additive_latency;
	unsigned int all_DIMMs_burst_lengths_bitmask;
	unsigned int all_DIMMs_registered;
	unsigned int all_DIMMs_unbuffered;
	unsigned int all_DIMMs_ECC_capable;

	unsigned long long total_mem;
	unsigned long long base_address;
} common_timing_params_t;

#endif

/*
 * Copyright (C) 2015 Marek Vasut <marex@denx.de>
 *
 * FIXME: This file contains temporary stub functions and is here
 *        only until these functions are properly merged into
 *        mainline.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_SDRAM_H__
#define __ARCH_SDRAM_H__

/* function declaration */
inline unsigned long sdram_calculate_size(void) { return 0; }
inline unsigned sdram_mmr_init_full(unsigned int sdr_phy_reg) { return 0; }
inline int sdram_calibration_full(void) { return 0; }

#endif	/* __ARCH_SDRAM_H__ */

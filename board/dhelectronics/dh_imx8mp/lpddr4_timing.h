/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#ifndef __LPDDR4_TIMING_H__
#define __LPDDR4_TIMING_H__

extern struct dram_timing_info dh_imx8mp_dhcom_dram_timing_32g_x32;

u8 dh_get_memcfg(void);

#endif /* __LPDDR4_TIMING_H__ */

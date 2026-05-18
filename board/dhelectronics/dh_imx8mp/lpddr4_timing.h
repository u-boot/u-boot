/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#ifndef __LPDDR4_TIMING_H__
#define __LPDDR4_TIMING_H__

static const u16 dh_imx8mp_dhcom_dram_size[] = {
	4096, 1024, 1536, 2048, 3072, 4096, 6144, 8192
};

extern struct dram_timing_info dh_imx8mp_dhcom_dram_timing_16g_x32;
static __maybe_unused struct dram_timing_info *dh_imx8mp_dhcom_dram_timing =
	&dh_imx8mp_dhcom_dram_timing_16g_x32;
void dh_imx8mp_dhcom_dram_patch_16g_x32_to_16g_x32(void);
void dh_imx8mp_dhcom_dram_patch_16g_x32_to_32g_x32_2r(void);
void dh_imx8mp_dhcom_dram_patch_16g_x32_to_32g_x32_1r(void);

u8 dh_get_memcfg(void);

#define DDRC_ECCCFG0_ECC_MODE_MASK	0x7

#endif /* __LPDDR4_TIMING_H__ */

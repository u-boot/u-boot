/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	Intel
 */

#include <common.h>
#include <asm/arch/fsp/azalia.h>
#include <asm/fsp/fsp_support.h>

/* ALC262 Verb Table - 10EC0262 */
static const uint32_t verb_table_data13[] = {
	/* Pin Complex (NID 0x11) */
	0x01171cf0,
	0x01171d11,
	0x01171e11,
	0x01171f41,
	/* Pin Complex (NID 0x12) */
	0x01271cf0,
	0x01271d11,
	0x01271e11,
	0x01271f41,
	/* Pin Complex (NID 0x14) */
	0x01471c10,
	0x01471d40,
	0x01471e01,
	0x01471f01,
	/* Pin Complex (NID 0x15) */
	0x01571cf0,
	0x01571d11,
	0x01571e11,
	0x01571f41,
	/* Pin Complex (NID 0x16) */
	0x01671cf0,
	0x01671d11,
	0x01671e11,
	0x01671f41,
	/* Pin Complex (NID 0x18) */
	0x01871c20,
	0x01871d98,
	0x01871ea1,
	0x01871f01,
	/* Pin Complex (NID 0x19) */
	0x01971c21,
	0x01971d98,
	0x01971ea1,
	0x01971f02,
	/* Pin Complex (NID 0x1A) */
	0x01a71c2f,
	0x01a71d30,
	0x01a71e81,
	0x01a71f01,
	/* Pin Complex */
	0x01b71c1f,
	0x01b71d40,
	0x01b71e21,
	0x01b71f02,
	/* Pin Complex */
	0x01c71cf0,
	0x01c71d11,
	0x01c71e11,
	0x01c71f41,
	/* Pin Complex */
	0x01d71c01,
	0x01d71dc6,
	0x01d71e14,
	0x01d71f40,
	/* Pin Complex */
	0x01e71cf0,
	0x01e71d11,
	0x01e71e11,
	0x01e71f41,
	/* Pin Complex */
	0x01f71cf0,
	0x01f71d11,
	0x01f71e11,
	0x01f71f41,
};

/*
 * This needs to be in ROM since if we put it in CAR, FSP init loses it when
 * it drops CAR.
 *
 * TODO(sjg@chromium.org): Move to device tree when FSP allows it
 *
 * VerbTable: (RealTek ALC262)
 * Revision ID = 0xFF, support all steps
 * Codec Verb Table For AZALIA
 * Codec Address: CAd value (0/1/2)
 * Codec Vendor: 0x10EC0262
 */
static const struct pch_azalia_verb_table azalia_verb_table[] = {
	{
		{
			0x10ec0262,
			0x0000,
			0xff,
			0x01,
			0x000b,
			0x0002,
		},
		verb_table_data13
	}
};

const struct pch_azalia_config azalia_config = {
	.pme_enable = 1,
	.docking_supported = 1,
	.docking_attached = 0,
	.hdmi_codec_enable = 1,
	.azalia_v_ci_enable = 1,
	.rsvdbits = 0,
	.azalia_verb_table_num = 1,
	.azalia_verb_table = azalia_verb_table,
	.reset_wait_timer_us = 300
};

void update_fsp_upd(struct upd_region *fsp_upd)
{
	struct memory_down_data *mem;

	/*
	 * Configure everything here to avoid the poor hard-pressed user
	 * needing to run Intel's binary configuration tool. It may also allow
	 * us to support the 1GB single core variant easily.
	 *
	 * TODO(sjg@chromium.org): Move to device tree
	 */
	fsp_upd->mrc_init_tseg_size = 8;
	fsp_upd->mrc_init_mmio_size = 0x800;
	fsp_upd->emmc_boot_mode = 0xff;
	fsp_upd->enable_sdio = 1;
	fsp_upd->enable_sdcard = 1;
	fsp_upd->enable_hsuart0 = 1;
	fsp_upd->azalia_config_ptr = (uint32_t)&azalia_config;
	fsp_upd->enable_i2_c0 = 0;
	fsp_upd->enable_i2_c2 = 0;
	fsp_upd->enable_i2_c3 = 0;
	fsp_upd->enable_i2_c4 = 0;
	fsp_upd->enable_xhci = 0;
	fsp_upd->igd_render_standby = 1;

	mem = &fsp_upd->memory_params;
	mem->enable_memory_down = 1;
	mem->dram_speed = 1;
	mem->dimm_width = 1;
	mem->dimm_density = 2;
	mem->dimm_tcl = 0xb;
	mem->dimm_trpt_rcd = 0xb;
	mem->dimm_twr = 0xc;
	mem->dimm_twtr = 6;
	mem->dimm_trrd = 6;
	mem->dimm_trtp = 6;
	mem->dimm_tfaw = 0x14;
}

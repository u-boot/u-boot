/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 * Copyright (C) 2015, Kodak Alaris, Inc
 *
 * SPDX-License-Identifier:	Intel
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/arch/fsp/azalia.h>
#include <asm/fsp/fsp_support.h>

DECLARE_GLOBAL_DATA_PTR;

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

/**
 * Override the FSP's configuration data.
 * If the device tree does not specify an integer setting, use the default
 * provided in Intel's Baytrail_FSP_Gold4.tgz release FSP/BayleyBayFsp.bsf file.
 */
void update_fsp_configs(struct fsp_config_data *config,
			struct fspinit_rtbuf *rt_buf)
{
	struct upd_region *fsp_upd = &config->fsp_upd;
	struct memory_down_data *mem;
	const void *blob = gd->fdt_blob;
	int node;

	/* Initialize runtime buffer for fsp_init() */
	rt_buf->common.stack_top = config->common.stack_top - 32;
	rt_buf->common.boot_mode = config->common.boot_mode;
	rt_buf->common.upd_data = &config->fsp_upd;

	fsp_upd->azalia_config_ptr = (uint32_t)&azalia_config;

	node = fdtdec_next_compatible(blob, 0, COMPAT_INTEL_BAYTRAIL_FSP);
	if (node < 0) {
		debug("%s: Cannot find FSP node\n", __func__);
		return;
	}

	fsp_upd->mrc_init_tseg_size = fdtdec_get_int(blob, node,
						     "fsp,mrc-init-tseg-size",
						     MRC_INIT_TSEG_SIZE_1MB);
	fsp_upd->mrc_init_mmio_size = fdtdec_get_int(blob, node,
						     "fsp,mrc-init-mmio-size",
						     MRC_INIT_MMIO_SIZE_2048MB);
	fsp_upd->mrc_init_spd_addr1 = fdtdec_get_int(blob, node,
						     "fsp,mrc-init-spd-addr1",
						     0xa0);
	fsp_upd->mrc_init_spd_addr2 = fdtdec_get_int(blob, node,
						     "fsp,mrc-init-spd-addr2",
						     0xa2);
	fsp_upd->emmc_boot_mode = fdtdec_get_int(blob, node,
						 "fsp,emmc-boot-mode",
						 EMMC_BOOT_MODE_EMMC41);
	fsp_upd->enable_sdio = fdtdec_get_bool(blob, node, "fsp,enable-sdio");
	fsp_upd->enable_sdcard = fdtdec_get_bool(blob, node,
						 "fsp,enable-sdcard");
	fsp_upd->enable_hsuart0 = fdtdec_get_bool(blob, node,
						  "fsp,enable-hsuart0");
	fsp_upd->enable_hsuart1 = fdtdec_get_bool(blob, node,
						  "fsp,enable-hsuart1");
	fsp_upd->enable_spi = fdtdec_get_bool(blob, node, "fsp,enable-spi");
	fsp_upd->enable_sata = fdtdec_get_bool(blob, node, "fsp,enable-sata");
	fsp_upd->sata_mode = fdtdec_get_int(blob, node, "fsp,sata-mode",
					    SATA_MODE_AHCI);
	fsp_upd->enable_azalia = fdtdec_get_bool(blob, node,
						 "fsp,enable-azalia");
	fsp_upd->enable_xhci = fdtdec_get_bool(blob, node, "fsp,enable-xhci");
	fsp_upd->lpe_mode = fdtdec_get_int(blob, node, "fsp,lpe-mode",
					   LPE_MODE_PCI);
	fsp_upd->lpss_sio_mode = fdtdec_get_int(blob, node, "fsp,lpss-sio-mode",
					   LPSS_SIO_MODE_PCI);
	fsp_upd->enable_dma0 = fdtdec_get_bool(blob, node, "fsp,enable-dma0");
	fsp_upd->enable_dma1 = fdtdec_get_bool(blob, node, "fsp,enable-dma1");
	fsp_upd->enable_i2_c0 = fdtdec_get_bool(blob, node, "fsp,enable-i2c0");
	fsp_upd->enable_i2_c1 = fdtdec_get_bool(blob, node, "fsp,enable-i2c1");
	fsp_upd->enable_i2_c2 = fdtdec_get_bool(blob, node, "fsp,enable-i2c2");
	fsp_upd->enable_i2_c3 = fdtdec_get_bool(blob, node, "fsp,enable-i2c3");
	fsp_upd->enable_i2_c4 = fdtdec_get_bool(blob, node, "fsp,enable-i2c4");
	fsp_upd->enable_i2_c5 = fdtdec_get_bool(blob, node, "fsp,enable-i2c5");
	fsp_upd->enable_i2_c6 = fdtdec_get_bool(blob, node, "fsp,enable-i2c6");
	fsp_upd->enable_pwm0 = fdtdec_get_bool(blob, node, "fsp,enable-pwm0");
	fsp_upd->enable_pwm1 = fdtdec_get_bool(blob, node, "fsp,enable-pwm1");
	fsp_upd->enable_hsi = fdtdec_get_bool(blob, node, "fsp,enable-hsi");
	fsp_upd->igd_dvmt50_pre_alloc = fdtdec_get_int(blob, node,
			"fsp,igd-dvmt50-pre-alloc", IGD_DVMT50_PRE_ALLOC_64MB);
	fsp_upd->aperture_size = fdtdec_get_int(blob, node, "fsp,aperture-size",
						APERTURE_SIZE_256MB);
	fsp_upd->gtt_size = fdtdec_get_int(blob, node, "fsp,gtt-size",
					   GTT_SIZE_2MB);
	fsp_upd->mrc_debug_msg = fdtdec_get_bool(blob, node,
						 "fsp,mrc-debug-msg");
	fsp_upd->isp_enable = fdtdec_get_bool(blob, node, "fsp,isp-enable");
	fsp_upd->scc_mode = fdtdec_get_int(blob, node, "fsp,scc-mode",
					   SCC_MODE_PCI);
	fsp_upd->igd_render_standby = fdtdec_get_bool(blob, node,
						      "fsp,igd-render-standby");
	fsp_upd->txe_uma_enable = fdtdec_get_bool(blob, node,
						  "fsp,txe-uma-enable");
	fsp_upd->os_selection = fdtdec_get_int(blob, node, "fsp,os-selection",
					       OS_SELECTION_LINUX);
	fsp_upd->emmc45_ddr50_enabled = fdtdec_get_bool(blob, node,
			"fsp,emmc45-ddr50-enabled");
	fsp_upd->emmc45_hs200_enabled = fdtdec_get_bool(blob, node,
			"fsp,emmc45-hs200-enabled");
	fsp_upd->emmc45_retune_timer_value = fdtdec_get_int(blob, node,
			"fsp,emmc45-retune-timer-value", 8);
	fsp_upd->enable_igd = fdtdec_get_bool(blob, node, "fsp,enable-igd");

	mem = &fsp_upd->memory_params;
	mem->enable_memory_down = fdtdec_get_bool(blob, node,
						  "fsp,enable-memory-down");
	if (mem->enable_memory_down) {
		node = fdtdec_next_compatible(blob, node,
					      COMPAT_INTEL_BAYTRAIL_FSP_MDP);
		if (node < 0) {
			debug("%s: Cannot find FSP memory-down-params node\n",
			      __func__);
		} else {
			mem->dram_speed = fdtdec_get_int(blob, node,
							 "fsp,dram-speed",
							 DRAM_SPEED_1333MTS);
			mem->dram_type = fdtdec_get_int(blob, node,
							"fsp,dram-type",
							DRAM_TYPE_DDR3L);
			mem->dimm_0_enable = fdtdec_get_bool(blob, node,
					"fsp,dimm-0-enable");
			mem->dimm_1_enable = fdtdec_get_bool(blob, node,
					"fsp,dimm-1-enable");
			mem->dimm_width = fdtdec_get_int(blob, node,
							 "fsp,dimm-width",
							 DIMM_WIDTH_X8);
			mem->dimm_density = fdtdec_get_int(blob, node,
							   "fsp,dimm-density",
							   DIMM_DENSITY_2GBIT);
			mem->dimm_bus_width = fdtdec_get_int(blob, node,
					"fsp,dimm-bus-width",
					DIMM_BUS_WIDTH_64BITS);
			mem->dimm_sides = fdtdec_get_int(blob, node,
							 "fsp,dimm-sides",
							 DIMM_SIDES_1RANKS);
			mem->dimm_tcl = fdtdec_get_int(blob, node,
						       "fsp,dimm-tcl", 0x09);
			mem->dimm_trpt_rcd = fdtdec_get_int(blob, node,
					"fsp,dimm-trpt-rcd", 0x09);
			mem->dimm_twr = fdtdec_get_int(blob, node,
						       "fsp,dimm-twr", 0x0a);
			mem->dimm_twtr = fdtdec_get_int(blob, node,
							"fsp,dimm-twtr", 0x05);
			mem->dimm_trrd = fdtdec_get_int(blob, node,
							"fsp,dimm-trrd", 0x04);
			mem->dimm_trtp = fdtdec_get_int(blob, node,
							"fsp,dimm-trtp", 0x05);
			mem->dimm_tfaw = fdtdec_get_int(blob, node,
							"fsp,dimm-tfaw", 0x14);
		}
	}
}

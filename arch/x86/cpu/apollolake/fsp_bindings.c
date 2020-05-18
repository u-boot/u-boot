// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2020 B&R Industrial Automation GmbH - http://www.br-automation.com
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/arch/fsp_bindings.h>

/**
 * read_u8_prop() - Read an u8 property from devicetree (scalar or array)
 * @node:  Valid node reference to read property from
 * @name:  Name of the property to read from
 * @count: If the property is expected to be an array, this is the
 *         number of expected elements
 *         Set to 0 if the property is expected to be a scalar
 * @dst:   Pointer to destination of where to save the value(s) read
 *         from devicetree
 */
static void read_u8_prop(ofnode node, char *name, size_t count, u8 *dst)
{
	u32 tmp;
	const u8 *buf;
	int ret;

	if (count == 0) {
		ret = ofnode_read_u32(node, name, &tmp);
		if (ret == 0)
			*dst = tmp;
	} else {
		buf = ofnode_read_u8_array_ptr(node, name, count);
		if (buf)
			memcpy(dst, buf, count);
	}
}

/**
 * read_u16_prop() - Read an u16 property from devicetree (scalar or array)
 * @node:  Valid node reference to read property from
 * @name:  Name of the property to read from
 * @count: If the property is expected to be an array, this is the
 *         number of expected elements
 *         Set to 0 if the property is expected to be a scalar
 * @dst:   Pointer to destination of where to save the value(s) read
 *         from devicetree
 * @return 0 on success, -ve on error
 */
static int read_u16_prop(ofnode node, char *name, size_t count, u16 *dst)
{
	u32 tmp;
	u32 buf[32];
	int ret;

	if (ARRAY_SIZE(buf) < count) {
		debug("ERROR: %s buffer to small!\n", __func__);
		return -ENOSPC;
	}

	if (count == 0) {
		ret = ofnode_read_u32(node, name, &tmp);
		if (ret == 0)
			*dst = tmp;
	} else {
		ret = ofnode_read_u32_array(node, name, buf, count);
		if (ret == 0)
			for (int i = 0; i < count; i++)
				dst[i] = buf[i];
	}

	return 0;
}

/**
 * read_u32_prop() - Read an u32 property from devicetree (scalar or array)
 * @node:  Valid node reference to read property from
 * @name:  Name of the property to read from
 * @count: If the property is expected to be an array, this is the
 *         number of expected elements
 *         set to 0 if the property is expected to be a scalar
 * @dst:   Pointer to destination of where to save the value(s) read
 *         from devicetree
 */
static void read_u32_prop(ofnode node, char *name, size_t count, u32 *dst)
{
	if (count == 0)
		ofnode_read_u32(node, name, dst);
	else
		ofnode_read_u32_array(node, name, dst, count);
}

/**
 * read_string_prop() - Read a string property from devicetree
 * @node:  Valid node reference to read property from
 * @name:  Name of the property to read from
 * @count: Size of the destination buffer
 * @dst:   Pointer to destination of where to save the values read
 *         from devicetree
 */
static void read_string_prop(ofnode node, char *name, size_t count, char *dst)
{
	const char *string_buf;

	if (count > 0) {
		string_buf = ofnode_read_string(node, name);
		if (string_buf)
			strlcpy(dst, string_buf, count);
	}
}

/**
 * read_swizzle_prop() - Read a swizzle property from devicetree
 * @node:  Valid node reference to read property from
 * @name:  Name of the property to read from
 * @count: Number of elements in the swizzle configuration
 * @dst:   pointer to destination of where to save the values read
 *         from devicetree
 */
static void read_swizzle_prop(ofnode node, char *name, size_t count, u8 *dst)
{
	const struct lpddr4_chan_swizzle_cfg *sch;
	/* Number of bytes to copy per DQS */
	const size_t sz = DQ_BITS_PER_DQS;
	const struct lpddr4_swizzle_cfg *swizzle_cfg;

	swizzle_cfg = (const struct lpddr4_swizzle_cfg *)
			ofnode_read_u8_array_ptr(node, name, count);

	if (!swizzle_cfg)
		return;
	/*
	 * CH0_DQB byte lanes in the bit swizzle configuration field are
	 * not 1:1. The mapping within the swizzling field is:
	 *   indices [0:7]   - byte lane 1 (DQS1) DQ[8:15]
	 *   indices [8:15]  - byte lane 0 (DQS0) DQ[0:7]
	 *   indices [16:23] - byte lane 3 (DQS3) DQ[24:31]
	 *   indices [24:31] - byte lane 2 (DQS2) DQ[16:23]
	 */
	sch = &swizzle_cfg->phys[LP4_PHYS_CH0B];
	memcpy(&dst[0 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS1], sz);
	memcpy(&dst[1 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS0], sz);
	memcpy(&dst[2 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS3], sz);
	memcpy(&dst[3 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS2], sz);

	/*
	 * CH0_DQA byte lanes in the bit swizzle configuration field are 1:1.
	 */
	sch = &swizzle_cfg->phys[LP4_PHYS_CH0A];
	memcpy(&dst[4 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS0], sz);
	memcpy(&dst[5 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS1], sz);
	memcpy(&dst[6 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS2], sz);
	memcpy(&dst[7 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS3], sz);

	sch = &swizzle_cfg->phys[LP4_PHYS_CH1B];
	memcpy(&dst[8 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS1], sz);
	memcpy(&dst[9 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS0], sz);
	memcpy(&dst[10 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS3], sz);
	memcpy(&dst[11 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS2], sz);

	/*
	 * CH0_DQA byte lanes in the bit swizzle configuration field are 1:1.
	 */
	sch = &swizzle_cfg->phys[LP4_PHYS_CH1A];
	memcpy(&dst[12 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS0], sz);
	memcpy(&dst[13 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS1], sz);
	memcpy(&dst[14 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS2], sz);
	memcpy(&dst[15 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS3], sz);
}

/**
 * fsp_update_config_from_dtb() - Read FSP config from devicetree node
 * @node: Valid node reference to read property from
 * @cfg:  Pointer to FSP config structure
 * @fsp_bindings: Binding describing which devicetree properties should
 *                be stored where in the FSP configuration structure
 *                The end of the list is declared by a NULL pointer in propname
 * @return 0 on success, -ve on error
 *
 * This function reads the configuration for FSP from the provided
 * devicetree node and saves it in the FSP configuration structure.
 * Configuration options that are not present in the devicetree are
 * left at their current value.
 */
__maybe_unused
static int fsp_update_config_from_dtb(ofnode node, u8 *cfg,
				      const struct fsp_binding *fsp_bindings)
{
	const struct fsp_binding *fspb;
	int ret;

	for (int i = 0; fsp_bindings[i].propname; i++) {
		fspb = &fsp_bindings[i];

		switch (fspb->type) {
		case FSP_UINT8:
			read_u8_prop(node, fspb->propname, fspb->count,
				     &cfg[fspb->offset]);
		break;
		case FSP_UINT16:
			ret = read_u16_prop(node, fspb->propname, fspb->count,
					    (u16 *)&cfg[fspb->offset]);
			if (ret)
				return ret;
		break;
		case FSP_UINT32:
			read_u32_prop(node, fspb->propname, fspb->count,
				      (u32 *)&cfg[fspb->offset]);
		break;
		case FSP_STRING:
			read_string_prop(node, fspb->propname, fspb->count,
					 (char *)&cfg[fspb->offset]);
		break;
		case FSP_LPDDR4_SWIZZLE:
			read_swizzle_prop(node, fspb->propname, fspb->count,
					  &cfg[fspb->offset]);
		break;
		}
	}

	return 0;
}

#if defined(CONFIG_SPL_BUILD)
const struct fsp_binding fsp_m_bindings[] = {
	{
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, serial_debug_port_address),
	.propname = "fspm,serial-debug-port-address",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, serial_debug_port_type),
	.propname = "fspm,serial-debug-port-type",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, serial_debug_port_device),
	.propname = "fspm,serial-debug-port-device",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, serial_debug_port_stride_size),
	.propname = "fspm,serial-debug-port-stride-size",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, mrc_fast_boot),
	.propname = "fspm,mrc-fast-boot",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, igd),
	.propname = "fspm,igd",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, igd_dvmt50_pre_alloc),
	.propname = "fspm,igd-dvmt50-pre-alloc",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, igd_aperture_size),
	.propname = "fspm,igd-aperture-size",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, gtt_size),
	.propname = "fspm,gtt-size",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, primary_video_adaptor),
	.propname = "fspm,primary-video-adaptor",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, package),
	.propname = "fspm,package",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, profile),
	.propname = "fspm,profile",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, memory_down),
	.propname = "fspm,memory-down",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, ddr3_l_page_size),
	.propname = "fspm,ddr3-l-page-size",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, ddr3_lasr),
	.propname = "fspm,ddr3-lasr",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, scrambler_support),
	.propname = "fspm,scrambler-support",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, interleaved_mode),
	.propname = "fspm,interleaved-mode",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_m_config, channel_hash_mask),
	.propname = "fspm,channel-hash-mask",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_m_config, slice_hash_mask),
	.propname = "fspm,slice-hash-mask",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, channels_slices_enable),
	.propname = "fspm,channels-slices-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, min_ref_rate2x_enable),
	.propname = "fspm,min-ref-rate2x-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, dual_rank_support_enable),
	.propname = "fspm,dual-rank-support-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, rmt_mode),
	.propname = "fspm,rmt-mode",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_m_config, memory_size_limit),
	.propname = "fspm,memory-size-limit",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_m_config, low_memory_max_value),
	.propname = "fspm,low-memory-max-value",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_m_config, high_memory_max_value),
	.propname = "fspm,high-memory-max-value",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, disable_fast_boot),
	.propname = "fspm,disable-fast-boot",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, dimm0_spd_address),
	.propname = "fspm,dimm0-spd-address",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, dimm1_spd_address),
	.propname = "fspm,dimm1-spd-address",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].rank_enable),
	.propname = "fspm,ch0-rank-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].device_width),
	.propname = "fspm,ch0-device-width",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].dram_density),
	.propname = "fspm,ch0-dram-density",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].option),
	.propname = "fspm,ch0-option",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].odt_config),
	.propname = "fspm,ch0-odt-config",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].tristate_clk1),
	.propname = "fspm,ch0-tristate-clk1",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].mode2_n),
	.propname = "fspm,ch0-mode2-n",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].odt_levels),
	.propname = "fspm,ch0-odt-levels",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].rank_enable),
	.propname = "fspm,ch1-rank-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].device_width),
	.propname = "fspm,ch1-device-width",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].dram_density),
	.propname = "fspm,ch1-dram-density",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].option),
	.propname = "fspm,ch1-option",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].odt_config),
	.propname = "fspm,ch1-odt-config",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].tristate_clk1),
	.propname = "fspm,ch1-tristate-clk1",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].mode2_n),
	.propname = "fspm,ch1-mode2-n",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].odt_levels),
	.propname = "fspm,ch1-odt-levels",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].rank_enable),
	.propname = "fspm,ch2-rank-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].device_width),
	.propname = "fspm,ch2-device-width",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].dram_density),
	.propname = "fspm,ch2-dram-density",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].option),
	.propname = "fspm,ch2-option",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].odt_config),
	.propname = "fspm,ch2-odt-config",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].tristate_clk1),
	.propname = "fspm,ch2-tristate-clk1",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].mode2_n),
	.propname = "fspm,ch2-mode2-n",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].odt_levels),
	.propname = "fspm,ch2-odt-levels",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].rank_enable),
	.propname = "fspm,ch3-rank-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].device_width),
	.propname = "fspm,ch3-device-width",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].dram_density),
	.propname = "fspm,ch3-dram-density",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].option),
	.propname = "fspm,ch3-option",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].odt_config),
	.propname = "fspm,ch3-odt-config",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].tristate_clk1),
	.propname = "fspm,ch3-tristate-clk1",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].mode2_n),
	.propname = "fspm,ch3-mode2-n",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].odt_levels),
	.propname = "fspm,ch3-odt-levels",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, rmt_check_run),
	.propname = "fspm,rmt-check-run",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_m_config,
			   rmt_margin_check_scale_high_threshold),
	.propname = "fspm,rmt-margin-check-scale-high-threshold",
	}, {
	.type = FSP_LPDDR4_SWIZZLE,
	.offset = offsetof(struct fsp_m_config, ch_bit_swizzling),
	.propname = "fspm,ch-bit-swizzling",
	.count = SIZE_OF_MEMBER(struct fsp_m_config, ch_bit_swizzling) /
		 SIZE_OF_MEMBER(struct fsp_m_config, ch_bit_swizzling[0][0])
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, msg_level_mask),
	.propname = "fspm,msg-level-mask",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, pre_mem_gpio_table_pin_num),
	.propname = "fspm,pre-mem-gpio-table-pin-num",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_m_config,
				      pre_mem_gpio_table_pin_num),
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, pre_mem_gpio_table_ptr),
	.propname = "fspm,pre-mem-gpio-table-ptr",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, pre_mem_gpio_table_entry_num),
	.propname = "fspm,pre-mem-gpio-table-entry-num",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, enhance_port8xh_decoding),
	.propname = "fspm,enhance-port8xh-decoding",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, spd_write_enable),
	.propname = "fspm,spd-write-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, mrc_data_saving),
	.propname = "fspm,mrc-data-saving",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, oem_loading_base),
	.propname = "fspm,oem-loading-base",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, oem_file_name),
	.propname = "fspm,oem-file-name",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_m_config, oem_file_name),
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, mrc_boot_data_ptr),
	.propname = "fspm,mrc-boot-data-ptr",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, e_mmc_trace_len),
	.propname = "fspm,e-mmc-trace-len",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, skip_cse_rbp),
	.propname = "fspm,skip-cse-rbp",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, npk_en),
	.propname = "fspm,npk-en",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, fw_trace_en),
	.propname = "fspm,fw-trace-en",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, fw_trace_destination),
	.propname = "fspm,fw-trace-destination",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, recover_dump),
	.propname = "fspm,recover-dump",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, msc0_wrap),
	.propname = "fspm,msc0-wrap",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, msc1_wrap),
	.propname = "fspm,msc1-wrap",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, msc0_size),
	.propname = "fspm,msc0-size",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, msc1_size),
	.propname = "fspm,msc1-size",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, pti_mode),
	.propname = "fspm,pti-mode",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, pti_training),
	.propname = "fspm,pti-training",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, pti_speed),
	.propname = "fspm,pti-speed",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, punit_mlvl),
	.propname = "fspm,punit-mlvl",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, pmc_mlvl),
	.propname = "fspm,pmc-mlvl",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, sw_trace_en),
	.propname = "fspm,sw-trace-en",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, periodic_retraining_disable),
	.propname = "fspm,periodic-retraining-disable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, enable_reset_system),
	.propname = "fspm,enable-reset-system",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, enable_s3_heci2),
	.propname = "fspm,enable-s3-heci2",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, variable_nvs_buffer_ptr),
	.propname = "fspm,variable-nvs-buffer-ptr",
	}, {
	.propname = NULL
	}
};

int fsp_m_update_config_from_dtb(ofnode node, struct fsp_m_config *cfg)
{
	return fsp_update_config_from_dtb(node, (u8 *)cfg, fsp_m_bindings);
}
#endif

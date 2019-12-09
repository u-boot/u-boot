/* SPDX-License-Identifier: Intel */
/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 * Copyright 2019 Google LLC
 */

#ifndef	__ASM_ARCH_FSP_M_UDP_H
#define	__ASM_ARCH_FSP_M_UDP_H

#include <asm/fsp2/fsp_api.h>

#define FSP_DRAM_CHANNELS	4

struct __packed fspm_arch_upd {
	u8	revision;
	u8	reserved[3];
	void	*nvs_buffer_ptr;
	void	*stack_base;
	u32	stack_size;
	u32	boot_loader_tolum_size;
	u32	boot_mode;
	u8	reserved1[8];
};

struct __packed fsp_ram_channel {
	u8	rank_enable;
	u8	device_width;
	u8	dram_density;
	u8	option;
	u8	odt_config;
	u8	tristate_clk1;
	u8	mode2_n;
	u8	odt_levels;
};

struct __packed fsp_m_config {
	u32	serial_debug_port_address;
	u8	serial_debug_port_type;
	u8	serial_debug_port_device;
	u8	serial_debug_port_stride_size;
	u8	mrc_fast_boot;
	u8	igd;
	u8	igd_dvmt50_pre_alloc;
	u8	igd_aperture_size;
	u8	gtt_size;
	u8	primary_video_adaptor;
	u8	package;
	u8	profile;
	u8	memory_down;

	u8	ddr3_l_page_size;
	u8	ddr3_lasr;
	u8	scrambler_support;
	u8	interleaved_mode;
	u16	channel_hash_mask;
	u16	slice_hash_mask;
	u8	channels_slices_enable;
	u8	min_ref_rate2x_enable;
	u8	dual_rank_support_enable;
	u8	rmt_mode;
	u16	memory_size_limit;
	u16	low_memory_max_value;

	u16	high_memory_max_value;
	u8	disable_fast_boot;
	u8	dimm0_spd_address;
	u8	dimm1_spd_address;
	struct fsp_ram_channel chan[FSP_DRAM_CHANNELS];
	u8	rmt_check_run;
	u16	rmt_margin_check_scale_high_threshold;
	u8	ch_bit_swizzling[FSP_DRAM_CHANNELS][32];
	u32	msg_level_mask;
	u8	unused_upd_space0[4];

	u8	pre_mem_gpio_table_pin_num[4];
	u32	pre_mem_gpio_table_ptr;
	u8	pre_mem_gpio_table_entry_num;
	u8	enhance_port8xh_decoding;
	u8	spd_write_enable;
	u8	mrc_data_saving;
	u32	oem_loading_base;

	u8	oem_file_name[16];

	void	*mrc_boot_data_ptr;
	u8	e_mmc_trace_len;
	u8	skip_cse_rbp;
	u8	npk_en;
	u8	fw_trace_en;
	u8	fw_trace_destination;
	u8	recover_dump;
	u8	msc0_wrap;
	u8	msc1_wrap;
	u32	msc0_size;

	u32	msc1_size;
	u8	pti_mode;
	u8	pti_training;
	u8	pti_speed;
	u8	punit_mlvl;

	u8	pmc_mlvl;
	u8	sw_trace_en;
	u8	periodic_retraining_disable;
	u8	enable_reset_system;

	u8	enable_s3_heci2;
	u8	unused_upd_space1[3];

	void	*variable_nvs_buffer_ptr;
	u8	reserved_fspm_upd[12];
};

/** FSP-M UPD Configuration */
struct __packed fspm_upd {
	struct fsp_upd_header header;
	struct fspm_arch_upd arch;
	struct fsp_m_config config;
	u8 unused_upd_space2[158];
	u16 upd_terminator;
};

#endif

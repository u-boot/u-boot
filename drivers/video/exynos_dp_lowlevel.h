/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _EXYNOS_EDP_LOWLEVEL_H
#define _EXYNOS_EDP_LOWLEVEL_H

void exynos_dp_enable_video_bist(unsigned int enable);
void exynos_dp_enable_video_mute(unsigned int enable);
void exynos_dp_reset(void);
void exynos_dp_enable_sw_func(unsigned int enable);
unsigned int exynos_dp_set_analog_power_down(unsigned int block, u32 enable);
unsigned int exynos_dp_get_pll_lock_status(void);
int exynos_dp_init_analog_func(void);
void exynos_dp_init_hpd(void);
void exynos_dp_init_aux(void);
void exynos_dp_config_interrupt(void);
unsigned int exynos_dp_get_plug_in_status(void);
unsigned int exynos_dp_detect_hpd(void);
unsigned int exynos_dp_start_aux_transaction(void);
unsigned int exynos_dp_write_byte_to_dpcd(unsigned int reg_addr,
				unsigned char data);
unsigned int exynos_dp_read_byte_from_dpcd(unsigned int reg_addr,
		unsigned char *data);
unsigned int exynos_dp_write_bytes_to_dpcd(unsigned int reg_addr,
		unsigned int count,
		unsigned char data[]);
unsigned int exynos_dp_read_bytes_from_dpcd( unsigned int reg_addr,
		unsigned int count,
		unsigned char data[]);
int exynos_dp_select_i2c_device( unsigned int device_addr,
		unsigned int reg_addr);
int exynos_dp_read_byte_from_i2c(unsigned int device_addr,
		unsigned int reg_addr, unsigned int *data);
int exynos_dp_read_bytes_from_i2c(unsigned int device_addr,
		unsigned int reg_addr, unsigned int count,
		unsigned char edid[]);
void exynos_dp_reset_macro(void);
void exynos_dp_set_link_bandwidth(unsigned char bwtype);
unsigned char exynos_dp_get_link_bandwidth(void);
void exynos_dp_set_lane_count(unsigned char count);
unsigned int exynos_dp_get_lane_count(void);
unsigned char exynos_dp_get_lanex_pre_emphasis(unsigned char lanecnt);
void exynos_dp_set_lane_pre_emphasis(unsigned int level,
		unsigned char lanecnt);
void exynos_dp_set_lanex_pre_emphasis(unsigned char request_val,
		unsigned char lanecnt);
void exynos_dp_set_training_pattern(unsigned int pattern);
void exynos_dp_enable_enhanced_mode(unsigned char enable);
void exynos_dp_enable_scrambling(unsigned int enable);
int exynos_dp_init_video(void);
void exynos_dp_config_video_slave_mode(struct edp_video_info *video_info);
void exynos_dp_set_video_color_format(struct edp_video_info *video_info);
int exynos_dp_config_video_bist(struct edp_device_info *edp_info);
unsigned int exynos_dp_is_slave_video_stream_clock_on(void);
void exynos_dp_set_video_cr_mn(unsigned int type, unsigned int m_value,
		unsigned int n_value);
void exynos_dp_set_video_timing_mode(unsigned int type);
void exynos_dp_enable_video_master(unsigned int enable);
void exynos_dp_start_video(void);
unsigned int exynos_dp_is_video_stream_on(void);
void exynos_dp_set_base_addr(void);

#endif /* _EXYNOS_DP_LOWLEVEL_H */

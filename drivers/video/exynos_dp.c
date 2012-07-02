/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <linux/err.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dp_info.h>
#include <asm/arch/dp.h>

#include "exynos_dp_lowlevel.h"

static struct exynos_dp_platform_data *dp_pd;

static void exynos_dp_disp_info(struct edp_disp_info *disp_info)
{
	disp_info->h_total = disp_info->h_res + disp_info->h_sync_width +
		disp_info->h_back_porch + disp_info->h_front_porch;
	disp_info->v_total = disp_info->v_res + disp_info->v_sync_width +
		disp_info->v_back_porch + disp_info->v_front_porch;

	return;
}

static int exynos_dp_init_dp(void)
{
	int ret;
	exynos_dp_reset();

	/* SW defined function Normal operation */
	exynos_dp_enable_sw_func(DP_ENABLE);

	ret = exynos_dp_init_analog_func();
	if (ret != EXYNOS_DP_SUCCESS)
		return ret;

	exynos_dp_init_hpd();
	exynos_dp_init_aux();

	return ret;
}

static unsigned char exynos_dp_calc_edid_check_sum(unsigned char *edid_data)
{
	int i;
	unsigned char sum = 0;

	for (i = 0; i < EDID_BLOCK_LENGTH; i++)
		sum = sum + edid_data[i];

	return sum;
}

static unsigned int exynos_dp_read_edid(void)
{
	unsigned char edid[EDID_BLOCK_LENGTH * 2];
	unsigned int extend_block = 0;
	unsigned char sum;
	unsigned char test_vector;
	int retval;

	/*
	 * EDID device address is 0x50.
	 * However, if necessary, you must have set upper address
	 * into E-EDID in I2C device, 0x30.
	 */

	/* Read Extension Flag, Number of 128-byte EDID extension blocks */
	exynos_dp_read_byte_from_i2c(I2C_EDID_DEVICE_ADDR, EDID_EXTENSION_FLAG,
			&extend_block);

	if (extend_block > 0) {
		printf("DP EDID data includes a single extension!\n");

		/* Read EDID data */
		retval = exynos_dp_read_bytes_from_i2c(I2C_EDID_DEVICE_ADDR,
						EDID_HEADER_PATTERN,
						EDID_BLOCK_LENGTH,
						&edid[EDID_HEADER_PATTERN]);
		if (retval != 0) {
			printf("DP EDID Read failed!\n");
			return -1;
		}
		sum = exynos_dp_calc_edid_check_sum(edid);
		if (sum != 0) {
			printf("DP EDID bad checksum!\n");
			return -1;
		}

		/* Read additional EDID data */
		retval = exynos_dp_read_bytes_from_i2c(I2C_EDID_DEVICE_ADDR,
				EDID_BLOCK_LENGTH,
				EDID_BLOCK_LENGTH,
				&edid[EDID_BLOCK_LENGTH]);
		if (retval != 0) {
			printf("DP EDID Read failed!\n");
			return -1;
		}
		sum = exynos_dp_calc_edid_check_sum(&edid[EDID_BLOCK_LENGTH]);
		if (sum != 0) {
			printf("DP EDID bad checksum!\n");
			return -1;
		}

		exynos_dp_read_byte_from_dpcd(DPCD_TEST_REQUEST,
					&test_vector);
		if (test_vector & DPCD_TEST_EDID_READ) {
			exynos_dp_write_byte_to_dpcd(DPCD_TEST_EDID_CHECKSUM,
				edid[EDID_BLOCK_LENGTH + EDID_CHECKSUM]);
			exynos_dp_write_byte_to_dpcd(DPCD_TEST_RESPONSE,
				DPCD_TEST_EDID_CHECKSUM_WRITE);
		}
	} else {
		debug("DP EDID data does not include any extensions.\n");

		/* Read EDID data */
		retval = exynos_dp_read_bytes_from_i2c(I2C_EDID_DEVICE_ADDR,
				EDID_HEADER_PATTERN,
				EDID_BLOCK_LENGTH,
				&edid[EDID_HEADER_PATTERN]);

		if (retval != 0) {
			printf("DP EDID Read failed!\n");
			return -1;
		}
		sum = exynos_dp_calc_edid_check_sum(edid);
		if (sum != 0) {
			printf("DP EDID bad checksum!\n");
			return -1;
		}

		exynos_dp_read_byte_from_dpcd(DPCD_TEST_REQUEST,
			&test_vector);
		if (test_vector & DPCD_TEST_EDID_READ) {
			exynos_dp_write_byte_to_dpcd(DPCD_TEST_EDID_CHECKSUM,
				edid[EDID_CHECKSUM]);
			exynos_dp_write_byte_to_dpcd(DPCD_TEST_RESPONSE,
				DPCD_TEST_EDID_CHECKSUM_WRITE);
		}
	}

	debug("DP EDID Read success!\n");

	return 0;
}

static unsigned int exynos_dp_handle_edid(struct edp_device_info *edp_info)
{
	unsigned char buf[12];
	unsigned int ret;
	unsigned char temp;
	unsigned char retry_cnt;
	unsigned char dpcd_rev[16];
	unsigned char lane_bw[16];
	unsigned char lane_cnt[16];

	memset(dpcd_rev, 0, 16);
	memset(lane_bw, 0, 16);
	memset(lane_cnt, 0, 16);
	memset(buf, 0, 12);

	retry_cnt = 5;
	while (retry_cnt) {
		/* Read DPCD 0x0000-0x000b */
		ret = exynos_dp_read_bytes_from_dpcd(DPCD_DPCD_REV, 12,
				buf);
		if (ret != EXYNOS_DP_SUCCESS) {
			if (retry_cnt == 0) {
				printf("DP read_byte_from_dpcd() failed\n");
				return ret;
			}
			retry_cnt--;
		} else
			break;
	}

	/* */
	temp = buf[DPCD_DPCD_REV];
	if (temp == DP_DPCD_REV_10 || temp == DP_DPCD_REV_11)
		edp_info->dpcd_rev = temp;
	else {
		printf("DP Wrong DPCD Rev : %x\n", temp);
		return -ENODEV;
	}

	temp = buf[DPCD_MAX_LINK_RATE];
	if (temp == DP_LANE_BW_1_62 || temp == DP_LANE_BW_2_70)
		edp_info->lane_bw = temp;
	else {
		printf("DP Wrong MAX LINK RATE : %x\n", temp);
		return -EINVAL;
	}

	/*Refer VESA Display Port Stnadard Ver1.1a Page 120 */
	if (edp_info->dpcd_rev == DP_DPCD_REV_11) {
		temp = buf[DPCD_MAX_LANE_COUNT] & 0x1f;
		if (buf[DPCD_MAX_LANE_COUNT] & 0x80)
			edp_info->dpcd_efc = 1;
		else
			edp_info->dpcd_efc = 0;
	} else {
		temp = buf[DPCD_MAX_LANE_COUNT];
		edp_info->dpcd_efc = 0;
	}

	if (temp == DP_LANE_CNT_1 || temp == DP_LANE_CNT_2 ||
			temp == DP_LANE_CNT_4) {
		edp_info->lane_cnt = temp;
	} else {
		printf("DP Wrong MAX LANE COUNT : %x\n", temp);
		return -EINVAL;
	}

	ret = exynos_dp_read_edid();
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP exynos_dp_read_edid() failed\n");
		return -EINVAL;
	}

	return ret;
}

static void exynos_dp_init_training(void)
{
	/*
	 * MACRO_RST must be applied after the PLL_LOCK to avoid
	 * the DP inter pair skew issue for at least 10 us
	 */
	exynos_dp_reset_macro();

	/* All DP analog module power up */
	exynos_dp_set_analog_power_down(POWER_ALL, 0);
}

static unsigned int exynos_dp_link_start(struct edp_device_info *edp_info)
{
	unsigned char buf[5];
	unsigned int ret = 0;

	debug("DP: %s was called\n", __func__);

	edp_info->lt_info.lt_status = DP_LT_CR;
	edp_info->lt_info.ep_loop = 0;
	edp_info->lt_info.cr_loop[0] = 0;
	edp_info->lt_info.cr_loop[1] = 0;
	edp_info->lt_info.cr_loop[2] = 0;
	edp_info->lt_info.cr_loop[3] = 0;

		/* Set sink to D0 (Sink Not Ready) mode. */
		ret = exynos_dp_write_byte_to_dpcd(DPCD_SINK_POWER_STATE,
				DPCD_SET_POWER_STATE_D0);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP write_dpcd_byte failed\n");
		return ret;
	}

	/* Set link rate and count as you want to establish*/
	exynos_dp_set_link_bandwidth(edp_info->lane_bw);
	exynos_dp_set_lane_count(edp_info->lane_cnt);

	/* Setup RX configuration */
	buf[0] = edp_info->lane_bw;
	buf[1] = edp_info->lane_cnt;

	ret = exynos_dp_write_bytes_to_dpcd(DPCD_LINK_BW_SET, 2,
			buf);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP write_dpcd_byte failed\n");
		return ret;
	}

	exynos_dp_set_lane_pre_emphasis(PRE_EMPHASIS_LEVEL_0,
			edp_info->lane_cnt);

	/* Set training pattern 1 */
	exynos_dp_set_training_pattern(TRAINING_PTN1);

	/* Set RX training pattern */
	buf[0] = DPCD_SCRAMBLING_DISABLED | DPCD_TRAINING_PATTERN_1;

	buf[1] = DPCD_PRE_EMPHASIS_SET_PATTERN_2_LEVEL_0 |
		DPCD_VOLTAGE_SWING_SET_PATTERN_1_LEVEL_0;
	buf[2] = DPCD_PRE_EMPHASIS_SET_PATTERN_2_LEVEL_0 |
		DPCD_VOLTAGE_SWING_SET_PATTERN_1_LEVEL_0;
	buf[3] = DPCD_PRE_EMPHASIS_SET_PATTERN_2_LEVEL_0 |
		DPCD_VOLTAGE_SWING_SET_PATTERN_1_LEVEL_0;
	buf[4] = DPCD_PRE_EMPHASIS_SET_PATTERN_2_LEVEL_0 |
		DPCD_VOLTAGE_SWING_SET_PATTERN_1_LEVEL_0;

	ret = exynos_dp_write_bytes_to_dpcd(DPCD_TRAINING_PATTERN_SET,
			5, buf);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP write_dpcd_byte failed\n");
		return ret;
	}

	return ret;
}

static unsigned int exynos_dp_training_pattern_dis(void)
{
	unsigned int ret = EXYNOS_DP_SUCCESS;

	exynos_dp_set_training_pattern(DP_NONE);

	ret = exynos_dp_write_byte_to_dpcd(DPCD_TRAINING_PATTERN_SET,
			DPCD_TRAINING_PATTERN_DISABLED);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP requst_link_traninig_req failed\n");
		return -EAGAIN;
	}

	return ret;
}

static unsigned int exynos_dp_enable_rx_to_enhanced_mode(unsigned char enable)
{
	unsigned char data;
	unsigned int ret = EXYNOS_DP_SUCCESS;

	ret = exynos_dp_read_byte_from_dpcd(DPCD_LANE_COUNT_SET,
			&data);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP read_from_dpcd failed\n");
		return -EAGAIN;
	}

	if (enable)
		data = DPCD_ENHANCED_FRAME_EN | DPCD_LN_COUNT_SET(data);
	else
		data = DPCD_LN_COUNT_SET(data);

	ret = exynos_dp_write_byte_to_dpcd(DPCD_LANE_COUNT_SET,
			data);
	if (ret != EXYNOS_DP_SUCCESS) {
			printf("DP write_to_dpcd failed\n");
			return -EAGAIN;

	}

	return ret;
}

static unsigned int exynos_dp_set_enhanced_mode(unsigned char enhance_mode)
{
	unsigned int ret = EXYNOS_DP_SUCCESS;

	ret = exynos_dp_enable_rx_to_enhanced_mode(enhance_mode);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP rx_enhance_mode failed\n");
		return -EAGAIN;
	}

	exynos_dp_enable_enhanced_mode(enhance_mode);

	return ret;
}

static int exynos_dp_read_dpcd_lane_stat(struct edp_device_info *edp_info,
		unsigned char *status)
{
	unsigned int ret, i;
	unsigned char buf[2];
	unsigned char lane_stat[DP_LANE_CNT_4] = {0,};
	unsigned char shift_val[DP_LANE_CNT_4] = {0,};

	shift_val[0] = 0;
	shift_val[1] = 4;
	shift_val[2] = 0;
	shift_val[3] = 4;

	ret = exynos_dp_read_bytes_from_dpcd(DPCD_LANE0_1_STATUS, 2, buf);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP read lane status failed\n");
		return ret;
	}

	for (i = 0; i < edp_info->lane_cnt; i++) {
		lane_stat[i] = (buf[(i / 2)] >> shift_val[i]) & 0x0f;
		if (lane_stat[0] != lane_stat[i]) {
			printf("Wrong lane status\n");
			return -EINVAL;
		}
	}

	*status = lane_stat[0];

	return ret;
}

static unsigned int exynos_dp_read_dpcd_adj_req(unsigned char lane_num,
		unsigned char *sw, unsigned char *em)
{
	unsigned int ret = EXYNOS_DP_SUCCESS;
	unsigned char buf;
	unsigned int dpcd_addr;
	unsigned char shift_val[DP_LANE_CNT_4] = {0, 4, 0, 4};

	/*lane_num value is used as arry index, so this range 0 ~ 3 */
	dpcd_addr = DPCD_ADJUST_REQUEST_LANE0_1 + (lane_num / 2);

	ret = exynos_dp_read_byte_from_dpcd(dpcd_addr, &buf);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP read adjust request failed\n");
		return -EAGAIN;
	}

	*sw = ((buf >> shift_val[lane_num]) & 0x03);
	*em = ((buf >> shift_val[lane_num]) & 0x0c) >> 2;

	return ret;
}

static int exynos_dp_equalizer_err_link(struct edp_device_info *edp_info)
{
	int ret;

	ret = exynos_dp_training_pattern_dis();
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP training_patter_disable() failed\n");
		edp_info->lt_info.lt_status = DP_LT_FAIL;
	}

	ret = exynos_dp_set_enhanced_mode(edp_info->dpcd_efc);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP set_enhanced_mode() failed\n");
		edp_info->lt_info.lt_status = DP_LT_FAIL;
	}

	return ret;
}

static int exynos_dp_reduce_link_rate(struct edp_device_info *edp_info)
{
	int ret;

	if (edp_info->lane_bw == DP_LANE_BW_2_70) {
		edp_info->lane_bw = DP_LANE_BW_1_62;
		printf("DP Change lane bw to 1.62Gbps\n");
		edp_info->lt_info.lt_status = DP_LT_START;
		ret = EXYNOS_DP_SUCCESS;
	} else {
		ret = exynos_dp_training_pattern_dis();
		if (ret != EXYNOS_DP_SUCCESS)
			printf("DP training_patter_disable() failed\n");

		ret = exynos_dp_set_enhanced_mode(edp_info->dpcd_efc);
		if (ret != EXYNOS_DP_SUCCESS)
			printf("DP set_enhanced_mode() failed\n");

		edp_info->lt_info.lt_status = DP_LT_FAIL;
	}

	return ret;
}

static unsigned int exynos_dp_process_clock_recovery(struct edp_device_info
							*edp_info)
{
	unsigned int ret = EXYNOS_DP_SUCCESS;
	unsigned char lane_stat;
	unsigned char lt_ctl_val[DP_LANE_CNT_4] = {0, };
	unsigned int i;
	unsigned char adj_req_sw;
	unsigned char adj_req_em;
	unsigned char buf[5];

	debug("DP: %s was called\n", __func__);
	mdelay(1);

	ret = exynos_dp_read_dpcd_lane_stat(edp_info, &lane_stat);
	if (ret != EXYNOS_DP_SUCCESS) {
			printf("DP read lane status failed\n");
			edp_info->lt_info.lt_status = DP_LT_FAIL;
			return ret;
	}

	if (lane_stat & DP_LANE_STAT_CR_DONE) {
		debug("DP clock Recovery training succeed\n");
		exynos_dp_set_training_pattern(TRAINING_PTN2);

		for (i = 0; i < edp_info->lane_cnt; i++) {
			ret = exynos_dp_read_dpcd_adj_req(i, &adj_req_sw,
					&adj_req_em);
			if (ret != EXYNOS_DP_SUCCESS) {
				edp_info->lt_info.lt_status = DP_LT_FAIL;
				return ret;
			}

			lt_ctl_val[i] = 0;
			lt_ctl_val[i] = adj_req_em << 3 | adj_req_sw;

			if ((adj_req_sw == VOLTAGE_LEVEL_3)
				|| (adj_req_em == PRE_EMPHASIS_LEVEL_3)) {
				lt_ctl_val[i] |= MAX_DRIVE_CURRENT_REACH_3 |
					MAX_PRE_EMPHASIS_REACH_3;
			}
			exynos_dp_set_lanex_pre_emphasis(lt_ctl_val[i], i);
		}

		buf[0] =  DPCD_SCRAMBLING_DISABLED | DPCD_TRAINING_PATTERN_2;
		buf[1] = lt_ctl_val[0];
		buf[2] = lt_ctl_val[1];
		buf[3] = lt_ctl_val[2];
		buf[4] = lt_ctl_val[3];

		ret = exynos_dp_write_bytes_to_dpcd(
				DPCD_TRAINING_PATTERN_SET, 5, buf);
		if (ret != EXYNOS_DP_SUCCESS) {
			printf("DP write traning pattern1 failed\n");
			edp_info->lt_info.lt_status = DP_LT_FAIL;
			return ret;
		} else
			edp_info->lt_info.lt_status = DP_LT_ET;
	} else {
		for (i = 0; i < edp_info->lane_cnt; i++) {
			lt_ctl_val[i] = exynos_dp_get_lanex_pre_emphasis(i);
				ret = exynos_dp_read_dpcd_adj_req(i,
						&adj_req_sw, &adj_req_em);
			if (ret != EXYNOS_DP_SUCCESS) {
				printf("DP read adj req failed\n");
				edp_info->lt_info.lt_status = DP_LT_FAIL;
				return ret;
			}

			if ((adj_req_sw == VOLTAGE_LEVEL_3) ||
					(adj_req_em == PRE_EMPHASIS_LEVEL_3))
				ret = exynos_dp_reduce_link_rate(edp_info);

			if ((DRIVE_CURRENT_SET_0_GET(lt_ctl_val[i]) ==
						adj_req_sw) &&
				(PRE_EMPHASIS_SET_0_GET(lt_ctl_val[i]) ==
						adj_req_em)) {
				edp_info->lt_info.cr_loop[i]++;
				if (edp_info->lt_info.cr_loop[i] == MAX_CR_LOOP)
					ret = exynos_dp_reduce_link_rate(
							edp_info);
			}

			lt_ctl_val[i] = 0;
			lt_ctl_val[i] = adj_req_em << 3 | adj_req_sw;

			if ((adj_req_sw == VOLTAGE_LEVEL_3) ||
					(adj_req_em == PRE_EMPHASIS_LEVEL_3)) {
				lt_ctl_val[i] |= MAX_DRIVE_CURRENT_REACH_3 |
					MAX_PRE_EMPHASIS_REACH_3;
			}
			exynos_dp_set_lanex_pre_emphasis(lt_ctl_val[i], i);
		}

		ret = exynos_dp_write_bytes_to_dpcd(
				DPCD_TRAINING_LANE0_SET, 4, lt_ctl_val);
		if (ret != EXYNOS_DP_SUCCESS) {
			printf("DP write traning pattern2 failed\n");
			edp_info->lt_info.lt_status = DP_LT_FAIL;
			return ret;
		}
	}

	return ret;
}

static unsigned int exynos_dp_process_equalizer_training(struct edp_device_info
		*edp_info)
{
	unsigned int ret = EXYNOS_DP_SUCCESS;
	unsigned char lane_stat, adj_req_sw, adj_req_em, i;
	unsigned char lt_ctl_val[DP_LANE_CNT_4] = {0,};
	unsigned char interlane_aligned = 0;
	unsigned char f_bw;
	unsigned char f_lane_cnt;
	unsigned char sink_stat;

	mdelay(1);

	ret = exynos_dp_read_dpcd_lane_stat(edp_info, &lane_stat);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP read lane status failed\n");
		edp_info->lt_info.lt_status = DP_LT_FAIL;
		return ret;
	}

	debug("DP lane stat : %x\n", lane_stat);

	if (lane_stat & DP_LANE_STAT_CR_DONE) {
		ret = exynos_dp_read_byte_from_dpcd(DPCD_LN_ALIGN_UPDATED,
				&sink_stat);
		if (ret != EXYNOS_DP_SUCCESS) {
			edp_info->lt_info.lt_status = DP_LT_FAIL;

			return ret;
		}

		interlane_aligned = (sink_stat & DPCD_INTERLANE_ALIGN_DONE);

		for (i = 0; i < edp_info->lane_cnt; i++) {
			ret = exynos_dp_read_dpcd_adj_req(i,
					&adj_req_sw, &adj_req_em);
			if (ret != EXYNOS_DP_SUCCESS) {
				printf("DP read adj req 1 failed\n");
				edp_info->lt_info.lt_status = DP_LT_FAIL;

				return ret;
			}

			lt_ctl_val[i] = 0;
			lt_ctl_val[i] = adj_req_em << 3 | adj_req_sw;

			if ((adj_req_sw == VOLTAGE_LEVEL_3) ||
				(adj_req_em == PRE_EMPHASIS_LEVEL_3)) {
				lt_ctl_val[i] |= MAX_DRIVE_CURRENT_REACH_3;
				lt_ctl_val[i] |= MAX_PRE_EMPHASIS_REACH_3;
			}
		}

		if (((lane_stat&DP_LANE_STAT_CE_DONE) &&
			(lane_stat&DP_LANE_STAT_SYM_LOCK))
			&& (interlane_aligned == DPCD_INTERLANE_ALIGN_DONE)) {
			debug("DP Equalizer training succeed\n");

			f_bw = exynos_dp_get_link_bandwidth();
			f_lane_cnt = exynos_dp_get_lane_count();

			debug("DP final BandWidth : %x\n", f_bw);
			debug("DP final Lane Count : %x\n", f_lane_cnt);

			edp_info->lt_info.lt_status = DP_LT_FINISHED;

			exynos_dp_equalizer_err_link(edp_info);

		} else {
			edp_info->lt_info.ep_loop++;

			if (edp_info->lt_info.ep_loop > MAX_EQ_LOOP) {
				if (edp_info->lane_bw == DP_LANE_BW_2_70) {
					ret = exynos_dp_reduce_link_rate(
							edp_info);
				} else {
					edp_info->lt_info.lt_status =
								DP_LT_FAIL;
					exynos_dp_equalizer_err_link(edp_info);
				}
			} else {
				for (i = 0; i < edp_info->lane_cnt; i++)
					exynos_dp_set_lanex_pre_emphasis(
							lt_ctl_val[i], i);

				ret = exynos_dp_write_bytes_to_dpcd(
					DPCD_TRAINING_LANE0_SET,
					4, lt_ctl_val);
				if (ret != EXYNOS_DP_SUCCESS) {
					printf("DP set lt pattern failed\n");
					edp_info->lt_info.lt_status =
								DP_LT_FAIL;
					exynos_dp_equalizer_err_link(edp_info);
				}
			}
		}
	} else if (edp_info->lane_bw == DP_LANE_BW_2_70) {
		ret = exynos_dp_reduce_link_rate(edp_info);
	} else {
		edp_info->lt_info.lt_status = DP_LT_FAIL;
		exynos_dp_equalizer_err_link(edp_info);
	}

	return ret;
}

static unsigned int exynos_dp_sw_link_training(struct edp_device_info *edp_info)
{
	unsigned int ret = 0;
	int training_finished;

	/* Turn off unnecessary lane */
	if (edp_info->lane_cnt == 1)
		exynos_dp_set_analog_power_down(CH1_BLOCK, 1);

	training_finished = 0;

	edp_info->lt_info.lt_status = DP_LT_START;

	/* Process here */
	while (!training_finished) {
		switch (edp_info->lt_info.lt_status) {
		case DP_LT_START:
			ret = exynos_dp_link_start(edp_info);
			if (ret != EXYNOS_DP_SUCCESS) {
				printf("DP LT:link start failed\n");
				return ret;
			}
			break;
		case DP_LT_CR:
			ret = exynos_dp_process_clock_recovery(edp_info);
			if (ret != EXYNOS_DP_SUCCESS) {
				printf("DP LT:clock recovery failed\n");
				return ret;
			}
			break;
		case DP_LT_ET:
			ret = exynos_dp_process_equalizer_training(edp_info);
			if (ret != EXYNOS_DP_SUCCESS) {
				printf("DP LT:equalizer training failed\n");
				return ret;
			}
			break;
		case DP_LT_FINISHED:
			training_finished = 1;
			break;
		case DP_LT_FAIL:
			return -1;
		}
	}

	return ret;
}

static unsigned int exynos_dp_set_link_train(struct edp_device_info *edp_info)
{
	unsigned int ret;

	exynos_dp_init_training();

	ret = exynos_dp_sw_link_training(edp_info);
	if (ret != EXYNOS_DP_SUCCESS)
		printf("DP dp_sw_link_traning() failed\n");

	return ret;
}

static void exynos_dp_enable_scramble(unsigned int enable)
{
	unsigned char data;

	if (enable) {
		exynos_dp_enable_scrambling(DP_ENABLE);

		exynos_dp_read_byte_from_dpcd(DPCD_TRAINING_PATTERN_SET,
				&data);
		exynos_dp_write_byte_to_dpcd(DPCD_TRAINING_PATTERN_SET,
			(u8)(data & ~DPCD_SCRAMBLING_DISABLED));
	} else {
		exynos_dp_enable_scrambling(DP_DISABLE);
		exynos_dp_read_byte_from_dpcd(DPCD_TRAINING_PATTERN_SET,
				&data);
		exynos_dp_write_byte_to_dpcd(DPCD_TRAINING_PATTERN_SET,
			(u8)(data | DPCD_SCRAMBLING_DISABLED));
	}
}

static unsigned int exynos_dp_config_video(struct edp_device_info *edp_info)
{
	unsigned int ret = 0;
	unsigned int retry_cnt;

	mdelay(1);

	if (edp_info->video_info.master_mode) {
		printf("DP does not support master mode\n");
		return -ENODEV;
	} else {
		/* debug slave */
		exynos_dp_config_video_slave_mode(&edp_info->video_info);
	}

	exynos_dp_set_video_color_format(&edp_info->video_info);

	if (edp_info->video_info.bist_mode) {
		if (exynos_dp_config_video_bist(edp_info) != 0)
			return -1;
	}

	ret = exynos_dp_get_pll_lock_status();
	if (ret != PLL_LOCKED) {
		printf("DP PLL is not locked yet\n");
		return -EIO;
	}

	if (edp_info->video_info.master_mode == 0) {
		retry_cnt = 10;
		while (retry_cnt) {
			ret = exynos_dp_is_slave_video_stream_clock_on();
			if (ret != EXYNOS_DP_SUCCESS) {
				if (retry_cnt == 0) {
					printf("DP stream_clock_on failed\n");
					return ret;
				}
				retry_cnt--;
				mdelay(1);
			} else
				break;
		}
	}

	/* Set to use the register calculated M/N video */
	exynos_dp_set_video_cr_mn(CALCULATED_M, 0, 0);

	/* For video bist, Video timing must be generated by register */
	exynos_dp_set_video_timing_mode(VIDEO_TIMING_FROM_CAPTURE);

	/* Enable video bist */
	if (edp_info->video_info.bist_pattern != COLOR_RAMP &&
		edp_info->video_info.bist_pattern != BALCK_WHITE_V_LINES &&
		edp_info->video_info.bist_pattern != COLOR_SQUARE)
		exynos_dp_enable_video_bist(edp_info->video_info.bist_mode);
	else
		exynos_dp_enable_video_bist(DP_DISABLE);

	/* Disable video mute */
	exynos_dp_enable_video_mute(DP_DISABLE);

	/* Configure video Master or Slave mode */
	exynos_dp_enable_video_master(edp_info->video_info.master_mode);

	/* Enable video */
	exynos_dp_start_video();

	if (edp_info->video_info.master_mode == 0) {
		retry_cnt = 100;
		while (retry_cnt) {
			ret = exynos_dp_is_video_stream_on();
			if (ret != EXYNOS_DP_SUCCESS) {
				if (retry_cnt == 0) {
					printf("DP Timeout of video stream\n");
					return ret;
				}
				retry_cnt--;
				mdelay(5);
			} else
				break;
		}
	}

	return ret;
}

unsigned int exynos_init_dp(void)
{
	unsigned int ret;
	struct edp_device_info *edp_info;
	struct edp_disp_info disp_info;

	edp_info = kzalloc(sizeof(struct edp_device_info), GFP_KERNEL);
	if (!edp_info) {
		debug("failed to allocate edp device object.\n");
		return -EFAULT;
	}

	edp_info = dp_pd->edp_dev_info;
	if (edp_info == NULL) {
		debug("failed to get edp_info data.\n");
		return -EFAULT;
	}
	disp_info = edp_info->disp_info;

	exynos_dp_disp_info(&edp_info->disp_info);

	if (dp_pd->phy_enable)
		dp_pd->phy_enable(1);

	ret = exynos_dp_init_dp();
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP exynos_dp_init_dp() failed\n");
		return ret;
	}

	ret = exynos_dp_handle_edid(edp_info);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("EDP handle_edid fail\n");
		return ret;
	}

	ret = exynos_dp_set_link_train(edp_info);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("DP link training fail\n");
		return ret;
	}

	exynos_dp_enable_scramble(DP_ENABLE);
	exynos_dp_enable_rx_to_enhanced_mode(DP_ENABLE);
	exynos_dp_enable_enhanced_mode(DP_ENABLE);

	exynos_dp_set_link_bandwidth(edp_info->lane_bw);
	exynos_dp_set_lane_count(edp_info->lane_cnt);

	exynos_dp_init_video();
	ret = exynos_dp_config_video(edp_info);
	if (ret != EXYNOS_DP_SUCCESS) {
		printf("Exynos DP init failed\n");
		return ret;
	}

	printf("Exynos DP init done\n");

	return ret;
}

void exynos_set_dp_platform_data(struct exynos_dp_platform_data *pd)
{
	if (pd == NULL) {
		debug("pd is NULL\n");
		return;
	}

	dp_pd = pd;
}

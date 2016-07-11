/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _DDR3_INIT_H
#define _DDR3_INIT_H

#if defined(CONFIG_ARMADA_38X)
#include "ddr3_a38x.h"
#include "ddr3_a38x_mc_static.h"
#include "ddr3_a38x_topology.h"
#endif
#include "ddr3_hws_hw_training.h"
#include "ddr3_hws_sil_training.h"
#include "ddr3_logging_def.h"
#include "ddr3_training_hw_algo.h"
#include "ddr3_training_ip.h"
#include "ddr3_training_ip_centralization.h"
#include "ddr3_training_ip_engine.h"
#include "ddr3_training_ip_flow.h"
#include "ddr3_training_ip_pbs.h"
#include "ddr3_training_ip_prv_if.h"
#include "ddr3_training_ip_static.h"
#include "ddr3_training_leveling.h"
#include "xor.h"

/*
 * MV_DEBUG_INIT need to be defines, otherwise the output of the
 * DDR2 training code is not complete and misleading
 */
#define MV_DEBUG_INIT

#ifdef MV_DEBUG_INIT
#define DEBUG_INIT_S(s)			puts(s)
#define DEBUG_INIT_D(d, l)		printf("%x", d)
#define DEBUG_INIT_D_10(d, l)		printf("%d", d)
#else
#define DEBUG_INIT_S(s)
#define DEBUG_INIT_D(d, l)
#define DEBUG_INIT_D_10(d, l)
#endif

#ifdef MV_DEBUG_INIT_FULL
#define DEBUG_INIT_FULL_S(s)		puts(s)
#define DEBUG_INIT_FULL_D(d, l)		printf("%x", d)
#define DEBUG_INIT_FULL_D_10(d, l)	printf("%d", d)
#define DEBUG_WR_REG(reg, val) \
	{ DEBUG_INIT_S("Write Reg: 0x"); DEBUG_INIT_D((reg), 8); \
	  DEBUG_INIT_S("= "); DEBUG_INIT_D((val), 8); DEBUG_INIT_S("\n"); }
#define DEBUG_RD_REG(reg, val) \
	{ DEBUG_INIT_S("Read  Reg: 0x"); DEBUG_INIT_D((reg), 8); \
	  DEBUG_INIT_S("= "); DEBUG_INIT_D((val), 8); DEBUG_INIT_S("\n"); }
#else
#define DEBUG_INIT_FULL_S(s)
#define DEBUG_INIT_FULL_D(d, l)
#define DEBUG_INIT_FULL_D_10(d, l)
#define DEBUG_WR_REG(reg, val)
#define DEBUG_RD_REG(reg, val)
#endif

#define DEBUG_INIT_FULL_C(s, d, l)			\
	{ DEBUG_INIT_FULL_S(s);				\
	  DEBUG_INIT_FULL_D(d, l);			\
	  DEBUG_INIT_FULL_S("\n"); }
#define DEBUG_INIT_C(s, d, l) \
	{ DEBUG_INIT_S(s); DEBUG_INIT_D(d, l); DEBUG_INIT_S("\n"); }

/*
 * Debug (Enable/Disable modules) and Error report
 */

#ifdef BASIC_DEBUG
#define MV_DEBUG_WL
#define MV_DEBUG_RL
#define MV_DEBUG_DQS_RESULTS
#endif

#ifdef FULL_DEBUG
#define MV_DEBUG_WL
#define MV_DEBUG_RL
#define MV_DEBUG_DQS

#define MV_DEBUG_PBS
#define MV_DEBUG_DFS
#define MV_DEBUG_MAIN_FULL
#define MV_DEBUG_DFS_FULL
#define MV_DEBUG_DQS_FULL
#define MV_DEBUG_RL_FULL
#define MV_DEBUG_WL_FULL
#endif

#if defined(CONFIG_ARMADA_38X)
#include "ddr3_a38x.h"
#include "ddr3_a38x_topology.h"
#endif

/* The following is a list of Marvell status */
#define MV_ERROR	(-1)
#define MV_OK		(0x00)	/* Operation succeeded                   */
#define MV_FAIL		(0x01)	/* Operation failed                      */
#define MV_BAD_VALUE	(0x02)	/* Illegal value (general)               */
#define MV_OUT_OF_RANGE	(0x03)	/* The value is out of range             */
#define MV_BAD_PARAM	(0x04)	/* Illegal parameter in function called  */
#define MV_BAD_PTR	(0x05)	/* Illegal pointer value                 */
#define MV_BAD_SIZE	(0x06)	/* Illegal size                          */
#define MV_BAD_STATE	(0x07)	/* Illegal state of state machine        */
#define MV_SET_ERROR	(0x08)	/* Set operation failed                  */
#define MV_GET_ERROR	(0x09)	/* Get operation failed                  */
#define MV_CREATE_ERROR	(0x0a)	/* Fail while creating an item           */
#define MV_NOT_FOUND	(0x0b)	/* Item not found                        */
#define MV_NO_MORE	(0x0c)	/* No more items found                   */
#define MV_NO_SUCH	(0x0d)	/* No such item                          */
#define MV_TIMEOUT	(0x0e)	/* Time Out                              */
#define MV_NO_CHANGE	(0x0f)	/* Parameter(s) is already in this value */
#define MV_NOT_SUPPORTED (0x10)	/* This request is not support           */
#define MV_NOT_IMPLEMENTED (0x11) /* Request supported but not implemented*/
#define MV_NOT_INITIALIZED (0x12) /* The item is not initialized          */
#define MV_NO_RESOURCE	(0x13)	/* Resource not available (memory ...)   */
#define MV_FULL		(0x14)	/* Item is full (Queue or table etc...)  */
#define MV_EMPTY	(0x15)	/* Item is empty (Queue or table etc...) */
#define MV_INIT_ERROR	(0x16)	/* Error occurred while INIT process      */
#define MV_HW_ERROR	(0x17)	/* Hardware error                        */
#define MV_TX_ERROR	(0x18)	/* Transmit operation not succeeded      */
#define MV_RX_ERROR	(0x19)	/* Recieve operation not succeeded       */
#define MV_NOT_READY	(0x1a)	/* The other side is not ready yet       */
#define MV_ALREADY_EXIST (0x1b)	/* Tried to create existing item         */
#define MV_OUT_OF_CPU_MEM   (0x1c) /* Cpu memory allocation failed.      */
#define MV_NOT_STARTED	(0x1d)	/* Not started yet                       */
#define MV_BUSY		(0x1e)	/* Item is busy.                         */
#define MV_TERMINATE	(0x1f)	/* Item terminates it's work.            */
#define MV_NOT_ALIGNED	(0x20)	/* Wrong alignment                       */
#define MV_NOT_ALLOWED	(0x21)	/* Operation NOT allowed                 */
#define MV_WRITE_PROTECT (0x22)	/* Write protected                       */
#define MV_INVALID	(int)(-1)

/* For checking function return values */
#define CHECK_STATUS(orig_func)		\
	{				\
		int status;		\
		status = orig_func;	\
		if (MV_OK != status)	\
			return status;	\
	}

enum log_level  {
	MV_LOG_LEVEL_0,
	MV_LOG_LEVEL_1,
	MV_LOG_LEVEL_2,
	MV_LOG_LEVEL_3
};

/* Globals */
extern u8 debug_training;
extern u8 is_reg_dump;
extern u8 generic_init_controller;
extern u32 freq_val[];
extern u32 is_pll_old;
extern struct cl_val_per_freq cas_latency_table[];
extern struct pattern_info pattern_table[];
extern struct cl_val_per_freq cas_write_latency_table[];
extern u8 debug_training;
extern u8 debug_centralization, debug_training_ip, debug_training_bist,
	debug_pbs, debug_training_static, debug_leveling;
extern u32 pipe_multicast_mask;
extern struct hws_tip_config_func_db config_func_info[];
extern u8 cs_mask_reg[];
extern u8 twr_mask_table[];
extern u8 cl_mask_table[];
extern u8 cwl_mask_table[];
extern u16 rfc_table[];
extern u32 speed_bin_table_t_rc[];
extern u32 speed_bin_table_t_rcd_t_rp[];
extern u32 ck_delay, ck_delay_16;

extern u32 g_zpri_data;
extern u32 g_znri_data;
extern u32 g_zpri_ctrl;
extern u32 g_znri_ctrl;
extern u32 g_zpodt_data;
extern u32 g_znodt_data;
extern u32 g_zpodt_ctrl;
extern u32 g_znodt_ctrl;
extern u32 g_dic;
extern u32 g_odt_config;
extern u32 g_rtt_nom;

extern u8 debug_training_access;
extern u8 debug_training_a38x;
extern u32 first_active_if;
extern enum hws_ddr_freq init_freq;
extern u32 delay_enable, ck_delay, ck_delay_16, ca_delay;
extern u32 mask_tune_func;
extern u32 rl_version;
extern int rl_mid_freq_wa;
extern u8 calibration_update_control; /* 2 external only, 1 is internal only */
extern enum hws_ddr_freq medium_freq;

extern u32 ck_delay, ck_delay_16;
extern enum hws_result training_result[MAX_STAGE_LIMIT][MAX_INTERFACE_NUM];
extern u32 first_active_if;
extern u32 mask_tune_func;
extern u32 freq_val[];
extern enum hws_ddr_freq init_freq;
extern enum hws_ddr_freq low_freq;
extern enum hws_ddr_freq medium_freq;
extern u8 generic_init_controller;
extern enum auto_tune_stage training_stage;
extern u32 is_pll_before_init;
extern u32 is_adll_calib_before_init;
extern u32 is_dfs_in_init;
extern int wl_debug_delay;
extern u32 silicon_delay[HWS_MAX_DEVICE_NUM];
extern u32 p_finger;
extern u32 n_finger;
extern u32 freq_val[DDR_FREQ_LIMIT];
extern u32 start_pattern, end_pattern;
extern u32 phy_reg0_val;
extern u32 phy_reg1_val;
extern u32 phy_reg2_val;
extern u32 phy_reg3_val;
extern enum hws_pattern sweep_pattern;
extern enum hws_pattern pbs_pattern;
extern u8 is_rzq6;
extern u32 znri_data_phy_val;
extern u32 zpri_data_phy_val;
extern u32 znri_ctrl_phy_val;
extern u32 zpri_ctrl_phy_val;
extern u8 debug_training_access;
extern u32 finger_test, p_finger_start, p_finger_end, n_finger_start,
	n_finger_end, p_finger_step, n_finger_step;
extern u32 mode2_t;
extern u32 xsb_validate_type;
extern u32 xsb_validation_base_address;
extern u32 odt_additional;
extern u32 debug_mode;
extern u32 delay_enable;
extern u32 ca_delay;
extern u32 debug_dunit;
extern u32 clamp_tbl[];
extern u32 freq_mask[HWS_MAX_DEVICE_NUM][DDR_FREQ_LIMIT];
extern u32 start_pattern, end_pattern;

extern u32 maxt_poll_tries;
extern u32 is_bist_reset_bit;
extern u8 debug_training_bist;

extern u8 vref_window_size[MAX_INTERFACE_NUM][MAX_BUS_NUM];
extern u32 debug_mode;
extern u32 effective_cs;
extern int ddr3_tip_centr_skip_min_win_check;
extern u32 *dq_map_table;
extern enum auto_tune_stage training_stage;
extern u8 debug_centralization;

extern u32 delay_enable;
extern u32 start_pattern, end_pattern;
extern u32 freq_val[DDR_FREQ_LIMIT];
extern u8 debug_training_hw_alg;
extern enum auto_tune_stage training_stage;

extern u8 debug_training_ip;
extern enum hws_result training_result[MAX_STAGE_LIMIT][MAX_INTERFACE_NUM];
extern enum auto_tune_stage training_stage;
extern u32 effective_cs;

extern u8 debug_leveling;
extern enum hws_result training_result[MAX_STAGE_LIMIT][MAX_INTERFACE_NUM];
extern enum auto_tune_stage training_stage;
extern u32 rl_version;
extern struct cl_val_per_freq cas_latency_table[];
extern u32 start_xsb_offset;
extern u32 debug_mode;
extern u32 odt_config;
extern u32 effective_cs;
extern u32 phy_reg1_val;

extern u8 debug_pbs;
extern u32 effective_cs;
extern u16 mask_results_dq_reg_map[];
extern enum hws_ddr_freq medium_freq;
extern u32 freq_val[];
extern enum hws_result training_result[MAX_STAGE_LIMIT][MAX_INTERFACE_NUM];
extern enum auto_tune_stage training_stage;
extern u32 debug_mode;
extern u32 *dq_map_table;

extern u32 vref;
extern struct cl_val_per_freq cas_latency_table[];
extern u32 target_freq;
extern struct hws_tip_config_func_db config_func_info[HWS_MAX_DEVICE_NUM];
extern u32 clamp_tbl[];
extern u32 init_freq;
/* list of allowed frequency listed in order of enum hws_ddr_freq */
extern u32 freq_val[];
extern u8 debug_training_static;
extern u32 first_active_if;

/* Prototypes */
int ddr3_tip_enable_init_sequence(u32 dev_num);

int ddr3_tip_init_a38x(u32 dev_num, u32 board_id);

int ddr3_hws_hw_training(void);
int ddr3_silicon_pre_init(void);
int ddr3_silicon_post_init(void);
int ddr3_post_run_alg(void);
int ddr3_if_ecc_enabled(void);
void ddr3_new_tip_ecc_scrub(void);

void ddr3_print_version(void);
void ddr3_new_tip_dlb_config(void);
struct hws_topology_map *ddr3_get_topology_map(void);

int ddr3_if_ecc_enabled(void);
int ddr3_tip_reg_write(u32 dev_num, u32 reg_addr, u32 data);
int ddr3_tip_reg_read(u32 dev_num, u32 reg_addr, u32 *data, u32 reg_mask);
int ddr3_silicon_get_ddr_target_freq(u32 *ddr_freq);
int ddr3_tip_a38x_get_freq_config(u8 dev_num, enum hws_ddr_freq freq,
				  struct hws_tip_freq_config_info
				  *freq_config_info);
int ddr3_a38x_update_topology_map(u32 dev_num,
				  struct hws_topology_map *topology_map);
int ddr3_tip_a38x_get_init_freq(int dev_num, enum hws_ddr_freq *freq);
int ddr3_tip_a38x_get_medium_freq(int dev_num, enum hws_ddr_freq *freq);
int ddr3_tip_a38x_if_read(u8 dev_num, enum hws_access_type interface_access,
			  u32 if_id, u32 reg_addr, u32 *data, u32 mask);
int ddr3_tip_a38x_if_write(u8 dev_num, enum hws_access_type interface_access,
			   u32 if_id, u32 reg_addr, u32 data, u32 mask);
int ddr3_tip_a38x_get_device_info(u8 dev_num,
				  struct ddr3_device_info *info_ptr);

int ddr3_tip_init_a38x(u32 dev_num, u32 board_id);

int print_adll(u32 dev_num, u32 adll[MAX_INTERFACE_NUM * MAX_BUS_NUM]);
int ddr3_tip_restore_dunit_regs(u32 dev_num);
void print_topology(struct hws_topology_map *topology_db);

u32 mv_board_id_get(void);

int ddr3_load_topology_map(void);
int ddr3_tip_init_specific_reg_config(u32 dev_num,
				      struct reg_data *reg_config_arr);
u32 ddr3_tip_get_init_freq(void);
void ddr3_hws_set_log_level(enum ddr_lib_debug_block block, u8 level);
int ddr3_tip_tune_training_params(u32 dev_num,
				  struct tune_train_params *params);
void get_target_freq(u32 freq_mode, u32 *ddr_freq, u32 *hclk_ps);
int ddr3_fast_path_dynamic_cs_size_config(u32 cs_ena);
void ddr3_fast_path_static_cs_size_config(u32 cs_ena);
u32 ddr3_get_device_width(u32 cs);
u32 mv_board_id_index_get(u32 board_id);
u32 mv_board_id_get(void);
u32 ddr3_get_bus_width(void);
void ddr3_set_log_level(u32 n_log_level);
int ddr3_calc_mem_cs_size(u32 cs, u32 *cs_size);

int hws_ddr3_cs_base_adr_calc(u32 if_id, u32 cs, u32 *cs_base_addr);

int ddr3_tip_print_pbs_result(u32 dev_num, u32 cs_num, enum pbs_dir pbs_mode);
int ddr3_tip_clean_pbs_result(u32 dev_num, enum pbs_dir pbs_mode);

int ddr3_tip_static_round_trip_arr_build(u32 dev_num,
					 struct trip_delay_element *table_ptr,
					 int is_wl, u32 *round_trip_delay_arr);

u32 hws_ddr3_tip_max_cs_get(void);

/*
 * Accessor functions for the registers
 */
static inline void reg_write(u32 addr, u32 val)
{
	writel(val, INTER_REGS_BASE + addr);
}

static inline u32 reg_read(u32 addr)
{
	return readl(INTER_REGS_BASE + addr);
}

static inline void reg_bit_set(u32 addr, u32 mask)
{
	setbits_le32(INTER_REGS_BASE + addr, mask);
}

static inline void reg_bit_clr(u32 addr, u32 mask)
{
	clrbits_le32(INTER_REGS_BASE + addr, mask);
}

#endif /* _DDR3_INIT_H */

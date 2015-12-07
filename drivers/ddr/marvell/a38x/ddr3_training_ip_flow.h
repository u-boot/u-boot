/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _DDR3_TRAINING_IP_FLOW_H_
#define _DDR3_TRAINING_IP_FLOW_H_

#include "ddr3_training_ip.h"
#include "ddr3_training_ip_pbs.h"

#define MRS0_CMD			0x3
#define MRS1_CMD			0x4
#define MRS2_CMD			0x8
#define MRS3_CMD			0x9

/*
 * Definitions of INTERFACE registers
 */

#define READ_BUFFER_SELECT		0x14a4

/*
 * Definitions of PHY registers
 */

#define KILLER_PATTERN_LENGTH		32
#define EXT_ACCESS_BURST_LENGTH		8

#define IS_ACTIVE(if_mask , if_id) \
	((if_mask) & (1 << (if_id)))
#define VALIDATE_ACTIVE(mask, id)		\
	{					\
	if (IS_ACTIVE(mask, id) == 0)		\
		continue;			\
	}

#define GET_TOPOLOGY_NUM_OF_BUSES() \
	(ddr3_get_topology_map()->num_of_bus_per_interface)

#define DDR3_IS_ECC_PUP3_MODE(if_mask) \
	(((if_mask) == 0xb) ? 1 : 0)
#define DDR3_IS_ECC_PUP4_MODE(if_mask) \
	(((((if_mask) & 0x10) == 0)) ? 0 : 1)
#define DDR3_IS_16BIT_DRAM_MODE(mask) \
	(((((mask) & 0x4) == 0)) ? 1 : 0)

#define MEGA				1000000
#define BUS_WIDTH_IN_BITS		8

/*
 * DFX address Space
 * Table 2: DFX address space
 * Address Bits   Value   Description
 * [31 : 20]   0x? DFX base address bases PCIe mapping
 * [19 : 15]   0...Number_of_client-1   Client Index inside pipe.
 *             See also Table 1 Multi_cast = 29 Broadcast = 28
 * [14 : 13]   2'b01   Access to Client Internal Register
 * [12 : 0]   Client Internal Register offset   See related Client Registers
 * [14 : 13]   2'b00   Access to Ram Wrappers Internal Register
 * [12 : 6]   0 Number_of_rams-1   Ram Index inside Client
 * [5 : 0]   Ram Wrapper Internal Register offset   See related Ram Wrappers
 * Registers
 */

/* nsec */
#define  TREFI_LOW				7800
#define  TREFI_HIGH				3900

#define  TR2R_VALUE_REG				0x180
#define  TR2R_MASK_REG				0x180
#define  TRFC_MASK_REG				0x7f
#define  TR2W_MASK_REG				0x600
#define  TW2W_HIGH_VALUE_REG			0x1800
#define  TW2W_HIGH_MASK_REG			0xf800
#define  TRFC_HIGH_VALUE_REG			0x20000
#define  TRFC_HIGH_MASK_REG			0x70000
#define  TR2R_HIGH_VALUE_REG			0x0
#define  TR2R_HIGH_MASK_REG			0x380000
#define  TMOD_VALUE_REG				0x16000000
#define  TMOD_MASK_REG				0x1e000000
#define  T_VALUE_REG				0x40000000
#define  T_MASK_REG				0xc0000000
#define  AUTO_ZQC_TIMING			15384
#define  WRITE_XBAR_PORT1			0xc03f8077
#define  READ_XBAR_PORT1			0xc03f8073
#define  DISABLE_DDR_TUNING_DATA		0x02294285
#define  ENABLE_DDR_TUNING_DATA			0x12294285

#define ODPG_TRAINING_STATUS_REG		0x18488
#define ODPG_TRAINING_TRIGGER_REG		0x1030
#define ODPG_STATUS_DONE_REG			0x16fc
#define ODPG_ENABLE_REG				0x186d4
#define ODPG_ENABLE_OFFS			0
#define ODPG_DISABLE_OFFS			8

#define ODPG_TRAINING_CONTROL_REG		0x1034
#define ODPG_OBJ1_OPCODE_REG			0x103c
#define ODPG_OBJ1_ITER_CNT_REG			0x10b4
#define CALIB_OBJ_PRFA_REG			0x10c4
#define ODPG_WRITE_LEVELING_DONE_CNTR_REG	0x10f8
#define ODPG_WRITE_READ_MODE_ENABLE_REG		0x10fc
#define TRAINING_OPCODE_1_REG			0x10b4
#define SDRAM_CONFIGURATION_REG			0x1400
#define DDR_CONTROL_LOW_REG			0x1404
#define SDRAM_TIMING_LOW_REG			0x1408
#define SDRAM_TIMING_HIGH_REG			0x140c
#define SDRAM_ACCESS_CONTROL_REG		0x1410
#define SDRAM_OPEN_PAGE_CONTROL_REG		0x1414
#define SDRAM_OPERATION_REG			0x1418
#define DUNIT_CONTROL_HIGH_REG			0x1424
#define ODT_TIMING_LOW				0x1428
#define DDR_TIMING_REG				0x142c
#define ODT_TIMING_HI_REG			0x147c
#define SDRAM_INIT_CONTROL_REG			0x1480
#define SDRAM_ODT_CONTROL_HIGH_REG		0x1498
#define DUNIT_ODT_CONTROL_REG			0x149c
#define READ_BUFFER_SELECT_REG			0x14a4
#define DUNIT_MMASK_REG				0x14b0
#define CALIB_MACHINE_CTRL_REG			0x14cc
#define DRAM_DLL_TIMING_REG			0x14e0
#define DRAM_ZQ_INIT_TIMIMG_REG			0x14e4
#define DRAM_ZQ_TIMING_REG			0x14e8
#define DFS_REG					0x1528
#define READ_DATA_SAMPLE_DELAY			0x1538
#define READ_DATA_READY_DELAY			0x153c
#define TRAINING_REG				0x15b0
#define TRAINING_SW_1_REG			0x15b4
#define TRAINING_SW_2_REG			0x15b8
#define TRAINING_PATTERN_BASE_ADDRESS_REG	0x15bc
#define TRAINING_DBG_1_REG			0x15c0
#define TRAINING_DBG_2_REG			0x15c4
#define TRAINING_DBG_3_REG			0x15c8
#define RANK_CTRL_REG				0x15e0
#define TIMING_REG				0x15e4
#define DRAM_PHY_CONFIGURATION			0x15ec
#define MR0_REG					0x15d0
#define MR1_REG					0x15d4
#define MR2_REG					0x15d8
#define MR3_REG					0x15dc
#define TIMING_REG				0x15e4
#define ODPG_CTRL_CONTROL_REG			0x1600
#define ODPG_DATA_CONTROL_REG			0x1630
#define ODPG_PATTERN_ADDR_OFFSET_REG		0x1638
#define ODPG_DATA_BUF_SIZE_REG			0x163c
#define PHY_LOCK_STATUS_REG			0x1674
#define PHY_REG_FILE_ACCESS			0x16a0
#define TRAINING_WRITE_LEVELING_REG		0x16ac
#define ODPG_PATTERN_ADDR_REG			0x16b0
#define ODPG_PATTERN_DATA_HI_REG		0x16b4
#define ODPG_PATTERN_DATA_LOW_REG		0x16b8
#define ODPG_BIST_LAST_FAIL_ADDR_REG		0x16bc
#define ODPG_BIST_DATA_ERROR_COUNTER_REG	0x16c0
#define ODPG_BIST_FAILED_DATA_HI_REG		0x16c4
#define ODPG_BIST_FAILED_DATA_LOW_REG		0x16c8
#define ODPG_WRITE_DATA_ERROR_REG		0x16cc
#define CS_ENABLE_REG				0x16d8
#define WR_LEVELING_DQS_PATTERN_REG		0x16dc

#define ODPG_BIST_DONE				0x186d4
#define ODPG_BIST_DONE_BIT_OFFS			0
#define ODPG_BIST_DONE_BIT_VALUE		0

#define RESULT_CONTROL_BYTE_PUP_0_REG		0x1830
#define RESULT_CONTROL_BYTE_PUP_1_REG		0x1834
#define RESULT_CONTROL_BYTE_PUP_2_REG		0x1838
#define RESULT_CONTROL_BYTE_PUP_3_REG		0x183c
#define RESULT_CONTROL_BYTE_PUP_4_REG		0x18b0

#define RESULT_CONTROL_PUP_0_BIT_0_REG		0x18b4
#define RESULT_CONTROL_PUP_0_BIT_1_REG		0x18b8
#define RESULT_CONTROL_PUP_0_BIT_2_REG		0x18bc
#define RESULT_CONTROL_PUP_0_BIT_3_REG		0x18c0
#define RESULT_CONTROL_PUP_0_BIT_4_REG		0x18c4
#define RESULT_CONTROL_PUP_0_BIT_5_REG		0x18c8
#define RESULT_CONTROL_PUP_0_BIT_6_REG		0x18cc
#define RESULT_CONTROL_PUP_0_BIT_7_REG		0x18f0
#define RESULT_CONTROL_PUP_1_BIT_0_REG		0x18f4
#define RESULT_CONTROL_PUP_1_BIT_1_REG		0x18f8
#define RESULT_CONTROL_PUP_1_BIT_2_REG		0x18fc
#define RESULT_CONTROL_PUP_1_BIT_3_REG		0x1930
#define RESULT_CONTROL_PUP_1_BIT_4_REG		0x1934
#define RESULT_CONTROL_PUP_1_BIT_5_REG		0x1938
#define RESULT_CONTROL_PUP_1_BIT_6_REG		0x193c
#define RESULT_CONTROL_PUP_1_BIT_7_REG		0x19b0
#define RESULT_CONTROL_PUP_2_BIT_0_REG		0x19b4
#define RESULT_CONTROL_PUP_2_BIT_1_REG		0x19b8
#define RESULT_CONTROL_PUP_2_BIT_2_REG		0x19bc
#define RESULT_CONTROL_PUP_2_BIT_3_REG		0x19c0
#define RESULT_CONTROL_PUP_2_BIT_4_REG		0x19c4
#define RESULT_CONTROL_PUP_2_BIT_5_REG		0x19c8
#define RESULT_CONTROL_PUP_2_BIT_6_REG		0x19cc
#define RESULT_CONTROL_PUP_2_BIT_7_REG		0x19f0
#define RESULT_CONTROL_PUP_3_BIT_0_REG		0x19f4
#define RESULT_CONTROL_PUP_3_BIT_1_REG		0x19f8
#define RESULT_CONTROL_PUP_3_BIT_2_REG		0x19fc
#define RESULT_CONTROL_PUP_3_BIT_3_REG		0x1a30
#define RESULT_CONTROL_PUP_3_BIT_4_REG		0x1a34
#define RESULT_CONTROL_PUP_3_BIT_5_REG		0x1a38
#define RESULT_CONTROL_PUP_3_BIT_6_REG		0x1a3c
#define RESULT_CONTROL_PUP_3_BIT_7_REG		0x1ab0
#define RESULT_CONTROL_PUP_4_BIT_0_REG		0x1ab4
#define RESULT_CONTROL_PUP_4_BIT_1_REG		0x1ab8
#define RESULT_CONTROL_PUP_4_BIT_2_REG		0x1abc
#define RESULT_CONTROL_PUP_4_BIT_3_REG		0x1ac0
#define RESULT_CONTROL_PUP_4_BIT_4_REG		0x1ac4
#define RESULT_CONTROL_PUP_4_BIT_5_REG		0x1ac8
#define RESULT_CONTROL_PUP_4_BIT_6_REG		0x1acc
#define RESULT_CONTROL_PUP_4_BIT_7_REG		0x1af0

#define WL_PHY_REG				0x0
#define WRITE_CENTRALIZATION_PHY_REG		0x1
#define RL_PHY_REG				0x2
#define READ_CENTRALIZATION_PHY_REG		0x3
#define PBS_RX_PHY_REG				0x50
#define PBS_TX_PHY_REG				0x10
#define PHY_CONTROL_PHY_REG			0x90
#define BW_PHY_REG				0x92
#define RATE_PHY_REG				0x94
#define CMOS_CONFIG_PHY_REG			0xa2
#define PAD_ZRI_CALIB_PHY_REG			0xa4
#define PAD_ODT_CALIB_PHY_REG			0xa6
#define PAD_CONFIG_PHY_REG			0xa8
#define PAD_PRE_DISABLE_PHY_REG			0xa9
#define TEST_ADLL_REG				0xbf
#define CSN_IOB_VREF_REG(cs)			(0xdb + (cs * 12))
#define CSN_IO_BASE_VREF_REG(cs)		(0xd0 + (cs * 12))

#define RESULT_DB_PHY_REG_ADDR			0xc0
#define RESULT_DB_PHY_REG_RX_OFFSET		5
#define RESULT_DB_PHY_REG_TX_OFFSET		0

/* TBD - for NP5 use only CS 0 */
#define PHY_WRITE_DELAY(cs)			WL_PHY_REG
/*( ( _cs_ == 0 ) ? 0x0 : 0x4 )*/
/* TBD - for NP5 use only CS 0 */
#define PHY_READ_DELAY(cs)			RL_PHY_REG

#define DDR0_ADDR_1				0xf8258
#define DDR0_ADDR_2				0xf8254
#define DDR1_ADDR_1				0xf8270
#define DDR1_ADDR_2				0xf8270
#define DDR2_ADDR_1				0xf825c
#define DDR2_ADDR_2				0xf825c
#define DDR3_ADDR_1				0xf8264
#define DDR3_ADDR_2				0xf8260
#define DDR4_ADDR_1				0xf8274
#define DDR4_ADDR_2				0xf8274

#define GENERAL_PURPOSE_RESERVED0_REG		0x182e0

#define GET_BLOCK_ID_MAX_FREQ(dev_num, block_id)	800000
#define CS0_RD_LVL_REF_DLY_OFFS			0
#define CS0_RD_LVL_REF_DLY_LEN			0
#define CS0_RD_LVL_PH_SEL_OFFS			0
#define CS0_RD_LVL_PH_SEL_LEN			0

#define CS_REGISTER_ADDR_OFFSET			4
#define CALIBRATED_OBJECTS_REG_ADDR_OFFSET	0x10

#define MAX_POLLING_ITERATIONS			100000

#define PHASE_REG_OFFSET			32
#define NUM_BYTES_IN_BURST			31
#define NUM_OF_CS				4
#define CS_REG_VALUE(cs_num)			(cs_mask_reg[cs_num])
#define ADLL_LENGTH				32

struct write_supp_result {
	enum hws_wl_supp stage;
	int is_pup_fail;
};

struct page_element {
	enum hws_page_size page_size_8bit;
	/* page size in 8 bits bus width */
	enum hws_page_size page_size_16bit;
	/* page size in 16 bits bus width */
	u32 ui_page_mask;
	/* Mask used in register */
};

int ddr3_tip_write_leveling_static_config(u32 dev_num, u32 if_id,
					  enum hws_ddr_freq frequency,
					  u32 *round_trip_delay_arr);
int ddr3_tip_read_leveling_static_config(u32 dev_num, u32 if_id,
					 enum hws_ddr_freq frequency,
					 u32 *total_round_trip_delay_arr);
int ddr3_tip_if_write(u32 dev_num, enum hws_access_type interface_access,
		      u32 if_id, u32 reg_addr, u32 data_value, u32 mask);
int ddr3_tip_if_polling(u32 dev_num, enum hws_access_type access_type,
			u32 if_id, u32 exp_value, u32 mask, u32 offset,
			u32 poll_tries);
int ddr3_tip_if_read(u32 dev_num, enum hws_access_type interface_access,
		     u32 if_id, u32 reg_addr, u32 *data, u32 mask);
int ddr3_tip_bus_read_modify_write(u32 dev_num,
				   enum hws_access_type access_type,
				   u32 if_id, u32 phy_id,
				   enum hws_ddr_phy phy_type,
				   u32 reg_addr, u32 data_value, u32 reg_mask);
int ddr3_tip_bus_read(u32 dev_num, u32 if_id, enum hws_access_type phy_access,
		      u32 phy_id, enum hws_ddr_phy phy_type, u32 reg_addr,
		      u32 *data);
int ddr3_tip_bus_write(u32 dev_num, enum hws_access_type e_interface_access,
		       u32 if_id, enum hws_access_type e_phy_access, u32 phy_id,
		       enum hws_ddr_phy e_phy_type, u32 reg_addr,
		       u32 data_value);
int ddr3_tip_freq_set(u32 dev_num, enum hws_access_type e_access, u32 if_id,
		      enum hws_ddr_freq memory_freq);
int ddr3_tip_adjust_dqs(u32 dev_num);
int ddr3_tip_init_controller(u32 dev_num);
int ddr3_tip_ext_read(u32 dev_num, u32 if_id, u32 reg_addr,
		      u32 num_of_bursts, u32 *addr);
int ddr3_tip_ext_write(u32 dev_num, u32 if_id, u32 reg_addr,
		       u32 num_of_bursts, u32 *addr);
int ddr3_tip_dynamic_read_leveling(u32 dev_num, u32 ui_freq);
int ddr3_tip_legacy_dynamic_read_leveling(u32 dev_num);
int ddr3_tip_dynamic_per_bit_read_leveling(u32 dev_num, u32 ui_freq);
int ddr3_tip_legacy_dynamic_write_leveling(u32 dev_num);
int ddr3_tip_dynamic_write_leveling(u32 dev_num);
int ddr3_tip_dynamic_write_leveling_supp(u32 dev_num);
int ddr3_tip_static_init_controller(u32 dev_num);
int ddr3_tip_configure_phy(u32 dev_num);
int ddr3_tip_load_pattern_to_odpg(u32 dev_num, enum hws_access_type access_type,
				  u32 if_id, enum hws_pattern pattern,
				  u32 load_addr);
int ddr3_tip_load_pattern_to_mem(u32 dev_num, enum hws_pattern e_pattern);
int ddr3_tip_configure_odpg(u32 dev_num, enum hws_access_type access_type,
			    u32 if_id, enum hws_dir direction, u32 tx_phases,
			    u32 tx_burst_size, u32 rx_phases,
			    u32 delay_between_burst, u32 rd_mode, u32 cs_num,
			    u32 addr_stress_jump, u32 single_pattern);
int ddr3_tip_set_atr(u32 dev_num, u32 flag_id, u32 value);
int ddr3_tip_write_mrs_cmd(u32 dev_num, u32 *cs_mask_arr, u32 cmd, u32 data,
			   u32 mask);
int ddr3_tip_write_cs_result(u32 dev_num, u32 offset);
int ddr3_tip_get_first_active_if(u8 dev_num, u32 interface_mask, u32 *if_id);
int ddr3_tip_reset_fifo_ptr(u32 dev_num);
int read_pup_value(int pup_values[MAX_INTERFACE_NUM * MAX_BUS_NUM],
		   int reg_addr, u32 mask);
int read_adll_value(u32 pup_values[MAX_INTERFACE_NUM * MAX_BUS_NUM],
		    int reg_addr, u32 mask);
int write_adll_value(u32 pup_values[MAX_INTERFACE_NUM * MAX_BUS_NUM],
		     int reg_addr);
int ddr3_tip_tune_training_params(u32 dev_num,
				  struct tune_train_params *params);

#endif /* _DDR3_TRAINING_IP_FLOW_H_ */

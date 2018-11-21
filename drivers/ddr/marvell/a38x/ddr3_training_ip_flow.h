/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _DDR3_TRAINING_IP_FLOW_H_
#define _DDR3_TRAINING_IP_FLOW_H_

#include "ddr3_training_ip.h"
#include "ddr3_training_ip_pbs.h"
#include "mv_ddr_regs.h"

#define KILLER_PATTERN_LENGTH		32
#define EXT_ACCESS_BURST_LENGTH		8

#define IS_ACTIVE(mask, id) \
	((mask) & (1 << (id)))

#define VALIDATE_ACTIVE(mask, id)		\
	{					\
	if (IS_ACTIVE(mask, id) == 0)		\
		continue;			\
	}

#define IS_IF_ACTIVE(if_mask, if_id) \
	((if_mask) & (1 << (if_id)))

#define VALIDATE_IF_ACTIVE(mask, id)		\
	{					\
	if (IS_IF_ACTIVE(mask, id) == 0)	\
		continue;			\
	}

#define IS_BUS_ACTIVE(if_mask , if_id) \
	(((if_mask) >> (if_id)) & 1)

#define VALIDATE_BUS_ACTIVE(mask, id)		\
	{					\
	if (IS_BUS_ACTIVE(mask, id) == 0)	\
		continue;			\
	}

#define DDR3_IS_ECC_PUP3_MODE(if_mask) \
	(((if_mask) == BUS_MASK_16BIT_ECC_PUP3) ? 1 : 0)

#define DDR3_IS_ECC_PUP4_MODE(if_mask) \
	((if_mask == BUS_MASK_32BIT_ECC || if_mask == BUS_MASK_16BIT_ECC) ? 1 : 0)

#define DDR3_IS_16BIT_DRAM_MODE(mask) \
	((mask == BUS_MASK_16BIT || mask == BUS_MASK_16BIT_ECC || mask == BUS_MASK_16BIT_ECC_PUP3) ? 1 : 0)

#define DDR3_IS_ECC_PUP8_MODE(if_mask) \
	((if_mask == MV_DDR_32BIT_ECC_PUP8_BUS_MASK || if_mask == MV_DDR_64BIT_ECC_PUP8_BUS_MASK) ? 1 : 0)

#define MV_DDR_IS_64BIT_DRAM_MODE(mask) \
	((((mask) & MV_DDR_64BIT_BUS_MASK) == MV_DDR_64BIT_BUS_MASK) || \
	(((mask) & MV_DDR_64BIT_ECC_PUP8_BUS_MASK) == MV_DDR_64BIT_ECC_PUP8_BUS_MASK) ? 1 : 0)

#define MV_DDR_IS_32BIT_IN_64BIT_DRAM_MODE(mask, octets_per_if_num/* FIXME: get from ATF */) \
	((octets_per_if_num == 9/* FIXME: get from ATF */) && \
	((mask == BUS_MASK_32BIT) || \
	(mask == MV_DDR_32BIT_ECC_PUP8_BUS_MASK)) ? 1 : 0)

#define MV_DDR_IS_HALF_BUS_DRAM_MODE(mask, octets_per_if_num/* FIXME: get from ATF */) \
	(MV_DDR_IS_32BIT_IN_64BIT_DRAM_MODE(mask, octets_per_if_num) || DDR3_IS_16BIT_DRAM_MODE(mask))

#define ECC_READ_BUS_0			0
#define ECC_PHY_ACCESS_3		3
#define ECC_PHY_ACCESS_4		4
#define ECC_PHY_ACCESS_8		8
#define MEGA				1000000
#define BUS_WIDTH_IN_BITS		8
#define MAX_POLLING_ITERATIONS		1000000
#define NUM_OF_CS			4
#define ADLL_LENGTH			32

#define GP_RSVD0_REG			0x182e0

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
#define AUTO_ZQC_TIMING				15384

enum mr_number {
	MR_CMD0,
	MR_CMD1,
	MR_CMD2,
	MR_CMD3,
	MR_LAST
};

struct mv_ddr_mr_data {
	u32 cmd;
	u32 reg_addr;
};

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
int mv_ddr_rl_dqs_burst(u32 dev_num, u32 if_id, u32 freq);
int ddr3_tip_legacy_dynamic_read_leveling(u32 dev_num);
int ddr3_tip_dynamic_per_bit_read_leveling(u32 dev_num, u32 ui_freq);
int ddr3_tip_legacy_dynamic_write_leveling(u32 dev_num);
int ddr3_tip_dynamic_write_leveling(u32 dev_num, int phase_remove);
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
int ddr3_tip_write_mrs_cmd(u32 dev_num, u32 *cs_mask_arr, enum mr_number mr_num, u32 data, u32 mask);
int ddr3_tip_write_cs_result(u32 dev_num, u32 offset);
int ddr3_tip_get_first_active_if(u8 dev_num, u32 interface_mask, u32 *if_id);
int ddr3_tip_reset_fifo_ptr(u32 dev_num);
int ddr3_tip_read_pup_value(u32 dev_num,
			    u32 pup_values[MAX_INTERFACE_NUM * MAX_BUS_NUM],
			    int reg_addr, u32 mask);
int ddr3_tip_read_adll_value(u32 dev_num,
			     u32 pup_values[MAX_INTERFACE_NUM * MAX_BUS_NUM],
			     u32 reg_addr, u32 mask);
int ddr3_tip_write_adll_value(u32 dev_num,
			      u32 pup_values[MAX_INTERFACE_NUM * MAX_BUS_NUM],
			      u32 reg_addr);
int ddr3_tip_tune_training_params(u32 dev_num,
				  struct tune_train_params *params);
struct page_element *mv_ddr_page_tbl_get(void);

#endif /* _DDR3_TRAINING_IP_FLOW_H_ */

// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "mv_ddr_topology.h"
#include "mv_ddr_common.h"
#include "mv_ddr_spd.h"
#include "ddr3_init.h"
#include "ddr_topology_def.h"
#include "ddr3_training_ip_db.h"
#include "ddr3_training_ip.h"


unsigned int mv_ddr_cl_calc(unsigned int taa_min, unsigned int tclk)
{
	unsigned int cl = ceil_div(taa_min, tclk);

	return mv_ddr_spd_supported_cl_get(cl);

}

unsigned int mv_ddr_cwl_calc(unsigned int tclk)
{
	unsigned int cwl;

	if (tclk >= 1250)
		cwl = 9;
	else if (tclk >= 1071)
		cwl = 10;
	else if (tclk >= 938)
		cwl = 11;
	else if (tclk >= 833)
		cwl = 12;
	else
		cwl = 0;

	return cwl;
}

struct mv_ddr_topology_map *mv_ddr_topology_map_update(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int octets_per_if_num = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	enum hws_speed_bin speed_bin_index;
	enum hws_ddr_freq freq = DDR_FREQ_LAST;
	unsigned int tclk;
	unsigned char val = 0;
	int i;


	if (tm->interface_params[0].memory_freq == DDR_FREQ_SAR)
		tm->interface_params[0].memory_freq = mv_ddr_init_freq_get();

	if (tm->cfg_src == MV_DDR_CFG_SPD) {
		/* check dram device type */
		val = mv_ddr_spd_dev_type_get(&tm->spd_data);
		if (val != MV_DDR_SPD_DEV_TYPE_DDR4) {
			printf("mv_ddr: unsupported dram device type found\n");
			return NULL;
		}

		/* update topology map with timing data */
		if (mv_ddr_spd_timing_calc(&tm->spd_data, tm->timing_data) > 0) {
			printf("mv_ddr: negative timing data found\n");
			return NULL;
		}

		/* update device width in topology map */
		tm->interface_params[0].bus_width = mv_ddr_spd_dev_width_get(&tm->spd_data);

		/* update die capacity in topology map */
		tm->interface_params[0].memory_size = mv_ddr_spd_die_capacity_get(&tm->spd_data);

		/* update bus bit mask in topology map */
		tm->bus_act_mask = mv_ddr_bus_bit_mask_get();

		/* update cs bit mask in topology map */
		val = mv_ddr_spd_cs_bit_mask_get(&tm->spd_data);
		for (i = 0; i < octets_per_if_num; i++) {
				tm->interface_params[0].as_bus_params[i].cs_bitmask = val;
		}

		/* check dram module type */
		val = mv_ddr_spd_module_type_get(&tm->spd_data);
		switch (val) {
		case MV_DDR_SPD_MODULE_TYPE_UDIMM:
		case MV_DDR_SPD_MODULE_TYPE_SO_DIMM:
		case MV_DDR_SPD_MODULE_TYPE_MINI_UDIMM:
		case MV_DDR_SPD_MODULE_TYPE_72BIT_SO_UDIMM:
		case MV_DDR_SPD_MODULE_TYPE_16BIT_SO_DIMM:
		case MV_DDR_SPD_MODULE_TYPE_32BIT_SO_DIMM:
			break;
		default:
			printf("mv_ddr: unsupported dram module type found\n");
			return NULL;
		}

		/* update mirror bit mask in topology map */
		val = mv_ddr_spd_mem_mirror_get(&tm->spd_data);
		for (i = 0; i < octets_per_if_num; i++) {
				tm->interface_params[0].as_bus_params[i].mirror_enable_bitmask = val << 1;
		}

		tclk = 1000000 / freq_val[tm->interface_params[0].memory_freq];
		/* update cas write latency (cwl) */
		val = mv_ddr_cwl_calc(tclk);
		if (val == 0) {
			printf("mv_ddr: unsupported cas write latency value found\n");
			return NULL;
		}
		tm->interface_params[0].cas_wl = val;

		/* update cas latency (cl) */
		mv_ddr_spd_supported_cls_calc(&tm->spd_data);
		val = mv_ddr_cl_calc(tm->timing_data[MV_DDR_TAA_MIN], tclk);
		if (val == 0) {
			printf("mv_ddr: unsupported cas latency value found\n");
			return NULL;
		}
		tm->interface_params[0].cas_l = val;
	} else if (tm->cfg_src == MV_DDR_CFG_DEFAULT) {
		/* set cas and cas-write latencies per speed bin, if they unset */
		speed_bin_index = tm->interface_params[0].speed_bin_index;
		freq = tm->interface_params[0].memory_freq;

		if (tm->interface_params[0].cas_l == 0)
			tm->interface_params[0].cas_l =
				cas_latency_table[speed_bin_index].cl_val[freq];

		if (tm->interface_params[0].cas_wl == 0)
			tm->interface_params[0].cas_wl =
				cas_write_latency_table[speed_bin_index].cl_val[freq];
	}


	return tm;
}

unsigned short mv_ddr_bus_bit_mask_get(void)
{
	unsigned short pri_and_ext_bus_width = 0x0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int octets_per_if_num = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);

	if (tm->cfg_src == MV_DDR_CFG_SPD) {
		enum mv_ddr_pri_bus_width pri_bus_width = mv_ddr_spd_pri_bus_width_get(&tm->spd_data);
		enum mv_ddr_bus_width_ext bus_width_ext = mv_ddr_spd_bus_width_ext_get(&tm->spd_data);

		switch (pri_bus_width) {
		case MV_DDR_PRI_BUS_WIDTH_16:
			pri_and_ext_bus_width = BUS_MASK_16BIT;
			break;
		case MV_DDR_PRI_BUS_WIDTH_32:
			pri_and_ext_bus_width = BUS_MASK_32BIT;
			break;
		case MV_DDR_PRI_BUS_WIDTH_64:
			pri_and_ext_bus_width = MV_DDR_64BIT_BUS_MASK;
			break;
		default:
			pri_and_ext_bus_width = 0x0;
		}

		if (bus_width_ext == MV_DDR_BUS_WIDTH_EXT_8)
			pri_and_ext_bus_width |= 1 << (octets_per_if_num - 1);
	}

	return pri_and_ext_bus_width;
}

unsigned int mv_ddr_if_bus_width_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int bus_width;

	switch (tm->bus_act_mask) {
	case BUS_MASK_16BIT:
	case BUS_MASK_16BIT_ECC:
	case BUS_MASK_16BIT_ECC_PUP3:
		bus_width = 16;
		break;
	case BUS_MASK_32BIT:
	case BUS_MASK_32BIT_ECC:
	case MV_DDR_32BIT_ECC_PUP8_BUS_MASK:
		bus_width = 32;
		break;
	case MV_DDR_64BIT_BUS_MASK:
	case MV_DDR_64BIT_ECC_PUP8_BUS_MASK:
		bus_width = 64;
		break;
	default:
		printf("mv_ddr: unsupported bus active mask parameter found\n");
		bus_width = 0;
	}

	return bus_width;
}

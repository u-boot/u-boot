/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _DDR3_HWS_HW_TRAINING_H
#define _DDR3_HWS_HW_TRAINING_H

/* struct used for DLB configuration array */
struct dlb_config {
	u32 reg_addr;
	u32 reg_data;
};

/* Topology update structure */
struct topology_update_info {
	int	update_ecc;
	u8	ecc;
	int	update_width;
	u8	width;
	int	update_ecc_pup3_mode;
	u8	ecc_pup_mode_offset;
};

/* Topology update defines */
#define TOPOLOGY_UPDATE_WIDTH_16BIT		1
#define TOPOLOGY_UPDATE_WIDTH_32BIT		0
#define TOPOLOGY_UPDATE_WIDTH_32BIT_MASK	0xf
#define TOPOLOGY_UPDATE_WIDTH_16BIT_MASK	0x3

#define TOPOLOGY_UPDATE_ECC_ON			1
#define TOPOLOGY_UPDATE_ECC_OFF			0
#define TOPOLOGY_UPDATE_ECC_OFFSET_PUP4		4
#define TOPOLOGY_UPDATE_ECC_OFFSET_PUP3		3

/*
 * 1. L2 filter should be set at binary header to 0xd000000,
 *    to avoid conflict with internal register IO.
 * 2. U-Boot modifies internal registers base to 0xf100000,
 *    and than should update L2 filter accordingly to 0xf000000 (3.75 GB)
 */
/* temporary limit l2 filter to 3GiB (LSP issue) */
#define L2_FILTER_FOR_MAX_MEMORY_SIZE 0xc0000000
#define ADDRESS_FILTERING_END_REGISTER 0x8c04

#define SUB_VERSION	0

#endif /* _DDR3_HWS_HW_TRAINING_H */

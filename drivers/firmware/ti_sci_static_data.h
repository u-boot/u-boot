/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C)  2021 Texas Instruments Incorporated - http://www.ti.com/
 *
 */

#ifndef __TI_SCI_STATIC_DATA_H
#define __TI_SCI_STATIC_DATA_H

struct ti_sci_resource_static_data {
	u32 dev_id;
	u16 range_start;
	u16 range_num;
	u8 subtype;
};

#if IS_ENABLED(CONFIG_K3_DM_FW)

#if IS_ENABLED(CONFIG_TARGET_J721E_R5_EVM)
static struct ti_sci_resource_static_data rm_static_data[] = {
	/* Free rings */
	{
		.dev_id = 235,
		.subtype = 1,
		.range_start = 124,
		.range_num = 32,
	},
	/* TX channels */
	{
		.dev_id = 236,
		.subtype = 13,
		.range_start = 6,
		.range_num = 2,
	},
	/* RX channels */
	{
		.dev_id = 236,
		.subtype = 10,
		.range_start = 6,
		.range_num = 2,
	},
	/* RX Free flows */
	{
		.dev_id = 236,
		.subtype = 0,
		.range_start = 60,
		.range_num = 8,
	},
	{ },
};
#endif /* CONFIG_TARGET_J721E_R5_EVM */

#if IS_ENABLED(CONFIG_TARGET_J7200_R5_EVM)
static struct ti_sci_resource_static_data rm_static_data[] = {
	/* Free rings */
	{
		.dev_id = 235,
		.subtype = 1,
		.range_start = 124,
		.range_num = 32,
	},
	/* TX channels */
	{
		.dev_id = 236,
		.subtype = 13,
		.range_start = 6,
		.range_num = 2,
	},
	/* RX channels */
	{
		.dev_id = 236,
		.subtype = 10,
		.range_start = 6,
		.range_num = 2,
	},
	/* RX Free flows */
	{
		.dev_id = 236,
		.subtype = 0,
		.range_start = 60,
		.range_num = 8,
	},
	{ },
};
#endif /* CONFIG_TARGET_J7200_R5_EVM */

#if IS_ENABLED(CONFIG_TARGET_J721S2_R5_EVM)
static struct ti_sci_resource_static_data rm_static_data[] = {
	/* Free rings */
	{
		.dev_id = 272,
		.subtype = 1,
		.range_start = 180,
		.range_num = 32,
	},
	/* TX channels */
	{
		.dev_id = 273,
		.subtype = 13,
		.range_start = 12,
		.range_num = 2,
	},
	/* RX channels */
	{
		.dev_id = 273,
		.subtype = 10,
		.range_start = 12,
		.range_num = 2,
	},
	/* RX Free flows */
	{
		.dev_id = 273,
		.subtype = 0,
		.range_start = 80,
		.range_num = 8,
	},
	{ },
};
#endif /* CONFIG_TARGET_J721S2_R5_EVM */

#else
static struct ti_sci_resource_static_data rm_static_data[] = {
	{ },
};
#endif /* CONFIG_K3_DM_FW */
#endif /* __TI_SCI_STATIC_DATA_H */

/* Copyright 2013-2015 Freescale Semiconductor Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __FSL_DPMNG_CMD_H
#define __FSL_DPMNG_CMD_H

/* Command IDs */
#define DPMNG_CMDID_GET_VERSION			0x831

/*                cmd, param, offset, width, type, arg_name */
#define DPMNG_RSP_GET_VERSION(cmd, mc_ver_info) \
do { \
	MC_RSP_OP(cmd, 0, 0,  32, uint32_t, mc_ver_info->revision); \
	MC_RSP_OP(cmd, 0, 32, 32, uint32_t, mc_ver_info->major); \
	MC_RSP_OP(cmd, 1, 0,  32, uint32_t, mc_ver_info->minor); \
} while (0)

#endif /* __FSL_DPMNG_CMD_H */

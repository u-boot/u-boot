/* Copyright 2014 Freescale Semiconductor Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __FSL_DPMNG_CMD_H
#define __FSL_DPMNG_CMD_H

/* Command IDs */
#define DPMNG_CMDID_GET_VERSION			0x831
#define DPMNG_CMDID_RESET_AIOP			0x832
#define DPMNG_CMDID_LOAD_AIOP			0x833
#define DPMNG_CMDID_RUN_AIOP			0x834
#define DPMNG_CMDID_RESET_MC_PORTAL		0x835

/*                cmd, param, offset, width, type, arg_name */
#define DPMNG_RSP_GET_VERSION(cmd, mc_ver_info) \
do { \
	MC_RSP_OP(cmd, 0, 0,  32, uint32_t, mc_ver_info->revision); \
	MC_RSP_OP(cmd, 0, 32, 32, uint32_t, mc_ver_info->major); \
	MC_RSP_OP(cmd, 1, 0,  32, uint32_t, mc_ver_info->minor); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPMNG_CMD_RESET_AIOP(cmd, container_id, aiop_tile_id) \
do { \
	MC_CMD_OP(cmd, 0, 0,  32, int,      aiop_tile_id); \
	MC_CMD_OP(cmd, 0, 32, 32, int,      container_id); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPMNG_CMD_LOAD_AIOP(cmd, container_id, aiop_tile_id, img_size, \
			    img_iova) \
do { \
	MC_CMD_OP(cmd, 0, 0,  32, int,      aiop_tile_id); \
	MC_CMD_OP(cmd, 0, 32, 32, int,      container_id); \
	MC_CMD_OP(cmd, 1, 0,  32, uint32_t, img_size); \
	MC_CMD_OP(cmd, 2, 0,  64, uint64_t, img_iova); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPMNG_CMD_RUN_AIOP(cmd, container_id, aiop_tile_id, cfg) \
do { \
	MC_CMD_OP(cmd, 0, 0,  32, int,	    aiop_tile_id); \
	MC_CMD_OP(cmd, 0, 32, 32, int,      container_id); \
	MC_CMD_OP(cmd, 1, 0,  32, uint32_t, cfg->cores_mask); \
	MC_CMD_OP(cmd, 2, 0,  64, uint64_t, cfg->options); \
} while (0)

#endif /* __FSL_DPMNG_CMD_H */

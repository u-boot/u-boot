/* Copyright 2014 Freescale Semiconductor Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_cmd.h>
#include <fsl-mc/fsl_dpmng.h>
#include "fsl_dpmng_cmd.h"

int mc_get_version(struct fsl_mc_io *mc_io, struct mc_version *mc_ver_info)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMNG_CMDID_GET_VERSION,
					  MC_CMD_PRI_LOW, 0);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPMNG_RSP_GET_VERSION(cmd, mc_ver_info);

	return 0;
}

int dpmng_reset_aiop(struct fsl_mc_io *mc_io, int container_id,
		     int aiop_tile_id)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMNG_CMDID_RESET_AIOP,
					  MC_CMD_PRI_LOW, 0);
	DPMNG_CMD_RESET_AIOP(cmd, container_id, aiop_tile_id);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpmng_load_aiop(struct fsl_mc_io *mc_io,
		    int container_id,
		    int aiop_tile_id,
		    uint64_t img_iova,
		    uint32_t img_size)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMNG_CMDID_LOAD_AIOP,
					  MC_CMD_PRI_LOW,
					  0);
	DPMNG_CMD_LOAD_AIOP(cmd, container_id, aiop_tile_id, img_size,
			    img_iova);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpmng_run_aiop(struct fsl_mc_io *mc_io,
		   int container_id,
		   int aiop_tile_id,
		   const struct dpmng_aiop_run_cfg *cfg)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMNG_CMDID_RUN_AIOP,
					  MC_CMD_PRI_LOW,
					  0);
	DPMNG_CMD_RUN_AIOP(cmd, container_id, aiop_tile_id, cfg);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpmng_reset_mc_portal(struct fsl_mc_io *mc_io)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMNG_CMDID_RESET_MC_PORTAL,
					  MC_CMD_PRI_LOW,
					  0);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

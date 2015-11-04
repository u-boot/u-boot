/*
 * Freescale Layerscape MC I/O wrapper
 *
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 * Author: Prabhakar Kushwaha <prabhakar@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_cmd.h>
#include <fsl-mc/fsl_dpmac.h>

int dpmac_open(struct fsl_mc_io *mc_io,
	       uint32_t cmd_flags,
	       int dpmac_id,
	       uint16_t *token)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_OPEN,
					  cmd_flags,
					  0);
	DPMAC_CMD_OPEN(cmd, dpmac_id);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*token = MC_CMD_HDR_READ_TOKEN(cmd.header);

	return err;
}

int dpmac_close(struct fsl_mc_io *mc_io,
		uint32_t cmd_flags,
		uint16_t token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_CLOSE, cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpmac_create(struct fsl_mc_io *mc_io,
		 uint32_t cmd_flags,
		 const struct dpmac_cfg *cfg,
		 uint16_t *token)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_CREATE,
					  cmd_flags,
					  0);
	DPMAC_CMD_CREATE(cmd, cfg);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*token = MC_CMD_HDR_READ_TOKEN(cmd.header);

	return 0;
}

int dpmac_destroy(struct fsl_mc_io *mc_io,
		  uint32_t cmd_flags,
		  uint16_t token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_DESTROY,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpmac_get_attributes(struct fsl_mc_io *mc_io,
			 uint32_t cmd_flags,
			 uint16_t token,
			 struct dpmac_attr *attr)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_GET_ATTR,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPMAC_RSP_GET_ATTRIBUTES(cmd, attr);

	return 0;
}

int dpmac_mdio_read(struct fsl_mc_io *mc_io,
		    uint32_t cmd_flags,
		    uint16_t token,
		    struct dpmac_mdio_cfg *cfg)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_MDIO_READ,
					  cmd_flags,
					  token);
	DPMAC_CMD_MDIO_READ(cmd, cfg);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPMAC_RSP_MDIO_READ(cmd, cfg->data);

	return 0;
}

int dpmac_mdio_write(struct fsl_mc_io *mc_io,
		     uint32_t cmd_flags,
		     uint16_t token,
		     struct dpmac_mdio_cfg *cfg)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_MDIO_WRITE,
					  cmd_flags,
					  token);
	DPMAC_CMD_MDIO_WRITE(cmd, cfg);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpmac_get_link_cfg(struct fsl_mc_io *mc_io,
		       uint32_t cmd_flags,
		       uint16_t token,
		       struct dpmac_link_cfg *cfg)
{
	struct mc_command cmd = { 0 };
	int err = 0;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_GET_LINK_CFG,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	DPMAC_RSP_GET_LINK_CFG(cmd, cfg);

	return 0;
}

int dpmac_set_link_state(struct fsl_mc_io *mc_io,
			 uint32_t cmd_flags,
			 uint16_t token,
			 struct dpmac_link_state *link_state)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_SET_LINK_STATE,
					  cmd_flags,
					  token);
	DPMAC_CMD_SET_LINK_STATE(cmd, link_state);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpmac_get_counter(struct fsl_mc_io *mc_io,
		      uint32_t cmd_flags,
		      uint16_t token,
		      enum dpmac_counter type,
		      uint64_t *counter)
{
	struct mc_command cmd = { 0 };
	int err = 0;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_GET_COUNTER,
					  cmd_flags,
					  token);
	DPMAC_CMD_GET_COUNTER(cmd, type);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	DPMAC_RSP_GET_COUNTER(cmd, *counter);

	return 0;
}

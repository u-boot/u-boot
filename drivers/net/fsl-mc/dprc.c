/*
 * Freescale Layerscape MC I/O wrapper
 *
 * Copyright (C) 2013-2015 Freescale Semiconductor, Inc.
 * Author: German Rivera <German.Rivera@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_cmd.h>
#include <fsl-mc/fsl_dprc.h>

int dprc_get_container_id(struct fsl_mc_io *mc_io, int *container_id)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_GET_CONT_ID,
					  MC_CMD_PRI_LOW, 0);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPRC_RSP_GET_CONTAINER_ID(cmd, *container_id);

	return 0;
}

int dprc_open(struct fsl_mc_io *mc_io, int container_id, uint16_t *token)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_OPEN, MC_CMD_PRI_LOW,
					  0);
	DPRC_CMD_OPEN(cmd, container_id);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*token = MC_CMD_HDR_READ_TOKEN(cmd.header);

	return 0;
}

int dprc_close(struct fsl_mc_io *mc_io, uint16_t token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_CLOSE, MC_CMD_PRI_HIGH,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dprc_reset_container(struct fsl_mc_io *mc_io,
			 uint16_t token,
			 int child_container_id)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_RESET_CONT,
					  MC_CMD_PRI_LOW, token);
	DPRC_CMD_RESET_CONTAINER(cmd, child_container_id);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dprc_get_attributes(struct fsl_mc_io *mc_io,
			uint16_t token,
			struct dprc_attributes *attr)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_GET_ATTR,
					  MC_CMD_PRI_LOW,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPRC_RSP_GET_ATTRIBUTES(cmd, attr);

	return 0;
}

int dprc_get_obj_count(struct fsl_mc_io *mc_io, uint16_t token, int *obj_count)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_GET_OBJ_COUNT,
					  MC_CMD_PRI_LOW, token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPRC_RSP_GET_OBJ_COUNT(cmd, *obj_count);

	return 0;
}

int dprc_get_obj(struct fsl_mc_io *mc_io,
		 uint16_t token,
		 int obj_index,
		 struct dprc_obj_desc *obj_desc)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_GET_OBJ,
					  MC_CMD_PRI_LOW,
					  token);
	DPRC_CMD_GET_OBJ(cmd, obj_index);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPRC_RSP_GET_OBJ(cmd, obj_desc);

	return 0;
}

int dprc_get_res_count(struct fsl_mc_io *mc_io,
		       uint16_t token,
		       char *type,
		       int *res_count)
{
	struct mc_command cmd = { 0 };
	int err;

	*res_count = 0;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_GET_RES_COUNT,
					  MC_CMD_PRI_LOW, token);
	DPRC_CMD_GET_RES_COUNT(cmd, type);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPRC_RSP_GET_RES_COUNT(cmd, *res_count);

	return 0;
}

int dprc_get_res_ids(struct fsl_mc_io *mc_io,
		     uint16_t token,
		     char *type,
		     struct dprc_res_ids_range_desc *range_desc)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_GET_RES_IDS,
					  MC_CMD_PRI_LOW, token);
	DPRC_CMD_GET_RES_IDS(cmd, range_desc, type);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPRC_RSP_GET_RES_IDS(cmd, range_desc);

	return 0;
}

int dprc_get_obj_region(struct fsl_mc_io *mc_io,
			uint16_t token,
			char *obj_type,
			int obj_id,
			uint8_t region_index,
			struct dprc_region_desc *region_desc)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_GET_OBJ_REG,
					  MC_CMD_PRI_LOW, token);
	DPRC_CMD_GET_OBJ_REGION(cmd, obj_type, obj_id, region_index);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPRC_RSP_GET_OBJ_REGION(cmd, region_desc);

	return 0;
}

int dprc_connect(struct fsl_mc_io *mc_io,
		 uint16_t token,
		 const struct dprc_endpoint *endpoint1,
		 const struct dprc_endpoint *endpoint2)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_CONNECT,
					  MC_CMD_PRI_LOW,
					  token);
	DPRC_CMD_CONNECT(cmd, endpoint1, endpoint2);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dprc_disconnect(struct fsl_mc_io *mc_io,
		    uint16_t token,
		    const struct dprc_endpoint *endpoint)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_DISCONNECT,
					  MC_CMD_PRI_LOW,
					  token);
	DPRC_CMD_DISCONNECT(cmd, endpoint);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dprc_get_connection(struct fsl_mc_io *mc_io,
			uint16_t token,
					const struct dprc_endpoint *endpoint1,
					struct dprc_endpoint *endpoint2,
					int *state)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_GET_CONNECTION,
					  MC_CMD_PRI_LOW,
					  token);
	DPRC_CMD_GET_CONNECTION(cmd, endpoint1);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPRC_RSP_GET_CONNECTION(cmd, endpoint2, *state);

	return 0;
}

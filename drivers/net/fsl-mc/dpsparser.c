// SPDX-License-Identifier: GPL-2.0+
/*
 * Data Path Soft Parser
 *
 * Copyright 2018 NXP
 */
#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_cmd.h>
#include <fsl-mc/fsl_dpsparser.h>

int dpsparser_open(struct fsl_mc_io *mc_io,
		   u32 cmd_flags,
		   u16 *token)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPSPARSER_CMDID_OPEN,
					  cmd_flags,
					  0);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*token = MC_CMD_HDR_READ_TOKEN(cmd.header);

	return err;
}

int dpsparser_close(struct fsl_mc_io *mc_io,
		    u32 cmd_flags,
		    u16 token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPSPARSER_CMDID_CLOSE, cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpsparser_create(struct fsl_mc_io *mc_io,
		     u16 token,
		     u32 cmd_flags,
		     u32 *obj_id)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPSPARSER_CMDID_CREATE,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	MC_CMD_READ_OBJ_ID(cmd, *obj_id);

	return 0;
}

int dpsparser_destroy(struct fsl_mc_io *mc_io,
		      u16 token,
		      u32 cmd_flags,
		      u32 obj_id)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPSPARSER_CMDID_DESTROY,
					  cmd_flags,
					  token);

	/* set object id to destroy */
	CMD_DESTROY_SET_OBJ_ID_PARAM0(cmd, obj_id);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpsparser_apply_spb(struct fsl_mc_io *mc_io,
			u32 cmd_flags,
			u16 token,
			u64 blob_addr,
			u16 *error)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPSPARSER_CMDID_APPLY_SPB,
					  cmd_flags,
					  token);
	DPSPARSER_CMD_BLOB_SET_ADDR(cmd, blob_addr);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters: MC error code */
	DPSPARSER_CMD_BLOB_REPORT_ERROR(cmd, *error);

	return 0;
}

int dpsparser_get_api_version(struct fsl_mc_io *mc_io,
			      u32 cmd_flags,
			      u16 *major_ver,
			      u16 *minor_ver)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPSPARSER_CMDID_GET_API_VERSION,
					  cmd_flags, 0);

	/* send command to mc */
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	mc_cmd_read_api_version(&cmd, major_ver, minor_ver);

	return 0;
}

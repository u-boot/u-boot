/*
 * Copyright (C) 2013-2015 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_cmd.h>
#include <fsl-mc/fsl_dpni.h>

int dpni_open(struct fsl_mc_io *mc_io,
	      uint32_t cmd_flags,
	      int dpni_id,
	      uint16_t *token)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_OPEN,
					  cmd_flags,
					  0);
	DPNI_CMD_OPEN(cmd, dpni_id);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*token = MC_CMD_HDR_READ_TOKEN(cmd.header);

	return 0;
}

int dpni_close(struct fsl_mc_io *mc_io,
	       uint32_t cmd_flags,
	       uint16_t token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_CLOSE,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpni_create(struct fsl_mc_io *mc_io,
		uint32_t cmd_flags,
		const struct dpni_cfg *cfg,
		uint16_t *token)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_CREATE,
					  cmd_flags,
					  0);
	DPNI_CMD_CREATE(cmd, cfg);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*token = MC_CMD_HDR_READ_TOKEN(cmd.header);

	return 0;
}

int dpni_destroy(struct fsl_mc_io *mc_io,
		 uint32_t cmd_flags,
		 uint16_t token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_DESTROY,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpni_set_pools(struct fsl_mc_io *mc_io,
		   uint32_t cmd_flags,
		   uint16_t token,
		   const struct dpni_pools_cfg *cfg)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_SET_POOLS,
					  cmd_flags,
					  token);
	DPNI_CMD_SET_POOLS(cmd, cfg);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpni_enable(struct fsl_mc_io *mc_io,
		uint32_t cmd_flags,
		uint16_t token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_ENABLE,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpni_disable(struct fsl_mc_io *mc_io,
		 uint32_t cmd_flags,
		 uint16_t token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_DISABLE,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpni_reset(struct fsl_mc_io *mc_io,
	       uint32_t cmd_flags,
	       uint16_t token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_RESET,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpni_get_attributes(struct fsl_mc_io *mc_io,
			uint32_t cmd_flags,
			uint16_t token,
			struct dpni_attr *attr)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_ATTR,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPNI_RSP_GET_ATTR(cmd, attr);

	return 0;
}

int dpni_get_rx_buffer_layout(struct fsl_mc_io *mc_io,
			      uint32_t cmd_flags,
			      uint16_t token,
			      struct dpni_buffer_layout *layout)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_RX_BUFFER_LAYOUT,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPNI_RSP_GET_RX_BUFFER_LAYOUT(cmd, layout);

	return 0;
}

int dpni_set_rx_buffer_layout(struct fsl_mc_io *mc_io,
			      uint32_t cmd_flags,
			      uint16_t token,
			      const struct dpni_buffer_layout *layout)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_SET_RX_BUFFER_LAYOUT,
					  cmd_flags,
					  token);
	DPNI_CMD_SET_RX_BUFFER_LAYOUT(cmd, layout);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpni_get_tx_buffer_layout(struct fsl_mc_io *mc_io,
			      uint32_t cmd_flags,
			      uint16_t token,
			      struct dpni_buffer_layout *layout)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_TX_BUFFER_LAYOUT,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPNI_RSP_GET_TX_BUFFER_LAYOUT(cmd, layout);

	return 0;
}

int dpni_set_tx_buffer_layout(struct fsl_mc_io *mc_io,
			      uint32_t cmd_flags,
			      uint16_t token,
			      const struct dpni_buffer_layout *layout)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_SET_TX_BUFFER_LAYOUT,
					  cmd_flags,
					  token);
	DPNI_CMD_SET_TX_BUFFER_LAYOUT(cmd, layout);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpni_get_tx_conf_buffer_layout(struct fsl_mc_io *mc_io,
				   uint32_t cmd_flags,
				   uint16_t token,
				   struct dpni_buffer_layout *layout)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_TX_CONF_BUFFER_LAYOUT,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPNI_RSP_GET_TX_CONF_BUFFER_LAYOUT(cmd, layout);

	return 0;
}

int dpni_set_tx_conf_buffer_layout(struct fsl_mc_io *mc_io,
				   uint32_t cmd_flags,
				   uint16_t token,
				   const struct dpni_buffer_layout *layout)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_SET_TX_CONF_BUFFER_LAYOUT,
					  cmd_flags,
					  token);
	DPNI_CMD_SET_TX_CONF_BUFFER_LAYOUT(cmd, layout);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpni_get_qdid(struct fsl_mc_io *mc_io,
		  uint32_t cmd_flags,
		  uint16_t token,
		  uint16_t *qdid)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_QDID,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPNI_RSP_GET_QDID(cmd, *qdid);

	return 0;
}

int dpni_get_tx_data_offset(struct fsl_mc_io *mc_io,
			    uint32_t cmd_flags,
			    uint16_t token,
			    uint16_t *data_offset)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_TX_DATA_OFFSET,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPNI_RSP_GET_TX_DATA_OFFSET(cmd, *data_offset);

	return 0;
}

int dpni_get_counter(struct fsl_mc_io *mc_io,
		     uint32_t cmd_flags,
		     uint16_t token,
		     enum dpni_counter counter,
		     uint64_t *value)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_COUNTER,
					  cmd_flags,
					  token);
	DPNI_CMD_GET_COUNTER(cmd, counter);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPNI_RSP_GET_COUNTER(cmd, *value);

	return 0;
}

int dpni_set_counter(struct fsl_mc_io *mc_io,
		     uint32_t cmd_flags,
		     uint16_t token,
		     enum dpni_counter counter,
		     uint64_t value)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_SET_COUNTER,
					  cmd_flags,
					  token);
	DPNI_CMD_SET_COUNTER(cmd, counter, value);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpni_set_link_cfg(struct fsl_mc_io *mc_io,
		      uint32_t cmd_flags,
		      uint16_t token,
		      const struct dpni_link_cfg *cfg)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_SET_LINK_CFG,
					  cmd_flags,
					  token);
	DPNI_CMD_SET_LINK_CFG(cmd, cfg);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpni_get_link_state(struct fsl_mc_io *mc_io,
			uint32_t cmd_flags,
			uint16_t token,
			struct dpni_link_state *state)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_LINK_STATE,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPNI_RSP_GET_LINK_STATE(cmd, state);

	return 0;
}


int dpni_set_primary_mac_addr(struct fsl_mc_io *mc_io,
			      uint32_t cmd_flags,
			      uint16_t token,
			      const uint8_t mac_addr[6])
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_SET_PRIM_MAC,
					  cmd_flags,
					  token);
	DPNI_CMD_SET_PRIMARY_MAC_ADDR(cmd, mac_addr);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpni_get_primary_mac_addr(struct fsl_mc_io *mc_io,
			      uint32_t cmd_flags,
			      uint16_t token,
			      uint8_t mac_addr[6])
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_PRIM_MAC,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPNI_RSP_GET_PRIMARY_MAC_ADDR(cmd, mac_addr);

	return 0;
}

int dpni_add_mac_addr(struct fsl_mc_io *mc_io,
		      uint32_t cmd_flags,
		      uint16_t token,
		      const uint8_t mac_addr[6])
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_ADD_MAC_ADDR,
					  cmd_flags,
					  token);
	DPNI_CMD_ADD_MAC_ADDR(cmd, mac_addr);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpni_remove_mac_addr(struct fsl_mc_io *mc_io,
			 uint32_t cmd_flags,
			 uint16_t token,
			 const uint8_t mac_addr[6])
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_REMOVE_MAC_ADDR,
					  cmd_flags,
					  token);
	DPNI_CMD_REMOVE_MAC_ADDR(cmd, mac_addr);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpni_set_tx_flow(struct fsl_mc_io *mc_io,
		     uint32_t cmd_flags,
		     uint16_t token,
		     uint16_t *flow_id,
		     const struct dpni_tx_flow_cfg *cfg)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_SET_TX_FLOW,
					  cmd_flags,
					  token);
	DPNI_CMD_SET_TX_FLOW(cmd, *flow_id, cfg);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPNI_RSP_SET_TX_FLOW(cmd, *flow_id);

	return 0;
}

int dpni_get_tx_flow(struct fsl_mc_io *mc_io,
		     uint32_t cmd_flags,
		     uint16_t token,
		     uint16_t flow_id,
		     struct dpni_tx_flow_attr *attr)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_TX_FLOW,
					  cmd_flags,
					  token);
	DPNI_CMD_GET_TX_FLOW(cmd, flow_id);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPNI_RSP_GET_TX_FLOW(cmd, attr);

	return 0;
}

int dpni_set_rx_flow(struct fsl_mc_io *mc_io,
		     uint32_t cmd_flags,
		     uint16_t token,
		     uint8_t tc_id,
		     uint16_t flow_id,
		     const struct dpni_queue_cfg *cfg)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_SET_RX_FLOW,
					  cmd_flags,
					  token);
	DPNI_CMD_SET_RX_FLOW(cmd, tc_id, flow_id, cfg);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpni_get_rx_flow(struct fsl_mc_io *mc_io,
		     uint32_t cmd_flags,
		     uint16_t token,
		     uint8_t tc_id,
		     uint16_t flow_id,
		     struct dpni_queue_attr *attr)
{
	struct mc_command cmd = { 0 };
	int err;
	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_RX_FLOW,
					  cmd_flags,
					  token);
	DPNI_CMD_GET_RX_FLOW(cmd, tc_id, flow_id);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPNI_RSP_GET_RX_FLOW(cmd, attr);

	return 0;
}

/* Copyright 2013-2015 Freescale Semiconductor Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __FSL_MC_CMD_H
#define __FSL_MC_CMD_H

#define MC_CMD_NUM_OF_PARAMS	7

#define MAKE_UMASK64(_width) \
	((uint64_t)((_width) < 64 ? ((uint64_t)1 << (_width)) - 1 : -1))

static inline uint64_t u64_enc(int lsoffset, int width, uint64_t val)
{
	return (uint64_t)(((uint64_t)val & MAKE_UMASK64(width)) << lsoffset);
}
static inline uint64_t u64_dec(uint64_t val, int lsoffset, int width)
{
	return (uint64_t)((val >> lsoffset) & MAKE_UMASK64(width));
}

struct mc_command {
	uint64_t header;
	uint64_t params[MC_CMD_NUM_OF_PARAMS];
};

enum mc_cmd_status {
	MC_CMD_STATUS_OK = 0x0, /*!< Completed successfully */
	MC_CMD_STATUS_READY = 0x1, /*!< Ready to be processed */
	MC_CMD_STATUS_AUTH_ERR = 0x3, /*!< Authentication error */
	MC_CMD_STATUS_NO_PRIVILEGE = 0x4, /*!< No privilege */
	MC_CMD_STATUS_DMA_ERR = 0x5, /*!< DMA or I/O error */
	MC_CMD_STATUS_CONFIG_ERR = 0x6, /*!< Configuration error */
	MC_CMD_STATUS_TIMEOUT = 0x7, /*!< Operation timed out */
	MC_CMD_STATUS_NO_RESOURCE = 0x8, /*!< No resources */
	MC_CMD_STATUS_NO_MEMORY = 0x9, /*!< No memory available */
	MC_CMD_STATUS_BUSY = 0xA, /*!< Device is busy */
	MC_CMD_STATUS_UNSUPPORTED_OP = 0xB, /*!< Unsupported operation */
	MC_CMD_STATUS_INVALID_STATE = 0xC /*!< Invalid state */
};

#define MC_CMD_HDR_CMDID_O	52	/* Command ID field offset */
#define MC_CMD_HDR_CMDID_S	12	/* Command ID field size */
#define MC_CMD_HDR_STATUS_O	16	/* Status field offset */
#define MC_CMD_HDR_TOKEN_O	38	/* Token field offset */
#define MC_CMD_HDR_TOKEN_S	10	/* Token field size */
#define MC_CMD_HDR_STATUS_S	8	/* Status field size*/
#define MC_CMD_HDR_PRI_O	15	/* Priority field offset */
#define MC_CMD_HDR_PRI_S	1	/* Priority field size */

#define MC_CMD_HDR_READ_STATUS(_hdr) \
	((enum mc_cmd_status)u64_dec((_hdr), \
		MC_CMD_HDR_STATUS_O, MC_CMD_HDR_STATUS_S))

#define MC_CMD_HDR_READ_TOKEN(_hdr) \
	((uint16_t)u64_dec((_hdr), MC_CMD_HDR_TOKEN_O, MC_CMD_HDR_TOKEN_S))

#define MC_CMD_PRI_LOW		0 /*!< Low Priority command indication */
#define MC_CMD_PRI_HIGH		1 /*!< High Priority command indication */

#define MC_EXT_OP(_ext, _param, _offset, _width, _type, _arg) \
	((_ext)[_param] |= u64_enc((_offset), (_width), _arg))

#define MC_CMD_OP(_cmd, _param, _offset, _width, _type, _arg) \
	((_cmd).params[_param] |= u64_enc((_offset), (_width), _arg))

#define MC_RSP_OP(_cmd, _param, _offset, _width, _type, _arg) \
	(_arg = (_type)u64_dec(_cmd.params[_param], (_offset), (_width)))

static inline uint64_t mc_encode_cmd_header(uint16_t cmd_id,
					    uint8_t priority,
					    uint16_t token)
{
	uint64_t hdr;

	hdr = u64_enc(MC_CMD_HDR_CMDID_O, MC_CMD_HDR_CMDID_S, cmd_id);
	hdr |= u64_enc(MC_CMD_HDR_TOKEN_O, MC_CMD_HDR_TOKEN_S, token);
	hdr |= u64_enc(MC_CMD_HDR_PRI_O, MC_CMD_HDR_PRI_S, priority);
	hdr |= u64_enc(MC_CMD_HDR_STATUS_O, MC_CMD_HDR_STATUS_S,
		       MC_CMD_STATUS_READY);

	return hdr;
}

/**
 * mc_write_command - writes a command to a Management Complex (MC) portal
 *
 * @portal: pointer to an MC portal
 * @cmd: pointer to a filled command
 */
static inline void mc_write_command(struct mc_command __iomem *portal,
				    struct mc_command *cmd)
{
	int i;

	/* copy command parameters into the portal */
	for (i = 0; i < MC_CMD_NUM_OF_PARAMS; i++)
		writeq(cmd->params[i], &portal->params[i]);

	/* submit the command by writing the header */
	writeq(cmd->header, &portal->header);
}

/**
 * mc_read_response - reads the response for the last MC command from a
 * Management Complex (MC) portal
 *
 * @portal: pointer to an MC portal
 * @resp: pointer to command response buffer
 *
 * Returns MC_CMD_STATUS_OK on Success; Error code otherwise.
 */
static inline enum mc_cmd_status mc_read_response(
					struct mc_command __iomem *portal,
					struct mc_command *resp)
{
	int i;
	enum mc_cmd_status status;

	/* Copy command response header from MC portal: */
	resp->header = readq(&portal->header);
	status = MC_CMD_HDR_READ_STATUS(resp->header);
	if (status != MC_CMD_STATUS_OK)
		return status;

	/* Copy command response data from MC portal: */
	for (i = 0; i < MC_CMD_NUM_OF_PARAMS; i++)
		resp->params[i] = readq(&portal->params[i]);

	return status;
}

int mc_send_command(struct fsl_mc_io *mc_io, struct mc_command *cmd);

#endif /* __FSL_MC_CMD_H */

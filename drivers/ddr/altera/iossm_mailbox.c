// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 *
 */

#include <hang.h>
#include <string.h>
#include <wait_bit.h>
#include <asm/arch/base_addr_soc64.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/sizes.h>
#include "iossm_mailbox.h"

#define TIMEOUT_120000MS			120000
#define TIMEOUT_60000MS				60000
#define TIMEOUT					TIMEOUT_120000MS
#define IOSSM_STATUS_CAL_SUCCESS		BIT(0)
#define IOSSM_STATUS_CAL_FAIL			BIT(1)
#define IOSSM_STATUS_CAL_BUSY			BIT(2)
#define IOSSM_STATUS_COMMAND_RESPONSE_READY	BIT(0)
#define IOSSM_CMD_RESPONSE_STATUS_OFFSET	0x45C
#define IOSSM_CMD_RESPONSE_DATA_0_OFFSET	0x458
#define IOSSM_CMD_RESPONSE_DATA_1_OFFSET	0x454
#define IOSSM_CMD_RESPONSE_DATA_2_OFFSET	0x450
#define IOSSM_CMD_REQ_OFFSET			0x43C
#define IOSSM_CMD_PARAM_0_OFFSET		0x438
#define IOSSM_CMD_PARAM_1_OFFSET		0x434
#define IOSSM_CMD_PARAM_2_OFFSET		0x430
#define IOSSM_CMD_PARAM_3_OFFSET		0x42C
#define IOSSM_CMD_PARAM_4_OFFSET		0x428
#define IOSSM_CMD_PARAM_5_OFFSET		0x424
#define IOSSM_CMD_PARAM_6_OFFSET		0x420
#define IOSSM_CMD_RESPONSE_DATA_SHORT_MASK	GENMASK(31, 16)
#define IOSSM_CMD_RESPONSE_DATA_SHORT(n)	FIELD_GET(IOSSM_CMD_RESPONSE_DATA_SHORT_MASK, n)
#define IOSSM_STATUS_CMD_RESPONSE_ERROR_MASK	GENMASK(7, 5)
#define IOSSM_STATUS_CMD_RESPONSE_ERROR(n)	FIELD_GET(IOSSM_STATUS_CMD_RESPONSE_ERROR_MASK, n)
#define IOSSM_STATUS_GENERAL_ERROR_MASK		GENMASK(4, 1)
#define IOSSM_STATUS_GENERAL_ERROR(n)		FIELD_GET(IOSSM_STATUS_GENERAL_ERROR_MASK, n)

/* Offset of Mailbox Read-only Registers  */
#define IOSSM_MAILBOX_HEADER_OFFSET			0x0
#define IOSSM_MEM_INTF_INFO_0_OFFSET			0x200
#define IOSSM_MEM_INTF_INFO_1_OFFSET			0x280
#define IOSSM_MEM_TECHNOLOGY_INTF0_OFFSET		0x210
#define IOSSM_MEM_TECHNOLOGY_INTF1_OFFSET		0x290
#define IOSSM_MEM_WIDTH_INFO_INTF0_OFFSET		0x230
#define IOSSM_MEM_WIDTH_INFO_INTF1_OFFSET		0x2B0
#define IOSSM_MEM_TOTAL_CAPACITY_INTF0_OFFSET		0x234
#define IOSSM_MEM_TOTAL_CAPACITY_INTF1_OFFSET		0x2B4
#define IOSSM_ECC_ENABLE_INTF0_OFFSET			0x240
#define IOSSM_ECC_ENABLE_INTF1_OFFSET			0x2C0
#define IOSSM_ECC_SCRUB_STATUS_INTF0_OFFSET		0x244
#define IOSSM_ECC_SCRUB_STATUS_INTF1_OFFSET		0x2C4
#define IOSSM_LP_MODE_INTF0_OFFSET			0x250
#define IOSSM_LP_MODE_INTF1_OFFSET			0x2D0
#define IOSSM_MEM_INIT_STATUS_INTF0_OFFSET		0x260
#define IOSSM_MEM_INIT_STATUS_INTF1_OFFSET		0x2E0
#define IOSSM_BIST_STATUS_INTF0_OFFSET			0x264
#define IOSSM_BIST_STATUS_INTF1_OFFSET			0x2E4
#define IOSSM_ECC_ERR_STATUS_OFFSET			0x300
#define IOSSM_ECC_ERR_DATA_START_OFFSET			0x310
#define IOSSM_STATUS_OFFSET				0x400
#define IOSSM_STATUS_CAL_INTF0_OFFSET			0x404
#define IOSSM_STATUS_CAL_INTF1_OFFSET			0x408

#define ECC_INTSTATUS_SERR SOCFPGA_SYSMGR_ADDRESS + 0x9C
#define ECC_INISTATUS_DERR SOCFPGA_SYSMGR_ADDRESS + 0xA0
#define DDR_CSR_CLKGEN_LOCKED_IO96B0_MASK BIT(16)
#define DDR_CSR_CLKGEN_LOCKED_IO96B1_MASK BIT(17)

/* offset info of GET_MEM_INTF_INFO */
#define INTF_IP_TYPE_MASK		GENMASK(31, 29)
#define INTF_INSTANCE_ID_MASK		GENMASK(28, 24)

/* offset info of GET_MEM_CAL_STATUS */
#define INTF_UNUSED			0x0
#define INTF_MEM_CAL_STATUS_SUCCESS	0x1
#define INTF_MEM_CAL_STATUS_FAIL	0x2
#define INTF_MEM_CAL_STATUS_ONGOING	0x4

/* offset info of  MEM_TECHNOLOGY_INTF */
#define INTF_DDR_TYPE_MASK		GENMASK(2, 0)

/* offset info of MEM_TOTAL_CAPACITY_INTF */
#define INTF_CAPACITY_GBITS_MASK	GENMASK(7, 0)

/* offset info of ECC_ENABLE_INTF */
#define INTF_ECC_ENABLE_TYPE_MASK	GENMASK(1, 0)
#define INTF_ECC_TYPE_MASK	BIT(8)

/* cmd opcode BIST_MEM_INIT_START, BIST performed on full memory address range */
#define BIST_FULL_MEM			BIT(6)

/* offset info of ECC_ENABLE_INTF */
#define INTF_BIST_STATUS_MASK		BIT(0)

/* offset info of ECC_ERR_STATUS */
#define ECC_ERR_COUNTER_MASK		GENMASK(15, 0)
#define ECC_ERR_OVERFLOW_MASK		GENMASK(31, 16)

/* offset info of ECC_ERR_DATA */
#define ECC_ERR_IP_TYPE_MASK		GENMASK(24, 22)
#define ECC_ERR_INSTANCE_ID_MASK	GENMASK(21, 17)
#define ECC_ERR_SOURCE_ID_MASK		GENMASK(16, 10)
#define ECC_ERR_TYPE_MASK		GENMASK(9, 6)
#define ECC_ERR_ADDR_UPPER_MASK		GENMASK(5, 0)
#define ECC_ERR_ADDR_LOWER_MASK		GENMASK(31, 0)
#define ECC_FULL_ADDR_UPPER_MASK	GENMASK(63, 32)
#define ECC_FULL_ADDR_LOWER_MASK	GENMASK(31, 0)

#define MAX_ECC_ERR_INFO_COUNT 16

#define BIST_START_ADDR_SPACE_MASK GENMASK(5, 0)
#define BIST_START_ADDR_LOW_MASK   GENMASK(31, 0)
#define BIST_START_ADDR_HIGH_MASK  GENMASK(37, 32)

#define IO96B_MB_REQ_SETUP(v, w, x, y, z) \
	usr_req.ip_type = v; \
	usr_req.ip_id = w; \
	usr_req.usr_cmd_type = x; \
	usr_req.usr_cmd_opcode = y; \
	usr_req.cmd_param[0] = z; \
	for (n = 1; n < NUM_CMD_PARAM; n++) \
		usr_req.cmd_param[n] = 0
#define MAX_RETRY_COUNT 3
#define NUM_CMD_RESPONSE_DATA 3

#define IO96B0_PLL_A_MASK	BIT(0)
#define IO96B0_PLL_B_MASK	BIT(1)
#define IO96B1_PLL_A_MASK	BIT(2)
#define IO96B1_PLL_B_MASK	BIT(3)

/* supported DDR type list */
static const char *ddr_type_list[7] = {
		"DDR4", "DDR5", "DDR5_RDIMM", "LPDDR4", "LPDDR5", "QDRIV", "UNKNOWN"
};

/* Define an enumeration for ECC error types */
enum ecc_error_type {
	SINGLE_BIT_ERROR = 0,			/* 0b0000 */
	MULTIPLE_SINGLE_BIT_ERRORS = 1,		/* 0b0001 */
	DOUBLE_BIT_ERROR = 2,			/* 0b0010 */
	MULTIPLE_DOUBLE_BIT_ERRORS = 3,		/* 0b0011 */
	SINGLE_BIT_ERROR_SCRUBBING = 8,		/* 0b1000 */
	WRITE_LINK_SINGLE_BIT_ERROR = 9,	/* 0b1001 */
	WRITE_LINK_DOUBLE_BIT_ERROR = 10,	/* 0b1010 */
	READ_LINK_SINGLE_BIT_ERROR = 11,	/* 0b1011 */
	READ_LINK_DOUBLE_BIT_ERROR = 12,	/* 0b1100 */
	READ_MODIFY_WRITE_DOUBLE_BIT_ERROR = 13	/* 0b1101 */
};

/*
 * ecc error info
 *
 * @ip_type:		The IP type of the interface that produced the ECC interrupt.
 * @instance_id:	The instance ID of the interface that produced the ECC interrupt.
 * @ecc_err_source_id:	The source ID associated with the ECC event.
 * @ecc_err_type:	The ECC error type of the ECC event.
 * @ecc_err_addr_upper:	Upper 6 bits of the address of the read data that caused the ECC event.
 * @ecc_err_addr_lower:	Lower 32 bits of the address of the read data that caused the ECC event.
 */
struct ecc_err_info {
	u32 ip_type;
	u32 instance_id;
	u32 source_id;
	enum ecc_error_type err_type;
	u32 addr_upper;
	u32 addr_lower;
};

struct ecc_overflow_error_desc {
	int bit;
	const char *msg;
};

static const struct ecc_overflow_error_desc ecc_overflow_errors[] = {
	{ 0,  " - Single-bit error\n" },
	{ 1,  " - Multiple single-bit errors\n" },
	{ 2,  " - Double-bit error\n" },
	{ 3,  " - Multiple double-bit errors\n" },
	{ 8,  " - Single-bit error during ECC scrubbing\n" },
	{ 9,  " - Write link ECC single-bit error (LPDDR5 only)\n" },
	{ 10, " - Write link ECC double-bit error (LPDDR5 only)\n" },
	{ 11, " - Read link ECC single-bit error (LPDDR5 only)\n" },
	{ 12, " - Read link ECC double-bit error (LPDDR5 only)\n" },
	{ 13, " - RMW read link ECC double-bit error (LPDDR5 only)\n" },
};

static int is_ddr_csr_clkgen_locked(u8 io96b_pll)
{
	int ret = 0;
	const char *pll_names[MAX_IO96B_SUPPORTED][2] = {
		{"io96b_0 clkgenA", "io96b_0 clkgenB"},
		{"io96b_1 clkgenA", "io96b_1 clkgenB"}
	};
	u32 masks[MAX_IO96B_SUPPORTED][2] = {
		{IO96B0_PLL_A_MASK, IO96B0_PLL_B_MASK},
		{IO96B1_PLL_A_MASK, IO96B1_PLL_B_MASK}
	};
	u32 lock_masks[MAX_IO96B_SUPPORTED] = {
		DDR_CSR_CLKGEN_LOCKED_IO96B0_MASK,
		DDR_CSR_CLKGEN_LOCKED_IO96B1_MASK
	};

	for (int i = 0; i < MAX_IO96B_SUPPORTED ; i++) {
		/* Check for PLL_A */
		if (io96b_pll & masks[i][0]) {
			ret = wait_for_bit_le32((const void *)(ECC_INTSTATUS_SERR), lock_masks[i],
						true, TIMEOUT, false);

			if (ret) {
				debug("%s: ddr csr %s locked is timeout\n",
				      __func__, pll_names[i][0]);
				goto err;
			} else {
				debug("%s: ddr csr %s is successfully locked\n",
				      __func__, pll_names[i][0]);
			}
		}

		/* Check for PLL_B */
		if (io96b_pll & masks[i][1]) {
			ret = wait_for_bit_le32((const void *)(ECC_INISTATUS_DERR), lock_masks[i],
						true, TIMEOUT, false);

			if (ret) {
				debug("%s: ddr csr %s locked is timeout\n",
				      __func__, pll_names[i][1]);
				goto err;
			} else {
				debug("%s: ddr csr %s is successfully locked\n",
				      __func__, pll_names[i][1]);
			}
		}
	}

err:
	return ret;
}

/*
 * Mailbox request function
 * This function will send the request to IOSSM mailbox and wait for response return
 *
 * @io96b_csr_addr: CSR address for the target IO96B
 * @req:            Structure contain command request for IOSSM mailbox command
 * @resp_data_len:  User desire extra response data fields other than
 *		    CMD_RESPONSE_DATA_SHORT field on CMD_RESPONSE_STATUS
 * @resp:           Structure contain responses returned from the requested IOSSM
 *		    mailbox command
 */
int io96b_mb_req(phys_addr_t io96b_csr_addr, struct io96b_mb_req req,
		 u32 resp_data_len, struct io96b_mb_resp *resp)
{
	int i, ret;
	u32 cmd_req;

	if (!resp) {
		ret = -EINVAL;
		goto err;
	}

	/* Zero initialization for responses */
	resp->cmd_resp_status = 0;

	/* Ensure CMD_REQ is cleared before write any command request */
	ret = wait_for_bit_le32((const void *)(io96b_csr_addr + IOSSM_CMD_REQ_OFFSET),
				GENMASK(31, 0), false, TIMEOUT, false);
	if (ret) {
		printf("%s: Timeout of waiting DDR mailbox ready to be functioned!\n",
		       __func__);
		goto err;
	}

	/* Write CMD_PARAM_* */
	for (i = 0; i < NUM_CMD_PARAM ; i++) {
		switch (i) {
		case 0:
			if (req.cmd_param[0])
				writel(req.cmd_param[0], io96b_csr_addr + IOSSM_CMD_PARAM_0_OFFSET);
			break;
		case 1:
			if (req.cmd_param[1])
				writel(req.cmd_param[1], io96b_csr_addr + IOSSM_CMD_PARAM_1_OFFSET);
			break;
		case 2:
			if (req.cmd_param[2])
				writel(req.cmd_param[2], io96b_csr_addr + IOSSM_CMD_PARAM_2_OFFSET);
			break;
		case 3:
			if (req.cmd_param[3])
				writel(req.cmd_param[3], io96b_csr_addr + IOSSM_CMD_PARAM_3_OFFSET);
			break;
		case 4:
			if (req.cmd_param[4])
				writel(req.cmd_param[4], io96b_csr_addr + IOSSM_CMD_PARAM_4_OFFSET);
			break;
		case 5:
			if (req.cmd_param[5])
				writel(req.cmd_param[5], io96b_csr_addr + IOSSM_CMD_PARAM_5_OFFSET);
			break;
		case 6:
			if (req.cmd_param[6])
				writel(req.cmd_param[6], io96b_csr_addr + IOSSM_CMD_PARAM_6_OFFSET);
			break;
		}
	}

	/* Write CMD_REQ (IP_TYPE, IP_INSTANCE_ID, CMD_TYPE and CMD_OPCODE) */
	cmd_req = FIELD_PREP(CMD_TARGET_IP_TYPE_MASK, req.ip_type) |
		FIELD_PREP(CMD_TARGET_IP_INSTANCE_ID_MASK, req.ip_id) |
		FIELD_PREP(CMD_TYPE_MASK, req.usr_cmd_type) |
		FIELD_PREP(CMD_OPCODE_MASK, req.usr_cmd_opcode);
	writel(cmd_req, io96b_csr_addr + IOSSM_CMD_REQ_OFFSET);

	debug("%s: Write 0x%x to IOSSM_CMD_REQ_OFFSET 0x%llx\n", __func__, cmd_req,
	      io96b_csr_addr + IOSSM_CMD_REQ_OFFSET);

	/* Read CMD_RESPONSE_READY in CMD_RESPONSE_STATUS */
	ret = wait_for_bit_le32((const void *)(io96b_csr_addr + IOSSM_CMD_RESPONSE_STATUS_OFFSET),
				IOSSM_STATUS_COMMAND_RESPONSE_READY, true, TIMEOUT, false);

	/* read CMD_RESPONSE_STATUS */
	resp->cmd_resp_status = readl(io96b_csr_addr + IOSSM_CMD_RESPONSE_STATUS_OFFSET);

	debug("%s: CMD_RESPONSE_STATUS 0x%llx: 0x%x\n", __func__, io96b_csr_addr +
	      IOSSM_CMD_RESPONSE_STATUS_OFFSET, resp->cmd_resp_status);

	if (ret) {
		printf("%s: CMD_RESPONSE ERROR:\n", __func__);

		printf("%s: STATUS_GENERAL_ERROR: 0x%lx\n", __func__,
		       IOSSM_STATUS_GENERAL_ERROR(resp->cmd_resp_status));
		printf("%s: STATUS_CMD_RESPONSE_ERROR: 0x%lx\n", __func__,
		       IOSSM_STATUS_CMD_RESPONSE_ERROR(resp->cmd_resp_status));
		goto err;
	}

	/* read CMD_RESPONSE_DATA_* */
	for (i = 0; i < resp_data_len; i++) {
		switch (i) {
		case 0:
			resp->cmd_resp_data[i] =
					readl(io96b_csr_addr + IOSSM_CMD_RESPONSE_DATA_0_OFFSET);

			debug("%s: IOSSM_CMD_RESPONSE_DATA_0_OFFSET 0x%llx: 0x%x\n", __func__,
			      io96b_csr_addr + IOSSM_CMD_RESPONSE_DATA_0_OFFSET,
			      resp->cmd_resp_data[i]);
			break;
		case 1:
			resp->cmd_resp_data[i] =
					readl(io96b_csr_addr + IOSSM_CMD_RESPONSE_DATA_1_OFFSET);

			debug("%s: IOSSM_CMD_RESPONSE_DATA_1_OFFSET 0x%llx: 0x%x\n", __func__,
			      io96b_csr_addr + IOSSM_CMD_RESPONSE_DATA_1_OFFSET,
			      resp->cmd_resp_data[i]);
			break;
		case 2:
			resp->cmd_resp_data[i] =
					readl(io96b_csr_addr + IOSSM_CMD_RESPONSE_DATA_2_OFFSET);

			debug("%s: IOSSM_CMD_RESPONSE_DATA_2_OFFSET 0x%llx: 0x%x\n", __func__,
			      io96b_csr_addr + IOSSM_CMD_RESPONSE_DATA_2_OFFSET,
			      resp->cmd_resp_data[i]);
			break;
		default:
			resp->cmd_resp_data[i] = 0;
			printf("%s: Invalid response data\n", __func__);
		}
	}

	/* write CMD_RESPONSE_READY = 0 */
	clrbits_le32((u32 *)(uintptr_t)(io96b_csr_addr + IOSSM_CMD_RESPONSE_STATUS_OFFSET),
		     IOSSM_STATUS_COMMAND_RESPONSE_READY);

	debug("%s: After clear CMD_RESPONSE_READY bit: 0x%llx: 0x%x\n", __func__,
	      io96b_csr_addr + IOSSM_CMD_RESPONSE_STATUS_OFFSET,
	      readl(io96b_csr_addr + IOSSM_CMD_RESPONSE_STATUS_OFFSET));

err:
	return ret;
}

/*
 * Initial function to be called to set memory interface IP type and instance ID
 * IP type and instance ID need to be determined before sending mailbox command
 */
void io96b_mb_init(struct io96b_info *io96b_ctrl)
{
	int i, j;
	u32 mem_intf_info_0, mem_intf_info_1;

	debug("%s: num_instance %d\n", __func__, io96b_ctrl->num_instance);

	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		debug("%s: get memory interface IO96B %d\n", __func__, i);
		io96b_ctrl->io96b[i].mb_ctrl.num_mem_interface = 0;

		mem_intf_info_0 = readl(io96b_ctrl->io96b[i].io96b_csr_addr +
					IOSSM_MEM_INTF_INFO_0_OFFSET);
		mem_intf_info_1 = readl(io96b_ctrl->io96b[i].io96b_csr_addr +
					IOSSM_MEM_INTF_INFO_1_OFFSET);

		io96b_ctrl->io96b[i].mb_ctrl.ip_type[0] = FIELD_GET(INTF_IP_TYPE_MASK,
								    mem_intf_info_0);
		io96b_ctrl->io96b[i].mb_ctrl.ip_id[0] = FIELD_GET(INTF_INSTANCE_ID_MASK,
								  mem_intf_info_0);
		io96b_ctrl->io96b[i].mb_ctrl.ip_type[1] = FIELD_GET(INTF_IP_TYPE_MASK,
								    mem_intf_info_1);
		io96b_ctrl->io96b[i].mb_ctrl.ip_id[1] = FIELD_GET(INTF_INSTANCE_ID_MASK,
								  mem_intf_info_1);

		for (j = 0; j < MAX_MEM_INTERFACE_SUPPORTED; j++) {
			if (io96b_ctrl->io96b[i].mb_ctrl.ip_type[j]) {
				io96b_ctrl->io96b[i].mb_ctrl.num_mem_interface++;

				debug("%s: IO96B %d mem_interface %d: ip_type_ret: 0x%x\n",
				      __func__, i, j, io96b_ctrl->io96b[i].mb_ctrl.ip_type[j]);
				debug("%s: IO96B %d mem_interface %d: instance_id_ret: 0x%x\n",
				      __func__, i, j, io96b_ctrl->io96b[i].mb_ctrl.ip_id[j]);
			}
		}

		debug("%s: IO96B %d: num_mem_interface: 0x%x\n", __func__, i,
		      io96b_ctrl->io96b[i].mb_ctrl.num_mem_interface);
	}
}

int io96b_cal_status(phys_addr_t addr)
{
	u32 cal_success, cal_fail;
	phys_addr_t status_addr = addr + IOSSM_STATUS_OFFSET;
	u32 start = get_timer(0);

	do {
		if (get_timer(start) > TIMEOUT_60000MS) {
			printf("%s: SDRAM calibration for IO96B instance 0x%llx timeout!\n",
			       __func__, status_addr);
			hang();
		}

		udelay(1);
		schedule();

		/* Polling until getting any calibration result */
		cal_success = readl(status_addr) & IOSSM_STATUS_CAL_SUCCESS;
		cal_fail = readl(status_addr) & IOSSM_STATUS_CAL_FAIL;
	} while (!cal_success && !cal_fail);

	debug("%s: Calibration for IO96B instance 0x%llx done at %ld msec!\n",
	      __func__,  status_addr, get_timer(start));

	if (cal_success && !cal_fail)
		return 0;
	else
		return -EPERM;
}

void init_mem_cal(struct io96b_info *io96b_ctrl)
{
	int count, i, ret;

	/* Initialize overall calibration status */
	io96b_ctrl->overall_cal_status = false;

	if (io96b_ctrl->ckgen_lock) {
		ret = is_ddr_csr_clkgen_locked(io96b_ctrl->io96b_pll);
		if (ret) {
			printf("%s: iossm IO96B ckgena_lock is not locked\n", __func__);
			hang();
		}
	}

	/* Check initial calibration status for the assigned IO96B */
	count = 0;
	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		ret = io96b_cal_status(io96b_ctrl->io96b[i].io96b_csr_addr);
		if (ret) {
			io96b_ctrl->io96b[i].cal_status = false;

			printf("%s: Initial DDR calibration IO96B_%d failed %d\n", __func__,
			       i, ret);

			hang();
		}

		io96b_ctrl->io96b[i].cal_status = true;

		printf("%s: Initial DDR calibration IO96B_%d succeed\n", __func__, i);

		count++;
	}

	if (count == io96b_ctrl->num_instance)
		io96b_ctrl->overall_cal_status = true;
}

int get_mem_technology(struct io96b_info *io96b_ctrl)
{
	int i, j, ret = 0;
	u32 mem_technology_intf;
	u8 ddr_type_ret;

	u32 mem_technology_intf_offset[MAX_MEM_INTERFACE_SUPPORTED] = {
		IOSSM_MEM_TECHNOLOGY_INTF0_OFFSET,
		IOSSM_MEM_TECHNOLOGY_INTF1_OFFSET
	};

	/* Initialize ddr type */
	io96b_ctrl->ddr_type = ddr_type_list[6];

	/* Get and ensure all memory interface(s) same DDR type */
	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		for (j = 0; j < io96b_ctrl->io96b[i].mb_ctrl.num_mem_interface; j++) {
			mem_technology_intf = readl(io96b_ctrl->io96b[i].io96b_csr_addr +
						    mem_technology_intf_offset[j]);

			ddr_type_ret = FIELD_GET(INTF_DDR_TYPE_MASK, mem_technology_intf);

			if (!strcmp(io96b_ctrl->ddr_type, "UNKNOWN"))
				io96b_ctrl->ddr_type = ddr_type_list[ddr_type_ret];

			if (ddr_type_list[ddr_type_ret] != io96b_ctrl->ddr_type) {
				printf("%s: Mismatch DDR type on IO96B_%d\n", __func__, i);

				ret = -EINVAL;
				goto err;
			}
		}
	}

err:
	return ret;
}

int get_mem_width_info(struct io96b_info *io96b_ctrl)
{
	int i, j, ret = 0;
	u32 mem_width_info;
	phys_size_t memory_size, total_memory_size = 0;

	u32 mem_total_capacity_intf_offset[MAX_MEM_INTERFACE_SUPPORTED] = {
		IOSSM_MEM_TOTAL_CAPACITY_INTF0_OFFSET,
		IOSSM_MEM_TOTAL_CAPACITY_INTF1_OFFSET
	};

	/* Get all memory interface(s) total memory size on all instance(s) */
	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		memory_size = 0;
		for (j = 0; j < io96b_ctrl->io96b[i].mb_ctrl.num_mem_interface; j++) {
			mem_width_info = readl(io96b_ctrl->io96b[i].io96b_csr_addr +
					       mem_total_capacity_intf_offset[j]);

			io96b_ctrl->io96b[i].mb_ctrl.memory_size[j] =
				FIELD_GET(INTF_CAPACITY_GBITS_MASK, mem_width_info) * SZ_1G / SZ_8;

			if (io96b_ctrl->io96b[i].mb_ctrl.memory_size[j] != 0)
				memory_size += io96b_ctrl->io96b[i].mb_ctrl.memory_size[j];
		}

		if (!memory_size) {
			printf("%s: Failed to get valid memory size\n", __func__);
			ret = -EINVAL;
			goto err;
		}

		total_memory_size = total_memory_size + memory_size;
	}

	if (!total_memory_size) {
		printf("%s: Failed to get valid memory size\n", __func__);
		ret = -EINVAL;
	}

	io96b_ctrl->overall_size = total_memory_size;

err:
	return ret;
}

int ecc_enable_status(struct io96b_info *io96b_ctrl)
{
	int i, j, ret = 0;
	u32 ecc_enable_intf;
	bool ecc_status, ecc_status_set = false, inline_ecc = false;

	u32 ecc_enable_intf_offset[MAX_MEM_INTERFACE_SUPPORTED] = {
		IOSSM_ECC_ENABLE_INTF0_OFFSET,
		IOSSM_ECC_ENABLE_INTF1_OFFSET
	};

	/* Initialize ECC status */
	io96b_ctrl->ecc_status = false;
	io96b_ctrl->inline_ecc = false;

	/* Get and ensure all memory interface(s) same ECC status */
	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		for (j = 0; j < io96b_ctrl->io96b[i].mb_ctrl.num_mem_interface; j++) {
			ecc_enable_intf = readl(io96b_ctrl->io96b[i].io96b_csr_addr +
						ecc_enable_intf_offset[j]);

			ecc_status = (FIELD_GET(INTF_ECC_ENABLE_TYPE_MASK, ecc_enable_intf)
				    == 0) ? false : true;
			inline_ecc = FIELD_GET(INTF_ECC_TYPE_MASK, ecc_enable_intf);

			if (!ecc_status_set) {
				io96b_ctrl->ecc_status = ecc_status;

				if (io96b_ctrl->ecc_status)
					io96b_ctrl->inline_ecc = inline_ecc;

				ecc_status_set = true;
			}

			if (ecc_status != io96b_ctrl->ecc_status ||
			    (io96b_ctrl->ecc_status && inline_ecc != io96b_ctrl->inline_ecc)) {
				printf("%s: Mismatch DDR ECC status on IO96B_%d\n", __func__, i);

				ret = -EINVAL;
				goto err;
			}
		}
	}

	debug("%s: ECC enable status: %d\n", __func__, io96b_ctrl->ecc_status);

err:
	return ret;
}

bool is_double_bit_error(enum ecc_error_type err_type)
{
	switch (err_type) {
	case DOUBLE_BIT_ERROR:
	case MULTIPLE_DOUBLE_BIT_ERRORS:
	case WRITE_LINK_DOUBLE_BIT_ERROR:
	case READ_LINK_DOUBLE_BIT_ERROR:
	case READ_MODIFY_WRITE_DOUBLE_BIT_ERROR:
		return true;

	default:
		return false;
	}
}

bool ecc_interrupt_status(struct io96b_info *io96b_ctrl)
{
	int i, j;
	u32 ecc_err_status;
	u16 ecc_err_counter, ecc_overflow_status;
	bool ecc_error_flag = false;

	/* Get ECC double-bit error status */
	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		ecc_err_status = readl(io96b_ctrl->io96b[i].io96b_csr_addr +
					IOSSM_ECC_ERR_STATUS_OFFSET);

		ecc_err_counter = FIELD_GET(ECC_ERR_COUNTER_MASK, ecc_err_status);
		log_err("%s: ECC error number detected on IO96B_%d: %d\n",
			__func__, i, ecc_err_counter);

		ecc_overflow_status = FIELD_GET(ECC_ERR_OVERFLOW_MASK, ecc_err_status);
		if (ecc_overflow_status != 0) {
			log_err("ECC Error Overflow Flags:\n");

			for (int i = 0; i < ARRAY_SIZE(ecc_overflow_errors); i++) {
				if (ecc_overflow_status & BIT(ecc_overflow_errors[i].bit)) {
					log_err("%s", ecc_overflow_errors[i].msg);
				}
			}
		}

		if (ecc_err_counter != 0) {
			phys_addr_t address;
			u32 ecc_err_data;
			struct ecc_err_info err_info;

			address = io96b_ctrl->io96b[i].io96b_csr_addr +
				    IOSSM_ECC_ERR_DATA_START_OFFSET;

			for (j = 0; j < ecc_err_counter && j < MAX_ECC_ERR_INFO_COUNT; j++) {
				ecc_err_data = readl(address);
				err_info.err_type = FIELD_GET(ECC_ERR_TYPE_MASK,
							      ecc_err_data);
				err_info.ip_type = FIELD_GET(ECC_ERR_IP_TYPE_MASK,
							     ecc_err_data);
				err_info.instance_id = FIELD_GET(ECC_ERR_INSTANCE_ID_MASK,
								 ecc_err_data);
				err_info.source_id = FIELD_GET(ECC_ERR_SOURCE_ID_MASK,
							       ecc_err_data);
				err_info.addr_upper = FIELD_GET(ECC_ERR_ADDR_UPPER_MASK,
								ecc_err_data);
				err_info.addr_lower = readl(address + sizeof(u32));

				log_err(" %s: DDR ECC Error Detected on IO96B_%d number:%d\n",
					__func__, i, j);
				log_err(" - error info address :0x%llx\n", address);
				log_err(" - error ip type: %d\n", err_info.ip_type);
				log_err(" - error instance id: %d\n", err_info.instance_id);
				log_err(" - error source id: %d\n", err_info.source_id);
				log_err(" - error type: %s\n",
					is_double_bit_error(err_info.err_type) ?
					"Double-bit error" : "Single-bit error");
				log_err(" - error address: 0x%016llx\n",
					(u64)FIELD_PREP(ECC_FULL_ADDR_UPPER_MASK,
							err_info.addr_upper) |
					FIELD_PREP(ECC_FULL_ADDR_LOWER_MASK,
						   err_info.addr_lower));

				if (is_double_bit_error(err_info.err_type)) {
					if (!ecc_error_flag)
						ecc_error_flag = true;
				}

				address += sizeof(u32) * 2;
			}
		}
	}

	if (ecc_error_flag)
		log_err("\n%s: ECC double-bit error detected!\n", __func__);

	return ecc_error_flag;
}

int out_of_band_bist_mem_init_start(struct io96b_info *io96b_ctrl)
{
	struct io96b_mb_req usr_req;
	struct io96b_mb_resp usr_resp;
	int i, j, n, ret = 0;
	bool bist_start, bist_success;
	u32 mem_init_status_intf, start;

	u32 mem_init_status_offset[MAX_MEM_INTERFACE_SUPPORTED] = {
		IOSSM_MEM_INIT_STATUS_INTF0_OFFSET,
		IOSSM_MEM_INIT_STATUS_INTF1_OFFSET
	};

	/* Full memory initialization BIST performed on all memory interface(s) */
	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		for (j = 0; j < io96b_ctrl->io96b[i].mb_ctrl.num_mem_interface; j++) {
			bist_start = false;
			bist_success = false;

			/* Start memory initialization BIST on full memory address */
			IO96B_MB_REQ_SETUP(io96b_ctrl->io96b[i].mb_ctrl.ip_type[j],
					   io96b_ctrl->io96b[i].mb_ctrl.ip_id[j],
					   CMD_TRIG_CONTROLLER_OP, BIST_MEM_INIT_START,
					   BIST_FULL_MEM);

			ret = io96b_mb_req(io96b_ctrl->io96b[i].io96b_csr_addr,
					   usr_req, 0, &usr_resp);
			if (ret)
				goto err;

			bist_start = IOSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status)
				& BIT(0);

			if (!bist_start) {
				printf("%s: Failed to initialize memory on IO96B_%d\n", __func__,
				       i);
				printf("%s: BIST_MEM_INIT_START Error code 0x%lx\n", __func__,
				       IOSSM_STATUS_CMD_RESPONSE_ERROR(usr_resp.cmd_resp_status));

				ret = -EINVAL;
				goto err;
			}

			/* Polling for the initiated memory initialization BIST status */
			start = get_timer(0);
			while (!bist_success) {
				udelay(1);

				mem_init_status_intf = readl(io96b_ctrl->io96b[i].io96b_csr_addr +
							     mem_init_status_offset[j]);

				bist_success = FIELD_GET(INTF_BIST_STATUS_MASK,
							 mem_init_status_intf);

				if (!bist_success && (get_timer(start) > TIMEOUT)) {
					printf("%s: Timeout initialize memory on IO96B_%d\n",
					       __func__, i);
					printf("%s: BIST_MEM_INIT_STATUS Error code 0x%lx\n",
					       __func__,
					IOSSM_STATUS_CMD_RESPONSE_ERROR(usr_resp.cmd_resp_status));

					ret = -ETIMEDOUT;
					goto err;
				}
			}
		}

		debug("%s: Memory initialized successfully on IO96B_%d\n", __func__, i);
	}

err:
	return ret;
}

int bist_mem_init_by_addr(struct io96b_info *io96b_ctrl, int inst_id, int intf_id,
			  phys_addr_t base_addr, phys_size_t size)
{
	struct io96b_mb_req usr_req;
	struct io96b_mb_resp usr_resp;
	int n, ret = 0;
	bool bist_start, bist_success;
	u32 mem_exp, mem_init_status_intf, start;
	phys_size_t chunk_size;

	u32 mem_init_status_offset[MAX_MEM_INTERFACE_SUPPORTED] = {
		IOSSM_MEM_INIT_STATUS_INTF0_OFFSET,
		IOSSM_MEM_INIT_STATUS_INTF1_OFFSET
	};

	/* Check if size is a power of 2 */
	if (size == 0 || (size & (size - 1)) != 0) {
		ret = -EINVAL;
		goto err;
	}

	mem_exp = 0;
	chunk_size = size;

	while (chunk_size >>= 1)
		mem_exp++;

	/* Start memory initialization BIST on the specified address range */
	IO96B_MB_REQ_SETUP(io96b_ctrl->io96b[inst_id].mb_ctrl.ip_type[intf_id],
			   io96b_ctrl->io96b[inst_id].mb_ctrl.ip_id[intf_id],
			   CMD_TRIG_CONTROLLER_OP, BIST_MEM_INIT_START, 0);

	/* CMD_PARAM_0 bit[5:0] = mem_exp */
	/* CMD_PARAM_0 bit[6]: 0 - on the specified address range */
	usr_req.cmd_param[0] = FIELD_PREP(BIST_START_ADDR_SPACE_MASK, mem_exp);
	/* Extract address fields START_ADDR[31:0] */
	usr_req.cmd_param[1] = FIELD_GET(BIST_START_ADDR_LOW_MASK, base_addr);
	/* Extract address fields START_ADDR[37:32] */
	usr_req.cmd_param[2] = FIELD_GET(BIST_START_ADDR_HIGH_MASK, base_addr);
	/* Initialize memory to all zeros */
	usr_req.cmd_param[3] = 0;

	bist_start = false;
	bist_success = false;

	/* Send request to DDR controller */
	debug("%s:Initializing memory: Addr=0x%llx, Size=2^%u\n", __func__,
	      base_addr, mem_exp);
	ret = io96b_mb_req(io96b_ctrl->io96b[inst_id].io96b_csr_addr,
			   usr_req, 0, &usr_resp);
	if (ret)
		goto err;

	bist_start = IOSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status)
		& BIT(0);

	if (!bist_start) {
		printf("%s: Failed to initialize memory on IO96B_%d\n", __func__,
		       inst_id);
		printf("%s: BIST_MEM_INIT_START Error code 0x%lx\n", __func__,
		       IOSSM_STATUS_CMD_RESPONSE_ERROR(usr_resp.cmd_resp_status));

		ret = -EINVAL;
		goto err;
	}

	/* Polling for the initiated memory initialization BIST status */
	start = get_timer(0);
	while (!bist_success) {
		udelay(1);

		mem_init_status_intf = readl(io96b_ctrl->io96b[inst_id].io96b_csr_addr +
					     mem_init_status_offset[intf_id]);

		bist_success = FIELD_GET(INTF_BIST_STATUS_MASK, mem_init_status_intf);

		if (!bist_success && (get_timer(start) > TIMEOUT)) {
			printf("%s: Timeout initialize memory on IO96B_%d\n",
			       __func__, inst_id);
			printf("%s: BIST_MEM_INIT_STATUS Error code 0x%lx\n",
			       __func__,
			IOSSM_STATUS_CMD_RESPONSE_ERROR(usr_resp.cmd_resp_status));

			ret = -ETIMEDOUT;
			goto err;
		}
	}

	debug("%s:DDR memory initializationat 0x%llx completed.\n", __func__, base_addr);

err:
	return ret;
}

int inline_ecc_bist_mem_init(struct io96b_info *io96b_ctrl)
{
	int i, j, ret = 0;

	/* Memory initialization BIST performed on all memory interfaces */
	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		for (j = 0; j < io96b_ctrl->io96b[i].mb_ctrl.num_mem_interface; j++) {
			ret = bist_mem_init_by_addr(io96b_ctrl, i, j, 0,
						    io96b_ctrl->io96b[i].mb_ctrl.memory_size[j]);
			if (ret) {
				printf("Error: Memory init failed at Instance %d, Interface %d\n",
				       i, j);
				goto err;
			}
		}
	}

err:
	return ret;
}

int bist_mem_init_start(struct io96b_info *io96b_ctrl)
{
	if (io96b_ctrl->inline_ecc)
		return inline_ecc_bist_mem_init(io96b_ctrl);
	else
		return out_of_band_bist_mem_init_start(io96b_ctrl);
}

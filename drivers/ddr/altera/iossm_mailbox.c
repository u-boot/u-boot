// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 *
 */

#include <cpu_func.h>
#include <hang.h>
#include <log.h>
#include <string.h>
#include <time.h>
#include <wait_bit.h>
#include <asm/arch/base_addr_soc64.h>
#include <asm/arch/handoff_soc64.h>
#include <asm/arch/iossm_mailbox.h>
#include <asm/arch/mailbox_s10.h>
#include <asm/arch/socfpga_ddr_ecc.h>
#include <asm/arch/system_manager.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/iopoll.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

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
#define IOSSM_MAILBOX_SPEC_VERSION_MASK		GENMASK(2, 0)
#define IOSSM_MAILBOX_SPEC_VERSION(n)		FIELD_GET(IOSSM_MAILBOX_SPEC_VERSION_MASK, n)

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

/* IO96B ECC inject */
#define IO96B_SBE_SYNDROME		0xF4
#define IO96B_DBE_SYNDROME		0xFF
#define IO96B_ECC_INJECT_DELAY_US	40
#define IO96B_ECC_INJECT_VERIFY_US	5000
#define IO96B_ECC_INJECT_POLL_STEP_US	10
#define IO96B_ECC_DDR_TOUCH_ALIGN	64
#define IO96B_ECC_DDR_TOUCH_PATTERN	0xCAFEBABEFEEDFACEULL
/* ECC_ERR_STATUS[31:16] overflow bits used to verify inject when Q is full */
#define ECC_OVF_SBE_MASK		(BIT(0) | BIT(1))
#define ECC_OVF_DBE_MASK		(BIT(2) | BIT(3))

/* SDM dedicated EMIF ECC queue (HPS_PUSH / READ_EMIF_ERROR) */
#define EMIF_ECC_SDM_MAX_ENTRIES	MBOX_HPS_PUSH_EMIF_MAX_ENTRIES
#define EMIF_ECC_SDM_ENTRY_WORDS	MBOX_HPS_PUSH_EMIF_ENTRY_WORDS
/* One entry per READ_EMIF_ERROR: Error Count + 3 entry words */
#define EMIF_ECC_SDM_RESP_MAX_WORDS	(1 + EMIF_ECC_SDM_ENTRY_WORDS)
/* SDM trailer word2: io96b_inst[7:0] | reserved[15:8]=0 | overflow[31:16] */
#define EMIF_ECC_ENTRY_IO96B_INST_MASK	GENMASK(7, 0)
#define EMIF_ECC_ENTRY_OVERFLOW_MASK	GENMASK(31, 16)
#define MAX_ECC_ERR_INFO_COUNT		16
#define ECC_SDM_PUSH_MAX	(MAX_IO96B_SUPPORTED * MAX_ECC_ERR_INFO_COUNT)

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
#define INTF_CAPACITY_GBITS_MASK	GENMASK(31, 0)

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

/**
 * struct emif_ecc_sdm_entry - one EMIF ECC entry for HPS_PUSH / READ
 * @word0: IOSSM ECC_ERR_DATA[0]
 * @word1: IOSSM ECC_ERR_DATA[1] (DDR addr LSB)
 * @word2: SDM trailer
 */
struct emif_ecc_sdm_entry {
	u32 word0;
	u32 word1;
	u32 word2;
};

/**
 * struct ecc_sdm_push - IO96B ECC entries waiting for HPS_PUSH_EMIF_ECC
 * @entry: scanned entries, oldest first
 * @n: number of entries stored
 */
struct ecc_sdm_push {
	struct emif_ecc_sdm_entry entry[ECC_SDM_PUSH_MAX];
	unsigned int n;
};

static const phys_addr_t io96b_csr_base[MAX_IO96B_SUPPORTED] = {
	SOCFPGA_IO96B0_CSR_ADDRESS,
	SOCFPGA_IO96B1_CSR_ADDRESS,
};

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

/**
 * struct ecc_error_desc - IOSSM ECC_ERR_TYPE / overflow name mapping
 * @type: ECC_ERR_TYPE value from ECC_ERR_DATA[0]
 * @ovf_bit: bit in ECC_ERR_OVERFLOW after FIELD_GET([31:16]),
 *	per IOSSM Mailbox Specification; types 0..3 match TYPE,
 *	scrubbing/LPDDR5 use TYPE - 1 (bits 7..12)
 * @msg: shared print string for type and overflow
 */
struct ecc_error_desc {
	enum ecc_error_type type;
	u8 ovf_bit;
	const char *msg;
};

static const struct ecc_error_desc ecc_errors[] = {
	{ SINGLE_BIT_ERROR,			0,
	  "Single-bit error" },
	{ MULTIPLE_SINGLE_BIT_ERRORS,		1,
	  "Multiple single-bit errors" },
	{ DOUBLE_BIT_ERROR,			2,
	  "Double-bit error" },
	{ MULTIPLE_DOUBLE_BIT_ERRORS,		3,
	  "Multiple double-bit errors" },
	{ SINGLE_BIT_ERROR_SCRUBBING,		7,
	  "Single-bit error during ECC scrubbing" },
	{ WRITE_LINK_SINGLE_BIT_ERROR,		8,
	  "Write link ECC single-bit error (LPDDR5 only)" },
	{ WRITE_LINK_DOUBLE_BIT_ERROR,		9,
	  "Write link ECC double-bit error (LPDDR5 only)" },
	{ READ_LINK_SINGLE_BIT_ERROR,		10,
	  "Read link ECC single-bit error (LPDDR5 only)" },
	{ READ_LINK_DOUBLE_BIT_ERROR,		11,
	  "Read link ECC double-bit error (LPDDR5 only)" },
	{ READ_MODIFY_WRITE_DOUBLE_BIT_ERROR,	12,
	  "RMW read link ECC double-bit error (LPDDR5 only)" },
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

	/* Clear stale RESPONSE_READY before posting a new command */
	clrbits_le32((u32 *)(uintptr_t)(io96b_csr_addr +
					IOSSM_CMD_RESPONSE_STATUS_OFFSET),
		     IOSSM_STATUS_COMMAND_RESPONSE_READY);

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

static bool is_mailbox_spec_compatible(struct io96b_info *io96b_ctrl)
{
	u32 mailbox_header;
	u8 mailbox_spec_ver;

	mailbox_header = readl(io96b_ctrl->io96b[0].io96b_csr_addr +
				IOSSM_MAILBOX_HEADER_OFFSET);
	mailbox_spec_ver = IOSSM_MAILBOX_SPEC_VERSION(mailbox_header);
	debug("%s: IOSSM mailbox version: %d\n", __func__, mailbox_spec_ver);

	/* for now there are two mailbox spec versions, 0 and 1; only version 1 is compatible */
	if (!mailbox_spec_ver)
		return false;

	return true;
}

/*
 * Initial function to be called to set memory interface IP type and instance ID
 * IP type and instance ID need to be determined before sending mailbox command
 */
void io96b_mb_init(struct io96b_info *io96b_ctrl)
{
	int i, j;
	u32 mem_intf_info_0, mem_intf_info_1;

	if (!is_mailbox_spec_compatible(io96b_ctrl)) {
		printf("DDR: Failed to get compatible mailbox version\n");
		hang();
	}

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

/**
 * emif_ecc_pack_word2() - build trailer word2
 * @io96b_inst: IO96B instance index
 * @ecc_err_status: raw ECC_ERR_STATUS (overflow taken from [31:16])
 */
static u32 emif_ecc_pack_word2(unsigned int io96b_inst, u32 ecc_err_status)
{
	return (io96b_inst & EMIF_ECC_ENTRY_IO96B_INST_MASK) |
	       (ecc_err_status & EMIF_ECC_ENTRY_OVERFLOW_MASK);
}

/**
 * io96b_ecc_print_overflow() - print ECC_ERR_STATUS overflow bits
 * Overflow means more errors occurred than the queue retained.
 * @overflow: ECC_ERR_OVERFLOW after FIELD_GET of ECC_ERR_STATUS[31:16]
 */
static void io96b_ecc_print_overflow(u32 overflow)
{
	int i;

	if (!overflow)
		return;

	for (i = 0; i < ARRAY_SIZE(ecc_errors); i++) {
		if (overflow & BIT(ecc_errors[i].ovf_bit))
			log_err(" - overflow: %s\n", ecc_errors[i].msg);
	}
}

/**
 * ecc_overflow_has_dbe() - any DBE-class bit set in ECC_ERR_OVERFLOW?
 * @overflow: ECC_ERR_OVERFLOW after FIELD_GET of ECC_ERR_STATUS[31:16]
 *
 * Retained Q entries can be all SBE after overflow; DBE overflow still
 * means a double-bit error occurred and recovery must run.
 */
static bool ecc_overflow_has_dbe(u32 overflow)
{
	int i;

	if (!overflow)
		return false;

	for (i = 0; i < ARRAY_SIZE(ecc_errors); i++) {
		if ((overflow & BIT(ecc_errors[i].ovf_bit)) &&
		    is_double_bit_error(ecc_errors[i].type))
			return true;
	}

	return false;
}

/**
 * io96b_ecc_print_error_type() - print ECC_ERR_TYPE from one entry
 * @err_type: ECC_ERR_TYPE field from ECC_ERR_DATA[0]
 */
static void io96b_ecc_print_error_type(enum ecc_error_type err_type)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ecc_errors); i++) {
		if (ecc_errors[i].type == err_type) {
			log_err(" - error type: %s (0x%x)\n",
				ecc_errors[i].msg, (unsigned int)err_type);
			return;
		}
	}

	log_err(" - error type: Unknown (0x%x)\n", (unsigned int)err_type);
}

/**
 * io96b_ecc_print_emif_entry() - print one EMIF/IOSSM ECC entry
 * @word0: ECC_ERR_DATA[0]
 * @word1: ECC_ERR_DATA[1] (DDR addr LSB)
 * @word2: io96b_inst[7:0] | reserved[15:8]=0 | overflow[31:16]
 * @idx: entry index for display
 *
 * Return: true if a double-bit error occurred
 */
static bool io96b_ecc_print_emif_entry(u32 word0, u32 word1, u32 word2,
				       unsigned int idx)
{
	enum ecc_error_type err_type = FIELD_GET(ECC_ERR_TYPE_MASK, word0);
	u32 addr_upper = FIELD_GET(ECC_ERR_ADDR_UPPER_MASK, word0);
	u32 overflow = FIELD_GET(EMIF_ECC_ENTRY_OVERFLOW_MASK, word2);
	unsigned int io96b_inst = FIELD_GET(EMIF_ECC_ENTRY_IO96B_INST_MASK,
					    word2);

	log_err(" DDR ECC Error Detected on IO96B_%u number:%u\n",
		io96b_inst, idx);
	io96b_ecc_print_overflow(overflow);
	log_err(" - error ip type: %lu\n",
		FIELD_GET(ECC_ERR_IP_TYPE_MASK, word0));
	log_err(" - error instance id: %lu\n",
		FIELD_GET(ECC_ERR_INSTANCE_ID_MASK, word0));
	log_err(" - error source id: %lu\n",
		FIELD_GET(ECC_ERR_SOURCE_ID_MASK, word0));
	io96b_ecc_print_error_type(err_type);
	log_err(" - error address: 0x%016llx\n",
		(u64)FIELD_PREP(ECC_FULL_ADDR_UPPER_MASK, addr_upper) |
		FIELD_PREP(ECC_FULL_ADDR_LOWER_MASK, word1));

	return is_double_bit_error(err_type) || ecc_overflow_has_dbe(overflow);
}

/**
 * ecc_sdm_push_add() - store one scanned IO96B ECC entry
 * @push: collection filled during the IO96B scan
 * @word0: IOSSM ECC_ERR_DATA[0]
 * @word1: IOSSM ECC_ERR_DATA[1] (DDR addr LSB)
 * @word2: io96b_inst[7:0] | reserved[15:8]=0 | overflow[31:16]
 *
 * Stops once ECC_SDM_PUSH_MAX entries are stored.
 */
static void ecc_sdm_push_add(struct ecc_sdm_push *push,
			     u32 word0, u32 word1, u32 word2)
{
	if (!IS_ENABLED(CONFIG_XPL_BUILD))
		return;

	if (push->n >= ECC_SDM_PUSH_MAX) {
		log_err("IO96B ECC: SDM push buffer full, dropping entry\n");
		return;
	}

	push->entry[push->n].word0 = word0;
	push->entry[push->n].word1 = word1;
	push->entry[push->n].word2 = word2;
	push->n++;
}

/**
 * ecc_sdm_push_flush() - append stored entries to the SDM EMIF queue
 * @push: entries collected from the IO96B scan
 *
 * Each command carries at most EMIF_ECC_SDM_MAX_ENTRIES and uses R=0.
 */
static void ecc_sdm_push_flush(const struct ecc_sdm_push *push)
{
	unsigned int off;

	if (!IS_ENABLED(CONFIG_XPL_BUILD))
		return;

	for (off = 0; off < push->n;) {
		unsigned int cnt = push->n - off;
		int ret;

		if (cnt > EMIF_ECC_SDM_MAX_ENTRIES)
			cnt = EMIF_ECC_SDM_MAX_ENTRIES;

		ret = mbox_hps_push_emif_ecc(&push->entry[off].word0, cnt);
		if (ret) {
			log_err("IO96B ECC: HPS_PUSH_EMIF_ECC failed (%d)\n",
				ret);
			return;
		}

		off += cnt;
	}
}

bool ecc_interrupt_status(struct io96b_info *io96b_ctrl)
{
	struct ecc_sdm_push sdm = { };
	int i, j;
	u32 ecc_err_status;
	u16 ecc_err_counter;
	bool ecc_error_flag = false;

	/* Get ECC double-bit error status */
	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		ecc_err_status = readl(io96b_ctrl->io96b[i].io96b_csr_addr +
					IOSSM_ECC_ERR_STATUS_OFFSET);

		ecc_err_counter = FIELD_GET(ECC_ERR_COUNTER_MASK, ecc_err_status);

		if (ecc_err_counter != 0) {
			phys_addr_t address;
			u32 word0, word1, word2;

			address = io96b_ctrl->io96b[i].io96b_csr_addr +
				    IOSSM_ECC_ERR_DATA_START_OFFSET;
			word2 = emif_ecc_pack_word2(i, ecc_err_status);

			for (j = 0; j < ecc_err_counter && j < MAX_ECC_ERR_INFO_COUNT; j++) {
				word0 = readl(address);
				word1 = readl(address + sizeof(u32));

				if (io96b_ecc_print_emif_entry(word0, word1,
							       word2, j))
					ecc_error_flag = true;

				if (IS_ENABLED(CONFIG_XPL_BUILD))
					ecc_sdm_push_add(&sdm, word0, word1,
							 word2);

				address += sizeof(u32) * 2;
			}
		}
	}

	if (ecc_error_flag)
		log_err("\n%s: ECC double-bit error detected!\n", __func__);

	if (IS_ENABLED(CONFIG_XPL_BUILD))
		ecc_sdm_push_flush(&sdm);

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

/**
 * io96b_csr_from_mask() - fill IO96B CSR bases from an instance bitmask
 * @io96b_ctrl: IO96B info to update
 * @mask: bit N set => IO96B_N present (compile-time CSR base)
 *
 * Return: 0 on success, -ENODEV if no instance bit is set.
 */
static int io96b_csr_from_mask(struct io96b_info *io96b_ctrl, u32 mask)
{
	u8 i, count = 0;

	for (i = 0; i < MAX_IO96B_SUPPORTED; i++) {
		if (!(mask & BIT(i)))
			continue;
		io96b_ctrl->io96b[count].io96b_csr_addr = io96b_csr_base[i];
		debug("%s: IO96B_%u CSR 0x%llx enabled\n", __func__, i,
		      (unsigned long long)io96b_ctrl->io96b[count].io96b_csr_addr);
		count++;
	}

	io96b_ctrl->num_instance = count;
	if (!count)
		return -ENODEV;

	return 0;
}

int io96b_csr_from_handoff_table(struct io96b_info *io96b_ctrl,
				 const u32 *handoff, u32 handoff_len)
{
	u32 mask = 0;

	if (!io96b_ctrl || !handoff || handoff_len < SOC64_HANDOFF_IO96B_LEN)
		return -EINVAL;

	if (IS_ENABLED(CONFIG_ARCH_SOCFPGA_AGILEX7M)) {
		/* handoff[1] bit N => IO96B_N present (DDR/EMIF; HBM uses UIB) */
		mask = handoff[1];
	} else {
		/* dual EMIF => both IO96B_0/1; else IO96B_0 only */
		mask = FIELD_GET(SOC64_HANDOFF_SDRAM_IO96B0_DUAL_EMIF_MASK,
				 handoff[SOC64_HANDOFF_SDRAM_PORT_EMIF_CONFIG_OFFSET]) ?
		       GENMASK(1, 0) : BIT(0);
	}

	return io96b_csr_from_mask(io96b_ctrl, mask);
}

int io96b_csr_from_handoff(struct io96b_info *io96b_ctrl)
{
	u32 handoff[SOC64_HANDOFF_IO96B_LEN];
	u32 mask = 0;
	int ret;

	if (!io96b_ctrl)
		return -EINVAL;

	if (!IS_ENABLED(CONFIG_XPL_BUILD)) {
		if (IS_ENABLED(CONFIG_ARCH_SOCFPGA_AGILEX7M)) {
			mask = FIELD_GET(ALT_SYSMGR_SCRATCH_REG_8_IO96B_HPS_MASK,
					 readl(socfpga_get_sysmgr_addr() +
					       SYSMGR_SOC64_BOOT_SCRATCH_COLD8));
			debug("%s: IO96B mask 0x%x from boot scratch cold8\n",
			      __func__, mask);
		} else if (IS_ENABLED(CONFIG_ARCH_SOCFPGA_AGILEX5)) {
			mask = FIELD_GET(ALT_SYSMGR_SCRATCH_REG_3_DDR_EMIF_INFO_MASK,
					 readl(socfpga_get_sysmgr_addr() +
					       SYSMGR_SOC64_BOOT_SCRATCH_COLD3)) ?
			       GENMASK(1, 0) : BIT(0);
			debug("%s: IO96B mask 0x%x from boot scratch cold3\n",
			      __func__, mask);
		} else {
			return -EOPNOTSUPP;
		}

		return io96b_csr_from_mask(io96b_ctrl, mask);
	}

	ret = socfpga_handoff_read((void *)SOC64_HANDOFF_IO96B_BASE, handoff,
				   SOC64_HANDOFF_IO96B_LEN);
	if (ret)
		return ret;

	return io96b_csr_from_handoff_table(io96b_ctrl, handoff,
					    SOC64_HANDOFF_IO96B_LEN);
}

/**
 * io96b_ecc_safe_ddr_touch_addr() - poke address for ECC inject
 * @addrp: filled with a cache-line-aligned poke address on success
 *
 * gd->ram_base + 32 MiB (past BL31) + rolling cache-line slot within 1 MiB.
 * Stays far below U-Boot proper at the top of bank 0.
 *
 * Return: 0 on success, -ENOSPC if DRAM is too small for that window.
 */
static int io96b_ecc_safe_ddr_touch_addr(phys_addr_t *addrp)
{
	const phys_addr_t align = IO96B_ECC_DDR_TOUCH_ALIGN;
	const phys_size_t win = SZ_1M;
	static u32 slot;
	phys_addr_t lo, addr, span;

	if (!addrp)
		return -EINVAL;

	/* Need room for the 32 MiB skip plus the 1 MiB poke window. */
	if (gd->ram_size < SZ_32M + win) {
		printf("%s: DRAM too small for safe poke window (%llu MiB)\n",
		       __func__, (unsigned long long)(gd->ram_size >> 20));
		return -ENOSPC;
	}

	lo = (gd->ram_base + SZ_32M + align - 1) & ~(align - 1);
	span = win / align;
	addr = lo + ((slot++ % span) * align);

	*addrp = addr;
	return 0;
}

/**
 * io96b_ecc_cmd_req() - build an IOSSM CMD_REQ for one memory interface
 * @ip_type: IP type from MEM_INTF_INFO
 * @ip_id: IP instance id from MEM_INTF_INFO
 * @opcode: mailbox opcode
 *
 * Return: CMD_REQ value for this interface.
 */
static u32 io96b_ecc_cmd_req(u32 ip_type, u32 ip_id, u32 opcode)
{
	return FIELD_PREP(CMD_TARGET_IP_TYPE_MASK, ip_type) |
	       FIELD_PREP(CMD_TARGET_IP_INSTANCE_ID_MASK, ip_id) |
	       FIELD_PREP(CMD_TYPE_MASK, CMD_TRIG_CONTROLLER_OP) |
	       FIELD_PREP(CMD_OPCODE_MASK, opcode);
}

/**
 * io96b_ecc_err_queue_clear() - clear one interface's IOSSM ECC error queue
 * @base: IO96B CSR base
 * @ip_type: IP type for this memory interface
 * @ip_id: IP instance id for this memory interface
 *
 * Post CLEAR and wait until CMD_REQ goes idle again so the next mailbox
 * command (e.g. ECC_INJECT_ERROR) cannot overwrite an in-flight CLEAR.
 * Drain CMD_RESPONSE_READY if the HW asserts it; do not require READY.
 *
 * Return: 0 on success, negative errno on timeout or CLEAR response error.
 */
static int io96b_ecc_err_queue_clear(phys_addr_t base, u32 ip_type, u32 ip_id)
{
	u32 cmd_req, resp_status;
	int ret;

	ret = wait_for_bit_le32((const void *)(base + IOSSM_CMD_REQ_OFFSET),
				GENMASK(31, 0), false, TIMEOUT, false);
	if (ret) {
		printf("%s: timeout waiting for IOSSM CMD_REQ @ 0x%llx\n",
		       __func__, (unsigned long long)base);
		return ret;
	}

	if (readl(base + IOSSM_CMD_RESPONSE_STATUS_OFFSET))
		writel(0, base + IOSSM_CMD_RESPONSE_STATUS_OFFSET);

	cmd_req = io96b_ecc_cmd_req(ip_type, ip_id, ECC_ERROR_QUEUE_CLEAR);
	writel(cmd_req, base + IOSSM_CMD_REQ_OFFSET);

	/* Wait until CLEAR is consumed before any later CMD_REQ write. */
	ret = wait_for_bit_le32((const void *)(base + IOSSM_CMD_REQ_OFFSET),
				GENMASK(31, 0), false, TIMEOUT, false);
	if (ret) {
		printf("%s: timeout waiting CLEAR complete @ 0x%llx\n",
		       __func__, (unsigned long long)base);
		return ret;
	}

	/*
	 * If IOSSM posted a response, ACK READY then fail on error fields.
	 * Do not treat a rejected CLEAR as success for status_io.
	 */
	resp_status = readl(base + IOSSM_CMD_RESPONSE_STATUS_OFFSET);
	if (resp_status & IOSSM_STATUS_COMMAND_RESPONSE_READY) {
		clrbits_le32((u32 *)(uintptr_t)(base +
				IOSSM_CMD_RESPONSE_STATUS_OFFSET),
			     IOSSM_STATUS_COMMAND_RESPONSE_READY);

		if (IOSSM_STATUS_GENERAL_ERROR(resp_status) ||
		    IOSSM_STATUS_CMD_RESPONSE_ERROR(resp_status))
			return -EIO;
	}

	return 0;
}

/**
 * io96b_ecc_err_status() - read IOSSM ECC_ERR_STATUS
 * @base: IO96B CSR base
 *
 * Return: raw ECC_ERR_STATUS register value.
 */
static u32 io96b_ecc_err_status(phys_addr_t base)
{
	return readl(base + IOSSM_ECC_ERR_STATUS_OFFSET);
}

/**
 * io96b_ecc_inject_visible() - inject reflected in ECC_ERR_STATUS?
 * @status: current ECC_ERR_STATUS value
 * @ctr_before: ECC_ERR_COUNTER before inject
 * @dbe: true if DBE syndrome was armed
 *
 * Room in Q -> counter must rise. Q full -> matching SBE/DBE overflow bits.
 *
 * Return: true if the inject is visible in @status.
 */
static bool io96b_ecc_inject_visible(u32 status, u16 ctr_before, bool dbe)
{
	u16 ovf_mask = dbe ? ECC_OVF_DBE_MASK : ECC_OVF_SBE_MASK;

	if (ctr_before < MAX_ECC_ERR_INFO_COUNT)
		return FIELD_GET(ECC_ERR_COUNTER_MASK, status) > ctr_before;

	return (FIELD_GET(ECC_ERR_OVERFLOW_MASK, status) & ovf_mask) != 0;
}

/**
 * io96b_ecc_wait_inject_done() - wait for Q depth or overflow to reflect inject
 * @base: IO96B CSR base
 * @ctr_before: ECC_ERR_COUNTER before inject
 * @dbe: true if DBE syndrome was armed
 *
 * Return: 0 if inject is visible, -ETIMEDOUT otherwise.
 */
static int io96b_ecc_wait_inject_done(phys_addr_t base, u16 ctr_before,
				      bool dbe)
{
	void __iomem *addr = (void __iomem *)(uintptr_t)
			     (base + IOSSM_ECC_ERR_STATUS_OFFSET);
	u32 status;

	return readl_poll_sleep_timeout(addr, status,
					io96b_ecc_inject_visible(status,
								 ctr_before,
								 dbe),
					IO96B_ECC_INJECT_POLL_STEP_US,
					IO96B_ECC_INJECT_VERIFY_US);
}

/**
 * io96b_ecc_trigger() - issue IOSSM inject CMD and DDR write/read-back
 * @base: IO96B CSR base
 * @ddr_target: cache-line-aligned DDR poke address
 * @syndrome: SBE or DBE syndrome for CMD_PARAM_0
 * @cmd_req: ECC_INJECT_ERROR CMD_REQ for this interface
 */
static void io96b_ecc_trigger(phys_addr_t base, phys_addr_t ddr_target,
			      u32 syndrome, u32 cmd_req)
{
	u64 pattern = IO96B_ECC_DDR_TOUCH_PATTERN;

	if (readl(base + IOSSM_CMD_RESPONSE_STATUS_OFFSET))
		writel(0, base + IOSSM_CMD_RESPONSE_STATUS_OFFSET);

	writel(syndrome, base + IOSSM_CMD_PARAM_0_OFFSET);
	writel(cmd_req, base + IOSSM_CMD_REQ_OFFSET);
	udelay(IO96B_ECC_INJECT_DELAY_US);

	writeq(pattern, ddr_target);
	flush_dcache_range(ddr_target, ddr_target + sizeof(pattern));

	invalidate_dcache_range(ddr_target, ddr_target + sizeof(pattern));
	(void)readq(ddr_target);
}

int io96b_ecc_inject(struct io96b_info *io96b_ctrl, bool dbe)
{
	phys_addr_t base, ddr_target;
	struct io96b_mb_ctrl *mb;
	u32 status, cmd_req;
	u16 ctr_before;
	int ret;

	if (!io96b_ctrl || !io96b_ctrl->num_instance)
		return -EINVAL;

	mb = &io96b_ctrl->io96b[0].mb_ctrl;
	if (!mb->num_mem_interface)
		return -EINVAL;

	base = io96b_ctrl->io96b[0].io96b_csr_addr;
	ret = io96b_ecc_safe_ddr_touch_addr(&ddr_target);
	if (ret)
		return ret;

	status = io96b_ecc_err_status(base);
	ctr_before = FIELD_GET(ECC_ERR_COUNTER_MASK, status);
	cmd_req = io96b_ecc_cmd_req(mb->ip_type[0], mb->ip_id[0],
				    ECC_INJECT_ERROR);

	debug("%s: %s inst=0 intf=0 target=0x%llx counter=%u ovf=0x%x\n",
	      __func__, dbe ? "DBE" : "SBE",
	      (unsigned long long)ddr_target, ctr_before,
	      (u16)FIELD_GET(ECC_ERR_OVERFLOW_MASK, status));

	io96b_ecc_trigger(base, ddr_target,
			  dbe ? IO96B_DBE_SYNDROME : IO96B_SBE_SYNDROME,
			  cmd_req);

	ret = io96b_ecc_wait_inject_done(base, ctr_before, dbe);
	if (!ret)
		return 0;

	status = io96b_ecc_err_status(base);
	printf("%s: %s inject at 0x%llx not visible (counter %u ovf 0x%x)\n",
	       __func__, dbe ? "DBE" : "SBE",
	       (unsigned long long)ddr_target,
	       (u16)FIELD_GET(ECC_ERR_COUNTER_MASK, status),
	       (u16)FIELD_GET(ECC_ERR_OVERFLOW_MASK, status));
	return -EIO;
}

int io96b_ecc_status_iossm(struct io96b_info *io96b_ctrl, bool *dbe)
{
	int i, j, ret;

	if (!io96b_ctrl || !dbe)
		return -EINVAL;

	*dbe = ecc_interrupt_status(io96b_ctrl);

	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		struct io96b_mb_ctrl *mb = &io96b_ctrl->io96b[i].mb_ctrl;

		for (j = 0; j < mb->num_mem_interface; j++) {
			ret = io96b_ecc_err_queue_clear(io96b_ctrl->io96b[i].io96b_csr_addr,
							mb->ip_type[j],
							mb->ip_id[j]);
			if (ret)
				return ret;
		}
	}

	if (io96b_ctrl->num_instance)
		debug("IO96B ECC: IOSSM error Q cleared\n");

	return 0;
}

#if CONFIG_IS_ENABLED(CMD_SOCFPGA_DDR_ECC)
int io96b_ecc_status_sdm(bool *dbe)
{
	u32 resp[EMIF_ECC_SDM_RESP_MAX_WORDS];
	u32 resp_len;
	u32 depth, idx = 0;
	bool saw_entry = false;
	int ret;

	if (!dbe)
		return -EINVAL;

	*dbe = false;

	while (idx < EMIF_ECC_SDM_MAX_ENTRIES) {
		resp_len = ARRAY_SIZE(resp);
		ret = mbox_read_emif_error(resp, &resp_len);
		if (ret)
			return -EIO;

		if (resp_len < 1) {
			printf("%s: empty mailbox response\n", __func__);
			return -EIO;
		}

		depth = resp[0];
		if (depth == 0) {
			if (!saw_entry)
				printf("IO96B ECC status_sdm: no ECC errors in SDM queue\n");
			else
				printf("IO96B ECC status_sdm: drained %u ECC error(s)\n",
				       idx);
			break;
		}

		if (depth > EMIF_ECC_SDM_MAX_ENTRIES ||
		    resp_len < EMIF_ECC_SDM_RESP_MAX_WORDS) {
			printf("%s: bad Error Count %u (resp_len %u)\n",
			       __func__, depth, resp_len);
			return -EIO;
		}

		if (!saw_entry) {
			printf("IO96B ECC status_sdm: %u ECC error(s) in SDM queue\n",
			       depth);
			saw_entry = true;
		}

		if (io96b_ecc_print_emif_entry(resp[1], resp[2], resp[3], idx))
			*dbe = true;
		idx++;
	}

	if (idx == EMIF_ECC_SDM_MAX_ENTRIES) {
		/* Cap defensive loop; queue should be empty after 8 pops */
		resp_len = ARRAY_SIZE(resp);
		ret = mbox_read_emif_error(resp, &resp_len);
		if (!ret && resp_len >= 1 && resp[0] != 0)
			printf("%s: warning: SDM ECC queue still non-empty after %u reads\n",
			       __func__, idx);
	}

	return 0;
}

/**
 * io96b_ddr_ecc_status_io() - ddr_ecc_ops.status_io for IO96B
 * @dbe: set true if a DBE was observed in the IO ECC queue
 *
 * Loads handoff CSR bases, reads interface identity, then drains the
 * IO ECC queue.
 *
 * Return: 0 on success, negative errno otherwise.
 */
static int io96b_ddr_ecc_status_io(bool *dbe)
{
	struct io96b_info io96b;
	int ret;

	if (!dbe)
		return -EINVAL;

	*dbe = false;
	memset(&io96b, 0, sizeof(io96b));

	ret = io96b_csr_from_handoff(&io96b);
	if (ret) {
		if (ret == -ENODEV)
			printf("No IO96B instance in handoff\n");
		else
			printf("Failed to read DDR handoff\n");
		return ret;
	}

	io96b_mb_init(&io96b);

	return io96b_ecc_status_iossm(&io96b, dbe);
}

/**
 * io96b_ddr_ecc_inject() - ddr_ecc_ops.inject for IO96B
 * @dbe: true for DBE, false for SBE
 *
 * Return: 0 on success, -EOPNOTSUPP if ECC disabled, or negative errno.
 */
static int io96b_ddr_ecc_inject(bool dbe)
{
	struct io96b_info io96b;
	int ret;

	memset(&io96b, 0, sizeof(io96b));

	ret = io96b_csr_from_handoff(&io96b);
	if (ret) {
		if (ret == -ENODEV)
			printf("No IO96B instance in handoff\n");
		else
			printf("Failed to read DDR handoff\n");
		return ret;
	}

	io96b_mb_init(&io96b);

	ret = ecc_enable_status(&io96b);
	if (ret || !io96b.ecc_status) {
		printf("DDR ECC is not enabled\n");
		return -EOPNOTSUPP;
	}

	return io96b_ecc_inject(&io96b, dbe);
}

static const struct ddr_ecc_ops io96b_ddr_ecc_ops = {
	.name = "io96b",
	.status_io = io96b_ddr_ecc_status_io,
	.status_sdm = io96b_ecc_status_sdm,
	.inject = io96b_ddr_ecc_inject,
};

const struct ddr_ecc_ops *ddr_ecc_get_ops(void)
{
	return &io96b_ddr_ecc_ops;
}
#endif /* CONFIG_CMD_SOCFPGA_DDR_ECC */

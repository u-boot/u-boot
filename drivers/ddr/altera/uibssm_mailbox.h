/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#define TIMEOUT_120000MS	120000
#define TIMEOUT			TIMEOUT_120000MS

#define UIBSSM_CMD_RESPONSE_DATA_SHORT_MASK	GENMASK(31, 16)
#define UIBSSM_CMD_RESPONSE_DATA_SHORT(x)	FIELD_GET(UIBSSM_CMD_RESPONSE_DATA_SHORT_MASK, x)
#define UIBSSM_CMD_RESPONSE_ERROR_MASK		GENMASK(7, 5)
#define UIBSSM_CMD_RESPONSE_ERROR(x)		FIELD_GET(UIBSSM_CMD_RESPONSE_ERROR_MASK, x)
#define UIBSSM_GENERAL_ERROR_MASK		GENMASK(4, 1)
#define UIBSSM_GENERAL_ERROR(x)			FIELD_GET(UIBSSM_GENERAL_ERROR_MASK, x)

/* UIB Responder Initialization Control Register */
#define UIB_R_INITCTL_OFFSET			0x10
#define UIB_R_INITCTL_INITREQ_MASK		BIT(0)
#define UIB_R_INITCTL_INITTYPE_MASK		GENMASK(11, 8)
#define UIB_R_INITCTL_INITREQ(x)		FIELD_PREP(UIB_R_INITCTL_INITREQ_MASK, x)
#define UIB_R_INITCTL_INITTYPE(x)		FIELD_PREP(UIB_R_INITCTL_INITTYPE_MASK, x)
#define UIB_RST_REQUEST_WITH_CAL		5

/* UIB Initialization control and status registers */
#define UIB_R_INITSTS_OFFSET		0x14
#define UIB_R_INITSTS_INITSTS_PASS	BIT(1)
#define MAX_UIB_SUPPORTED		8

#define UIB_R_MBWRCTL			0x20
#define UIB_R_MBWRADDR			0x24
#define UIB_R_MBWRDATA			0x28
#define UIB_R_MBWRCTL_MBWRADDR_VALID	BIT(0)
#define UIB_R_MBWRCTL_MBWRDATA_VALID	BIT(4)
#define UIB_R_MBWRCTL_MBWRDATA_END	BIT(7)

#define UIB_R_MBRDCTL			0x30
#define UIB_R_MBRDADDR			0x34
#define UIB_R_MBRRDATA			0x38
#define UIB_R_MBRDCTL_MBRDADDR_VALID	BIT(0)
#define UIB_R_MBRDCTL_MBRDDATA_VALID	BIT(4)
#define UIB_R_MBRDCTL_MBRDDATA_END	BIT(7)

/* Responder Error Mask Register */
#define UIB_R_ERRMSK_PSEUDO_CH0_OFFSET	0x520
#define UIB_R_ERRMSK_PSEUDO_CH1_OFFSET	0X820
#define UIB_DRAM_SBE_MSK		BIT(25)
#define UIB_INTERNAL_CORR_ERR_MSK	BIT(30)
#define UIB_DRAM_SBE(x)			FIELD_PREP(UIB_DRAM_SBE_MSK, x)
#define UIB_INTERNAL_CORR_ERR(x)	FIELD_PREP(UIB_INTERNAL_CORR_ERR_MSK, x)

/* CMD_REQ Register Definition */
#define CMD_TYPE_MASK			GENMASK(23, 16)
#define CMD_OPCODE_MASK			GENMASK(15, 0)

/* supported mailbox command type */
enum uibssm_mailbox_cmd_type  {
	UIB_CMD_TRIG_CONTROLLER_OP = 0x04
};

/* supported mailbox command opcode */
enum uibssm_mailbox_cmd_opcode  {
	UIB_BIST_MEM_INIT_START = 0x0303,
	UIB_BIST_MEM_INIT_STATUS
};

/* CMD_PARAM_0 for opcode UIB_BIST_MEM_INIT_START */
#define UIB_BIST_FULL_MEM	BIT(6)

/* UIBSSM_CMD_RESPONSE_DATA_SHORT for opcode UIB_BIST_MEM_INIT_START */
#define UIB_BIST_INITIATE_PASS	BIT(0)

/*
 * UIBSSM mailbox response outputs
 *
 * @cmd_resp_status: Command Interface status
 */
struct uib_mb_resp {
	u32 cmd_resp_status;
};

/*
 * UIB instance specific information
 *
 * @uib_csr_addr:	UIB instance CSR address
 * @cal_status:		UIB instance calibration status
 */
struct uib_instance {
	phys_addr_t uib_csr_addr;
	bool cal_status;
};

/*
 * Overall UIB instance(s) information
 *
 * @num_instance:	Number of instance(s) assigned to HPS
 * @overall_cal_status: Overall calibration status for all UIB instance(s)
 * @ecc_status:		ECC enable status (false = disabled, true = enabled)
 * @overall_size:	Total HBM memory size
 * @uib:		UIB instance specific information
 */
struct uib_info {
	u8 num_instance;
	bool overall_cal_status;
	bool ecc_status;
	phys_size_t overall_size;
	struct uib_instance uib[MAX_UIB_SUPPORTED];
};

/* Supported UIB function */
int uib_mb_req(phys_addr_t uib_csr_addr,
	       u32 usr_cmd_type, u32 usr_cmd_opcode,
	       u32 cmd_param_0, struct uib_mb_resp *resp);
int uib_cal_status(phys_addr_t addr);
void uib_init_mem_cal(struct uib_info *uib_ctrl);
void uib_trig_mem_cal(struct uib_info *uib_ctrl);
int uib_bist_mem_init_start(struct uib_info *uib_ctrl);

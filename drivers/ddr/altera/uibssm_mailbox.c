// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#include <hang.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include "uibssm_mailbox.h"

#define MAX_RETRIES 3

int uib_bist_mem_init_start(struct uib_info *uib_ctrl)
{
	struct uib_mb_resp usr_resp;
	bool bist_start, bist_success;
	u32 start;

	for (int i = 0; i < uib_ctrl->num_instance; i++) {
		bist_start = false;
		bist_success = false;

		/*
		 * Full memory initialization BIST performed on all UIB channels
		 * start memory initialization BIST on full memory address
		 */
		uib_mb_req(uib_ctrl->uib[i].uib_csr_addr,
			   UIB_CMD_TRIG_CONTROLLER_OP,
			   UIB_BIST_MEM_INIT_START,
			   UIB_BIST_FULL_MEM, &usr_resp);

		bist_start = UIBSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status) &
							    UIB_BIST_INITIATE_PASS;

		if (!bist_start) {
			printf("%s: Failed to initialize memory on UIB instance %d\n",
			       __func__, i);

			return -EINVAL;
		}

		/* Polling for the initiated memory initialization BIST status */
		start = get_timer(0);
		while (!bist_success) {
			udelay(1);

			/*
			 * cmd_param_0 is not used in BIST status request,
			 * hence set the value to 0
			 */
			uib_mb_req(uib_ctrl->uib[i].uib_csr_addr,
				   UIB_CMD_TRIG_CONTROLLER_OP,
				   UIB_BIST_MEM_INIT_STATUS,
				   0, &usr_resp);

			bist_success = UIBSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status) &
								      BIT(0);

			if (!bist_success && (get_timer(start) > TIMEOUT)) {
				printf("%s: Timeout initializing memory on UIB instance %d\n",
				       __func__, i);

				return -ETIMEDOUT;
			}
		}

		debug("%s: Memory initialized successfully on UIB instance %d\n", __func__, i);
	}

	return 0;
}

int uib_cal_status(phys_addr_t addr)
{
	int ret = 0;
	phys_addr_t status_addr = addr + UIB_R_INITSTS_OFFSET;

	/* Ensure calibration completed */
	ret = wait_for_bit_le32((const void *)status_addr, UIB_R_INITSTS_INITSTS_PASS, true,
				TIMEOUT, false);
	if (ret)
		printf("%s: HBM calibration UIB instance 0x%llx timeout\n", __func__, status_addr);

	return ret;
}

void uib_init_mem_cal(struct uib_info *uib_ctrl)
{
	int i, ret;

	if (!uib_ctrl->num_instance) {
		uib_ctrl->overall_cal_status = false;
	} else {
		uib_ctrl->overall_cal_status = true;

		/* Check initial calibration status for the assigned UIB */
		for (i = 0; i < uib_ctrl->num_instance; i++) {
			ret = uib_cal_status(uib_ctrl->uib[i].uib_csr_addr);
			if (ret) {
				uib_ctrl->uib[i].cal_status = false;
				uib_ctrl->overall_cal_status = false;

				printf("%s: Initial HBM calibration UIB_%d failed\n", __func__, i);
				break;
			}

			uib_ctrl->uib[i].cal_status = true;

			debug("%s: Initial HBM calibration UIB_%d succeed\n", __func__, i);
		}
	}
}

/* Trying 3 times re-calibration if initial calibration failed */
void uib_trig_mem_cal(struct uib_info *uib_ctrl)
{
	int i, j, cal_stat;

	if (!uib_ctrl->num_instance) {
		uib_ctrl->overall_cal_status = false;
	} else {
		uib_ctrl->overall_cal_status = true;

		for (i = 0; i < uib_ctrl->num_instance; i++) {
			uib_ctrl->uib[i].cal_status = false;

			/* Initiate Re-calibration */
			for (j = 0; j < MAX_RETRIES; j++) {
				clrsetbits_le32(uib_ctrl->uib[i].uib_csr_addr +
						UIB_R_INITCTL_OFFSET,
						UIB_R_INITCTL_INITTYPE_MASK |
						UIB_R_INITCTL_INITREQ_MASK,
						UIB_R_INITCTL_INITTYPE(UIB_RST_REQUEST_WITH_CAL) |
						UIB_R_INITCTL_INITREQ(1));

				cal_stat = uib_cal_status(uib_ctrl->uib[i].uib_csr_addr);
				if (cal_stat)
					continue;

				debug("%s: HBM re-calibration UIB_%d succeed\n", __func__, i);

				uib_ctrl->uib[i].cal_status = true;
				break;
			}

			if (!uib_ctrl->uib[i].cal_status) {
				uib_ctrl->overall_cal_status = false;

				printf("%s: HBM re-calibration UIB_%d failed\n", __func__, i);
				break;
			}
		}
	}
}

static void uib_mailbox_write_request(u32 data, u32 target_write_addr, phys_addr_t csr_addr)
{
	int ret;

	/*
	 * Read from chms0020 MBWRADDR_VALID and ensure its not set or
	 * wait till it get cleared by controller
	 */
	debug("%s: #1 Read MBWRADDR_VALID from UIB_R_MBWRCTL\n", __func__);
	ret = wait_for_bit_le32((const void *)csr_addr + UIB_R_MBWRCTL,
				UIB_R_MBWRCTL_MBWRADDR_VALID, false, TIMEOUT, false);
	if (ret) {
		printf("%s: TIMEOUT!!! MBWRADDR_VALID is not zero\n", __func__);

		hang();
	}

	/* Write <target write address> to chms0024 MBWRADDR */
	debug("%s: #2 Write 0x%x to UIB_R_MBWRADDR\n", __func__, target_write_addr);
	writel(target_write_addr, csr_addr + UIB_R_MBWRADDR);

	/*
	 * Write 1 to chms0020 MBWRADDR_VALID to indicate the address is now valid
	 * for FW to read
	 */
	debug("%s: #3 Write 1 to MBWRADDR_VALID for FW to read address\n", __func__);
	setbits_le32(csr_addr + UIB_R_MBWRCTL, UIB_R_MBWRCTL_MBWRADDR_VALID);

	/*
	 * Read from chms0020 MBWRDATA_VALID and ensure its not set or
	 * wait till it get cleared by controller
	 */
	debug("%s: #4 Read MBWRDATA_VALID from UIB_R_MBWRCTL\n", __func__);
	ret = wait_for_bit_le32((const void *)csr_addr + UIB_R_MBWRCTL,
				UIB_R_MBWRCTL_MBWRDATA_VALID, false, TIMEOUT, false);
	if (ret) {
		printf("%s: TIMEOUT!!! MBWRADDR_VALID is not zero\n", __func__);

		hang();
	}

	/*
	 * Read from chms0020 MBWRDATA_END and ensure its not set or
	 * wait till it get cleared by controller
	 */
	debug("%s: #5 Read R_MBWRCTL_MBWRDATA_END from UIB_R_MBWRCTL\n", __func__);
	ret = wait_for_bit_le32((const void *)csr_addr + UIB_R_MBWRCTL,
				UIB_R_MBWRCTL_MBWRDATA_END, false, TIMEOUT, false);
	if (ret) {
		printf("%s: TIMEOUT!!! MBWRDATA_END is not zero\n", __func__);

		hang();
	}

	/* Write <write data> to chms0028 MMR_MBWRDATA */
	debug("%s: #6 Write 0x%x to UIB_R_MBWRDATA\n", __func__, data);
	writel(data, csr_addr + UIB_R_MBWRDATA);

	/*
	 * Write 1 to chms0020 MBWRDATA_END to indicate if the <write data> is the last burst
	 * for FW to read for the <target write address>
	 */
	debug("%s: #7 Write 1 to MBWRDATA_END to inform FW this is last burst of data to read\n",
	      __func__);
	setbits_le32(csr_addr + UIB_R_MBWRCTL, UIB_R_MBWRCTL_MBWRDATA_END);

	/* Write 1 to chms0020 MBWRDATA_VALID to indicate the data is now valid for FW to read */
	debug("%s: #8 Write 1 to MBWRDATA_VALID for FW to read data\n", __func__);
	setbits_le32(csr_addr + UIB_R_MBWRCTL, UIB_R_MBWRCTL_MBWRDATA_VALID);
}

static u32 uib_mailbox_read_request(u32 target_read_addr, phys_addr_t csr_addr)
{
	int ret;
	u32 reg, rd_data = 0;

	/*
	 * Read from chms0030 MBRDADDR_VALID and ensure its not set or
	 * wait till it get cleared by controller
	 */
	debug("%s: #1 Read MBRDADDR_VALID from UIB_R_MBRDCTL\n", __func__);
	ret = wait_for_bit_le32((const void *)csr_addr + UIB_R_MBRDCTL,
				UIB_R_MBRDCTL_MBRDADDR_VALID, false, TIMEOUT, false);
	if (ret) {
		printf("%s: TIMEOUT!!! MBRDADDR_VALID is not zero\n", __func__);

		hang();
	}

	/* Write <target read address> to chms0034 MBRDADDR */
	debug("%s: #2 Write 0x%x to UIB_R_MBRDADDR\n", __func__, target_read_addr);
	writel(target_read_addr, csr_addr + UIB_R_MBRDADDR);

	/*
	 * Write 1 to chms0030 MBRDADDR_VALID to indicate the address is now valid
	 * for FW to read
	 */
	debug("%s: #3 Write 1 to MBRDADDR_VALID for FW to read address\n", __func__);
	setbits_le32(csr_addr + UIB_R_MBRDCTL, UIB_R_MBRDCTL_MBRDADDR_VALID);

	/*
	 * Continuously poll the chms0030 MBRDDATA_VALID. If MBRDDATA_VALID are set, read
	 * chms0038 MBRDDATA and chms0030 MBRDDATA_END to retrieve the <read data> and
	 * <end of read burst> status accordingly
	 */
	debug("%s: #4 Read MBRDDATA_VALID from UIB_R_MBRDCTL\n", __func__);
	ret = wait_for_bit_le32((const void *)csr_addr + UIB_R_MBRDCTL,
				UIB_R_MBRDCTL_MBRDDATA_VALID, true, TIMEOUT, false);
	if (ret) {
		printf("%s: TIMEOUT!!! MBRDDATA_VALID is zero\n", __func__);

		hang();
	}

	reg = readl(csr_addr + UIB_R_MBRRDATA);
	debug("%s: #5 Read data from UIB_R_MBRRDATA = 0x%x\n", __func__, reg);
	rd_data = reg;

	reg = readl(csr_addr + UIB_R_MBRDCTL);
	debug("%s: #6 Read end of read burst status from UIB_R_MBRDCTL = 0x%x\n", __func__, reg);

	/*
	 * Once done retrieving the data, write 1 to chms0030 MBRDDATA_VALID,
	 * chms0030 MBRDDATA_END to clear the register
	 */
	debug("%s: #7 Write 1 to MBRDDATA_VALID for FW to read address\n", __func__);
	setbits_le32(csr_addr + UIB_R_MBRDCTL, UIB_R_MBRDCTL_MBRDDATA_VALID |
			UIB_R_MBWRCTL_MBWRDATA_END);

	return rd_data;
}

int uib_mb_req(phys_addr_t uib_csr_addr, u32 usr_cmd_type, u32 usr_cmd_opcode,
	       u32 cmd_param_0, struct uib_mb_resp *resp)
{
	u32 cmd_req;

	/* Initialized zeros for responses */
	resp->cmd_resp_status = 0;

	/* Write CMD_REQ (CMD_TYPE and CMD_OPCODE) */
	cmd_req = FIELD_PREP(CMD_TYPE_MASK, usr_cmd_type) |
		FIELD_PREP(CMD_OPCODE_MASK, usr_cmd_opcode);
	uib_mailbox_write_request(cmd_req, 0, uib_csr_addr);

	debug("%s: Write 0x%x to UIBSSM_CMD_REQ_OFFSET 0x%llx\n", __func__, cmd_req, uib_csr_addr);

	/* Write CMD_PARAM_* */
	if (cmd_param_0)
		uib_mailbox_write_request(cmd_param_0, 0, uib_csr_addr);
	else
		debug("%s: cmd_param_0 is NULL\n", __func__);

	/* read CMD_RESPONSE_STATUS */
	resp->cmd_resp_status = uib_mailbox_read_request(0, uib_csr_addr);

	debug("%s: CMD_RESPONSE_STATUS 0x%llx: 0x%x\n", __func__,
	      uib_csr_addr, resp->cmd_resp_status);
	debug("%s: STATUS_CMD_RESPONSE_ERROR: 0x%lx\n", __func__,
	      UIBSSM_CMD_RESPONSE_ERROR(resp->cmd_resp_status));
	debug("%s: STATUS_GENERAL_ERROR: 0x%lx\n", __func__,
	      UIBSSM_GENERAL_ERROR(resp->cmd_resp_status));

	return 0;
}

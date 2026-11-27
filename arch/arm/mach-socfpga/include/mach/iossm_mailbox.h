/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 *
 */

#ifndef _IOSSM_MAILBOX_H_
#define _IOSSM_MAILBOX_H_

#define MAX_IO96B_SUPPORTED			2
#define MAX_MEM_INTERFACE_SUPPORTED			2
#define NUM_CMD_RESPONSE_DATA			3
#define NUM_CMD_PARAM				7

/* supported mailbox command type */
enum iossm_mailbox_cmd_type  {
	CMD_NOP,
	CMD_GET_SYS_INFO,
	CMD_GET_MEM_INFO,
	CMD_GET_MEM_CAL_INFO,
	CMD_TRIG_CONTROLLER_OP,
	CMD_TRIG_MEM_CAL_OP
};

/* supported mailbox command opcode */
enum iossm_mailbox_cmd_opcode  {
	ECC_ENABLE_SET = 0x0101,
	ECC_INTERRUPT_MASK = 0x0105,
	ECC_WRITEBACK_ENABLE = 0x0106,
	ECC_INJECT_ERROR = 0x0109,
	ECC_ERROR_QUEUE_CLEAR = 0x0110,
	ECC_SCRUB_MODE_0_START = 0x0202,
	ECC_SCRUB_MODE_1_START = 0x0203,
	BIST_STANDARD_MODE_START = 0x0301,
	BIST_MEM_INIT_START = 0x0303,
	BIST_SET_DATA_PATTERN_UPPER = 0x0305,
	BIST_SET_DATA_PATTERN_LOWER = 0x0306,
	TRIG_MEM_CAL = 0x000a
};

/*
 * IOSSM mailbox required information
 *
 * @num_mem_interface:	Number of memory interfaces instantiated
 * @ip_type:		IP type implemented on the IO96B
 * @ip_instance_id:	IP identifier for every IP instance implemented on the IO96B
 * @memory_size[2]:	Memory size for every IP instance implemented on the IO96B
 */
struct io96b_mb_ctrl {
	u32 num_mem_interface;
	u32 ip_type[2];
	u32 ip_id[2];
	phys_size_t memory_size[2];
};

/* CMD_REQ Register Definition */
#define CMD_TARGET_IP_TYPE_MASK		GENMASK(31, 29)
#define CMD_TARGET_IP_INSTANCE_ID_MASK	GENMASK(28, 24)
#define CMD_TYPE_MASK			GENMASK(23, 16)
#define CMD_OPCODE_MASK			GENMASK(15, 0)

/* Computes the Inline ECC data region size */
#define CALC_INLINE_ECC_HW_SIZE(size) (((size) * 7) / 8)

/*
 * IOSSM mailbox request
 * @ip_type:	    IP type for the specified memory interface
 * @ip_id:	    IP instance ID for the specified memory interface
 * @usr_cmd_type:   User desire IOSSM mailbox command type
 * @usr_cmd_opcode: User desire IOSSM mailbox command opcode
 * @cmd_param_*:    Parameters (if applicable) for the requested IOSSM mailbox command
 */
struct io96b_mb_req {
	u32 ip_type;
	u32 ip_id;
	u32 usr_cmd_type;
	u32 usr_cmd_opcode;
	u32 cmd_param[NUM_CMD_PARAM];
};

/*
 * IOSSM mailbox response outputs
 *
 * @cmd_resp_status: Command Interface status
 * @cmd_resp_data_*: More spaces for command response
 */
struct io96b_mb_resp {
	u32 cmd_resp_status;
	u32 cmd_resp_data[NUM_CMD_RESPONSE_DATA];
};

/*
 * IO96B instance specific information
 *
 * @io96b_csr_addr:	IO96B instance CSR address
 * @cal_status:		IO96B instance calibration status
 * @mb_ctrl:		IOSSM mailbox required information
 */
struct io96b_instance {
	phys_addr_t io96b_csr_addr;
	bool cal_status;
	struct io96b_mb_ctrl mb_ctrl;
};

/*
 * Overall IO96B instance(s) information
 *
 * @num_instance:	Number of instance(s) assigned to HPS
 * @overall_cal_status: Overall calibration status for all IO96B instance(s)
 * @ddr_type:		DDR memory type
 * @ecc_status:		ECC enable status (false = disabled, true = enabled)
 * @inline_ecc:		Inline ECC or Out of Band ECC (false = Out of Band ECC, true = Inline ECC)
 * @overall_size:	Total DDR memory size
 * @io96b[]:		IO96B instance specific information
 * @ckgen_lock:		IO96B GEN PLL lock (false = not locked, true = locked)
 * @num_port:		Number of IO96B port.
 * @io96b_pll:		Selected IO96B PLL. Example bit 0: EMIF0 PLL A selected,
 *			bit 1: EMIF0 PLL B selected, bit 2 - EMIF1 PLL A selected,
 *			bit 3: EMIF1 PLL B selected
 */
struct io96b_info {
	u8 num_instance;
	bool overall_cal_status;
	const char *ddr_type;
	bool ecc_status;
	bool inline_ecc;
	phys_size_t overall_size;
	struct io96b_instance io96b[MAX_IO96B_SUPPORTED];
	bool ckgen_lock;
	u8 num_port;
	u8 io96b_pll;
};

int io96b_mb_req(phys_addr_t io96b_csr_addr, struct io96b_mb_req req,
		 u32 resp_data_len, struct io96b_mb_resp *resp);

/* Supported IOSSM mailbox function */
void io96b_mb_init(struct io96b_info *io96b_ctrl);
int io96b_cal_status(phys_addr_t addr);
void init_mem_cal(struct io96b_info *io96b_ctrl);
int get_mem_technology(struct io96b_info *io96b_ctrl);
int get_mem_width_info(struct io96b_info *io96b_ctrl);
int ecc_enable_status(struct io96b_info *io96b_ctrl);
int bist_mem_init_start(struct io96b_info *io96b_ctrl);
bool ecc_interrupt_status(struct io96b_info *io96b_ctrl);

/**
 * io96b_csr_from_handoff_table() - fill IO96B CSR bases from a handoff table
 * @io96b_ctrl: IO96B info to update (@num_instance and @io96b[].io96b_csr_addr)
 * @handoff: SDRAM handoff word array already read from OCRAM
 * @handoff_len: number of words in @handoff (must be SOC64_HANDOFF_IO96B_LEN)
 *
 * Return: 0 on success, -EINVAL if arguments are invalid, -ENODEV if no
 * IO96B instance is enabled.
 */
int io96b_csr_from_handoff_table(struct io96b_info *io96b_ctrl,
				 const u32 *handoff, u32 handoff_len);

/**
 * io96b_csr_from_handoff() - fill IO96B CSR bases for the current phase
 * @io96b_ctrl: IO96B info to update (@num_instance and @io96b[].io96b_csr_addr)
 *
 * Return: 0 on success, negative errno from handoff or scratch decode.
 */
int io96b_csr_from_handoff(struct io96b_info *io96b_ctrl);

/**
 * io96b_ecc_inject() - inject an IO96B ECC error via the IOSSM mailbox
 * @io96b_ctrl: IO96B context with CSR bases and mailbox IP type/id filled
 * @dbe: true for double-bit error, false for single-bit error
 *
 * Targets instance 0 / interface 0 with one safe DDR poke address.
 *
 * Return: 0 on success, -EINVAL for bad arguments, or negative errno.
 */
int io96b_ecc_inject(struct io96b_info *io96b_ctrl, bool dbe);

/**
 * io96b_ecc_status_iossm() - drain the IO ECC queue
 * @io96b_ctrl: IO96B context; caller must have run io96b_mb_init()
 * @dbe: set true if a double-bit error was observed (always written)
 *
 * Return: 0 if the queue was drained and cleared, negative errno otherwise.
 * On clear failure after a DBE was observed, @dbe is still true so the
 * caller can prefer recovery over the errno.
 */
int io96b_ecc_status_iossm(struct io96b_info *io96b_ctrl, bool *dbe);

#if CONFIG_IS_ENABLED(CMD_SOCFPGA_DDR_ECC)
/**
 * io96b_ecc_status_sdm() - drain the SDM ECC queue
 * @dbe: set true if any returned entry is a DBE (always written)
 *
 * Return: 0 if the queue was drained, negative errno on mailbox or
 * response-format failure.
 */
int io96b_ecc_status_sdm(bool *dbe);
#endif

#endif /* _IOSSM_MAILBOX_H_ */

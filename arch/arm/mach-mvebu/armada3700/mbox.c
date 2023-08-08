/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 * https://spdx.org/licenses
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch-armada3700/mbox.h>
#include <asm/arch-armada3700/efuse.h>

enum a3700_boot_dev {
	AUTO		= 0,
	SPINOR		= 1,
	EMMCNORM	= 2,
	EMMCALT		= 3,
	SATA		= 4,
	SPINAND		= 5,
	UART		= 6,
	INVALID		= 7,
	VECTOR_DIV	= 8,
	VECTOR_XTAL	= 9,

	MAX_BOOT_DEVS
};

#define A3700_BOOT_DEV_NAMES	{"AUTO", "SPINOR", "EMMCNORM", "EMMCALT", \
			"SATA", "SPINAND", "UART", "", "", ""}

struct a3700_efuse_info {
	/* efuse ID */
	enum efuse_id		id;
	/* mailbox operation size bit/byte/word, etc. */
	enum mbox_opsize	mbopsz;
	/* efuse row or start row for multi-row values */
	uint32_t		row;
	/* number of write operations required for setting the field */
	uint32_t		numops;
	/* efuse bit offset within the row for bit size efuses */
	int32_t			bitoffs[4];
};

#define A3700_EFUSE_INFO	{ \
	{ EFUSE_ID_BOOT_DEVICE,       MB_OPSZ_BIT,   1,   4, { 48, 52, 56, 60 } }, \
	{ EFUSE_ID_KAK_DIGEST,        MB_OPSZ_256B,  8,   1, { 0, 0, 0, 0 } }, \
	{ EFUSE_ID_CSK_INDEX,         MB_OPSZ_DWORD, 3,   1, { 0, 0, 0, 0 } }, \
	{ EFUSE_ID_AES_KEY,           MB_OPSZ_256B,  26,  1, { 0, 0, 0, 0 } }, \
	{ EFUSE_ID_ENCRYPTION_EN,     MB_OPSZ_BIT,   0,   2, { 56, 60, 0, 0 } }, \
	{ EFUSE_ID_JTAG_DIGECT,       MB_OPSZ_256B,  16,  1, { 0, 0, 0, 0 } }, \
	{ EFUSE_ID_SEC_JTAG_DIS,      MB_OPSZ_BIT,   0,   1, { 24, 0, 0, 0 } }, \
	{ EFUSE_ID_SEC_JTAG_PERM_DIS, MB_OPSZ_BIT,   0,   1, { 28, 0, 0, 0 } }, \
	{ EFUSE_ID_AP_JTAG_DIS,       MB_OPSZ_BIT,   0,   1, { 16, 0, 0, 0 } }, \
	{ EFUSE_ID_AP_JTAG_PERM_DIS,  MB_OPSZ_BIT,   0,   1, { 20, 0, 0, 0 } }, \
	{ EFUSE_ID_SPI_NAND_CFG,      MB_OPSZ_DWORD, 6,   1, { 0, 0, 0, 0 } }, \
	{ EFUSE_ID_PIN,               MB_OPSZ_DWORD, 4,   1, { 0, 0, 0, 0 } }, \
	{ EFUSE_ID_TOKEN,             MB_OPSZ_DWORD, 5,   1, { 0, 0, 0, 0 } }, \
	{ EFUSE_ID_SPI_CS,            MB_OPSZ_BIT,   1,   2, { 40, 44, 0, 0 } }, \
	{ EFUSE_ID_EMMC_CLOCK,        MB_OPSZ_BIT,   1,   2, { 32, 36, 0, 0 } }, \
	{ EFUSE_ID_OPERATION_MODE,    MB_OPSZ_BIT,   1,   2, { 0, 4, 0, 0 } }, \
	{ EFUSE_ID_UART_DIS,          MB_OPSZ_BIT,   0,   1, { 32, 0, 0, 0 } }, \
	{ EFUSE_ID_UART_PERM_DIS,     MB_OPSZ_BIT,   0,   1, { 36, 0, 0, 0 } }, \
	{ EFUSE_ID_ESC_SEQ_DIS,       MB_OPSZ_BIT,   1,   1, { 20, 0, 0, 0 } }, \
	{ EFUSE_ID_GPIO_TOGGLE_DIS,   MB_OPSZ_BIT,   1,   1, { 16, 0, 0, 0 } }, \
	{ EFUSE_ID_LONG_KEY_EN,       MB_OPSZ_BIT,   1,   1, { 12, 0, 0, 0 } } \
}

/*****************************************************************************
 *	mbox_send
 *****************************************************************************/
int mbox_send(enum mbox_opsize opsz, enum mbox_op op, uint32_t row,
	      uint32_t offs, uint32_t *args)
{
	uint32_t	params[MBOX_MAX_ARGS];
	uint32_t	n, params_to_send;

	if (args == 0) {
		printf("%s: Invalid argument\n", __func__);
		return 1;
	}

	if (op != MB_OP_READ && op != MB_OP_WRITE) {
		printf("%s: Invalid operation\n", __func__);
		return 1;
	}

	memset(params, 0, MBOX_MAX_ARGS * sizeof(uint32_t));

	/* First parameter in the list describes eFuse row */
	params[0] = row;

	switch (opsz) {
	case MB_OPSZ_BIT:
	case MB_OPSZ_BYTE:
	case MB_OPSZ_WORD:
		params_to_send = 3;
		params[1] = offs;
		params[2] = args[0];
		break;
	case MB_OPSZ_DWORD:
		params_to_send = 3;
		params[1] = args[0];
		params[2] = args[1];
		break;
	case MB_OPSZ_256B:
		params_to_send = 9;
		memcpy(&params[1], args, 8 * sizeof(uint32_t));
		break;
	default:
		printf("%s: Invalid size\n", __func__);
		return 1;
	}


	/* First, fill all command arguments */
	for (n = 0; n < params_to_send; n++) {
		debug("=>MBOX WRITE PARAM[%d] = %08X\n", n, params[n]);
		writel(params[n], (long)MBOX_SEND_ARG_OFFS(n));
	}

	/* Writing command triggers mailbox dispatch and
	   intarrupt on secure CPU side */
	debug("=>MBOX WRITE CMD = %08X\n", MBOX_COMMAND(opsz, op));
	writel(MBOX_COMMAND(opsz, op), MBOX_SEND_CMD_OFFS);

	return 0;
}

/*****************************************************************************
 *	mbox_receive - BLOCKING
 *****************************************************************************/
int mbox_receive(enum mbox_status *stat, uint32_t *args,
		 uint32_t timeout_us)
{
	uint32_t n;
	uint32_t regval;

	if (args == 0) {
		*stat = MB_STAT_BAD_ARGUMENT;
		return 1;
	}

	/* Poll for secure CPU command completion */
	for (n = 0; n < timeout_us; n++) {
		regval = readl(MBOX_SEC_CPU_INT_STAT_REG);
		if (regval & MBOX_SEC_CPU_CMD_SET)
			break;
		mdelay(100);
	}

	if (n == timeout_us) {
		printf("%s: MB timeout\n", __func__);
		return 1;
	}

	/* Read comamnd status and arguments */
	for (n = 0; n < MBOX_MAX_ARGS; n++) {
		args[n] = readl((long)MBOX_RECEIVE_ARG_OFFS(n));
		debug("<=MBOX READ ARG[%d] = %08X\n", n, args[n]);
	}

	*stat = readl(MBOX_RECEIVE_STAT_OFFS);
	debug("<=MBOX READ STATUS = %08X\n", *stat);

	/* Reset host interrupt */
	regval = readl(MBOX_HOST_INT_RESET) | MBOX_SEC_CPU_CMD_COMPLETE;
	writel(regval, MBOX_HOST_INT_RESET);

	return 0;
}

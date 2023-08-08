/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 * https://spdx.org/licenses
 */

#include <common.h>
#include <command.h>
#include <vsprintf.h>
#include <malloc.h>
#include <errno.h>
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

static struct a3700_efuse_info efuse_info[EFUSE_ID_MAX] = A3700_EFUSE_INFO;
static char *bdnames[] = A3700_BOOT_DEV_NAMES;

/******************************************************************************
 *	efuse_id_valid
 ******************************************************************************/
int efuse_id_valid(enum efuse_id fid)
{
	if (fid < EFUSE_ID_MAX)
		return 1;
	else
		return 0;
}

/******************************************************************************
 *	efuse_write
 ******************************************************************************/
int efuse_write(enum efuse_id fid, const char *value)
{
	uint32_t		args[MBOX_MAX_ARGS];
	enum a3700_boot_dev	bdev;
	uint32_t		info_idx, numval = 0;
	uint32_t		numwords, ascii_words;
	char			buf[9];
	int			status, n;
	enum mbox_status	cmd_stat;
	uint64_t		fullrow;
	char			*str, *strcopy, *substr;
	uint32_t		val[4];

	/* Find efuse info with length and offset */
	for (info_idx = 0; info_idx < EFUSE_ID_MAX; info_idx++) {
		if (efuse_info[info_idx].id == fid)
			break;
	}

	if (info_idx == EFUSE_ID_MAX) {
		printf("%s: Unsupported eFuse ID\n", __func__);
		return 1;
	}

	/* Each word is coded by 8 ASCII characters, one character per nibble */
	numwords = strlen(value) >> 3;
	memset(buf, 0, 9);
	memset(args, 0, MBOX_MAX_ARGS * sizeof(uint32_t));

	/* parse input parameter */
	switch (fid) {
	case EFUSE_ID_BOOT_DEVICE:
		for (bdev = 0; bdev < MAX_BOOT_DEVS; bdev++) {
			if (strcmp(value, bdnames[bdev]) == 0)
				break;
		}
		if (bdev == MAX_BOOT_DEVS) {
			printf("%s: Unsupported Boot Device\n", __func__);
			return 1;
		}
		numval = bdev;
		break;

	case EFUSE_ID_KAK_DIGEST:
	case EFUSE_ID_AES_KEY:
	case EFUSE_ID_JTAG_DIGECT:
	case EFUSE_ID_PIN:
	case EFUSE_ID_TOKEN:
		/* TODO - add support for 512b keys if needed */
		if (efuse_info[info_idx].mbopsz == MB_OPSZ_256B) {
			ascii_words = 8;
		} else if (efuse_info[info_idx].mbopsz == MB_OPSZ_DWORD) {
			ascii_words = 2;
		} else {
			printf("Bad operation size for this key!\n");
			return 1;
		}
		if (numwords != ascii_words) {
			printf("%s: Unsupported digest length (%d) - expected %d bytes\n",
			       __func__, numwords << 3, ascii_words << 3);
			return 1;
		}
		/* Convert ASCII representation to WORD integer arguments
		   The eFuse has to have LSB part in lower rows */
		for (n = 0; n < numwords; n++) {
			/* 8 ASCII characters in WORD argument */
			memcpy(buf, &value[n * 8], 8);
			args[numwords - n - 1] = simple_strtoul(buf, 0, 16);
		}
		break;

	case EFUSE_ID_CSK_INDEX:
		numval = simple_strtoul(value, 0, 10);
		if (numval > 15) {
			printf("%s: Invalid CSK index %d, expected [0..15]\n",
			       __func__, numval);
			return 1;
		}
		/* The CSK key validity is stored in a single efuse row,
		   but the bit offset depends on CSK index.
		   Each index is selected by majority vote out of 3 bits.
		   In order to set single index, 2 or 3 physical bits should
		   be programmed. Gap between index start positions is 4 bits.
		 */
		fullrow = 0x7 << (4 * numval);
		args[0] = fullrow & 0xFFFFFFFF;
		args[1] = (fullrow >> 32) & 0xFFFFFFFF;
		break;

	case EFUSE_ID_ENCRYPTION_EN:
		numval = simple_strtoul(value, 0, 16);
		if ((numval != 0x11) && (numval != 0x10) && (numval != 0x1)) {
			printf("%s: Invalid value %x, expected 1, 10 or 11\n",
			       __func__, numval);
			return 1;
		}
		numval = (numval & 1) | ((numval & 0x10) >> 3);
		break;

	case EFUSE_ID_SEC_JTAG_DIS:
	case EFUSE_ID_SEC_JTAG_PERM_DIS:
	case EFUSE_ID_AP_JTAG_DIS:
	case EFUSE_ID_AP_JTAG_PERM_DIS:
	case EFUSE_ID_UART_DIS:
	case EFUSE_ID_UART_PERM_DIS:
	case EFUSE_ID_ESC_SEQ_DIS:
	case EFUSE_ID_GPIO_TOGGLE_DIS:
	case EFUSE_ID_LONG_KEY_EN:
	case EFUSE_ID_EMMC_CLOCK:
		numval = simple_strtoul(value, 0, 10);
		if (numval != 1) {
			printf("%s: Invalid value %d, expected 1\n",
			       __func__, numval);
			return 1;
		}
		break;

	case EFUSE_ID_OPERATION_MODE:
	case EFUSE_ID_SPI_CS:
		numval = simple_strtoul(value, 0, 10);
		if (numval > 3) {
			printf("%s: Invalid value %d, expected [0..3]\n",
			       __func__, numval);
			return 1;
		}
		break;

	case EFUSE_ID_SPI_NAND_CFG:
		strcopy = strdup(value);
		if (strcopy == NULL) {
			printf("%s: Unable to duplicate parameters list!\n",
			       __func__);
			return 1;
		}
		str = strcopy;
		for (n = 0; n < 4; n++) {
			if (str == NULL) {
				printf("%s: Invalid parameters list, expected PZ.BP.SO.SN\n",
				       __func__);
				return 1;
			}
			substr = strsep(&str, ".");
			val[n] = simple_strtoul(substr, 0, 10);
			if (((n == 0) && (val[n] > 0xFFFF)) ||
			    ((n > 0) && (val[n] > 0xFF))) {
				printf("%s: Invalid value[%d] %d, expected %s\n",
				       __func__, n, val[n], n == 0 ?
				       "0 - 65535" : "0 - 255");
				return 1;
			}
		}
		free(strcopy);
		/* PZ - bit[15:0], PB - bit[23:16] */
		args[0] = (val[1] << 16) | val[0];
		/* SO - bit[39:32], SN - bit[55:48] */
		args[1] = (val[3] << 16) | val[2];
		break;

	default:
		printf("%s: This eFuse ID write function is not implemented\n",
		       __func__);
		return 1;
	}

	/* Send command to the remote CPU */
	for (n = 0; n < efuse_info[info_idx].numops; n++) {
		/* for bit fields write 1 bit a time */
		if (efuse_info[info_idx].mbopsz == MB_OPSZ_BIT)
			args[0] = (numval >> n) & 1;

		status = mbox_send(efuse_info[info_idx].mbopsz,
				   MB_OP_WRITE,
				   efuse_info[info_idx].row,
				   efuse_info[info_idx].bitoffs[n],
				   args);

		if (status != 0) {
			printf("%s: Failed to dispatch command to remote CPU (n=%d)\n",
			       __func__, n);
			break;
		}

		/* Ensure the command execution ended on remote CPU */
		cmd_stat = MB_STAT_SUCCESS;
		status = mbox_receive(&cmd_stat, args, MBOX_CMD_TIMEOUT);
		if (status != 0 || cmd_stat != MB_STAT_SUCCESS) {
			printf("%s: Remote command execution failed (n=%d). Error local=%d, remote=%d\n",
			       __func__, n, status, cmd_stat);
			break;
		}
	}

	return status;
}

/******************************************************************************
 *	efuse_read
 ******************************************************************************/
int efuse_read(enum efuse_id fid, char *value)
{
	uint32_t		args[MBOX_MAX_ARGS];
	uint32_t		info_idx, n;
	uint32_t		count;
	int			status;
	enum mbox_status	cmd_stat;
	uint32_t		numval = 0;
	uint32_t		page_sz, page_block, spare_offs, spare_page;
	uint32_t		numwords;
	uint64_t		fullrow;

	/* Find efuse info with length and offset */
	for (info_idx = 0; info_idx < EFUSE_ID_MAX; info_idx++) {
		if (efuse_info[info_idx].id == fid)
			break;
	}
	if (info_idx == EFUSE_ID_MAX) {
		printf("%s: Unsupported eFuse ID\n", __func__);
		return 1;
	}

	memset(args, 0, MBOX_MAX_ARGS * sizeof(uint32_t));

	/* Send command to the remote CPU */
	for (n = 0; n < efuse_info[info_idx].numops; n++) {
		status = mbox_send(efuse_info[info_idx].mbopsz,
				   MB_OP_READ,
				   efuse_info[info_idx].row,
				   efuse_info[info_idx].bitoffs[n],
				   args);

		if (status != 0) {
			printf("%s: Failed to dispatch command to remote CPU (n=%d)\n",
			       __func__, n);
			return status;
		}

		/*
		 * Ensure the command execution ended
		 * on remote CPU and get the result
		 */
		cmd_stat = MB_STAT_SUCCESS;
		status = mbox_receive(&cmd_stat, args, MBOX_CMD_TIMEOUT);
		if (status != 0) {
			printf("%s: Failed locally (n=%d). Error=%d\n",
			       __func__, n, status);
			return status;
		} else if (cmd_stat != MB_STAT_SUCCESS) {
			printf("%s: Failed on remote (n=%d). Error=%d\n",
			       __func__, n, cmd_stat);
			return 1;
		}

		/* for bit fields collect 1 bit a time */
		if (efuse_info[info_idx].mbopsz == MB_OPSZ_BIT)
			numval |= (args[0] & 1) << n;
	}

	/* format the output value */
	switch (fid) {
	case EFUSE_ID_BOOT_DEVICE:
		if (numval >= MAX_BOOT_DEVS || numval == INVALID)
			sprintf(value, "INVALID VALUE (%d)", numval);
		else
			sprintf(value, "%s (%d)", bdnames[numval], numval);
		break;

	case EFUSE_ID_KAK_DIGEST:
	case EFUSE_ID_AES_KEY:
	case EFUSE_ID_JTAG_DIGECT:
	case EFUSE_ID_PIN:
	case EFUSE_ID_TOKEN:
		/* TODO - add support for 512b keys if needed */
		if (efuse_info[info_idx].mbopsz == MB_OPSZ_256B) {
			numwords = 8;
		} else if (efuse_info[info_idx].mbopsz == MB_OPSZ_DWORD) {
			numwords = 2;
		} else {
			printf("Bad operation size for this key!\n");
			return 1;
		}
		/* The eFuse has LSB part in lower rows */
		for (n = 0; n < numwords; n++)
			sprintf(value + n * 8, "%08X", args[numwords - n - 1]);
		break;

	case EFUSE_ID_CSK_INDEX:
		/* show all valid CSK IDs */
		count = 0;
		fullrow = args[1];	/* MSB */
		fullrow <<= 32;
		fullrow |= args[0];	/* LSB */
		for (n = 0; n < 15; n++) {
			/* Each CSK validity is a 3 bit majority vote
			   The distance between fileds is 4 bits */
			numval = (fullrow >> (n * 4)) & 0x7;
			/* Two or more bits set - CSK is valid */
			if (numval > 4 || numval == 3)
				count += sprintf(value + count, "%d ", n);
		}
		if (count == 0)
			sprintf(value, "NONE");
		break;

	case EFUSE_ID_ENCRYPTION_EN:
		sprintf(value, "%01d%01d", (numval >> 1) & 1, numval & 1);
		break;

	case EFUSE_ID_LONG_KEY_EN:
		sprintf(value, "%s (%d)", numval == 0 ?
			"DISABLED" : "ENABLED", numval);
		break;

	case EFUSE_ID_SEC_JTAG_DIS:
	case EFUSE_ID_SEC_JTAG_PERM_DIS:
	case EFUSE_ID_AP_JTAG_DIS:
	case EFUSE_ID_AP_JTAG_PERM_DIS:
	case EFUSE_ID_UART_DIS:
	case EFUSE_ID_UART_PERM_DIS:
	case EFUSE_ID_ESC_SEQ_DIS:
	case EFUSE_ID_GPIO_TOGGLE_DIS:
		sprintf(value, "%s (%d)", numval == 1 ?
			"DISABLED" : "ENABLED", numval);
		break;

	case EFUSE_ID_OPERATION_MODE:
	case EFUSE_ID_SPI_CS:
		sprintf(value, "%d", numval);
		break;

	case EFUSE_ID_EMMC_CLOCK:
		sprintf(value, "%s (%d)", numval == 0 ?
			"12.5MHz" : "50MHz", numval);
		break;

	case EFUSE_ID_SPI_NAND_CFG:
		page_sz = args[0] & 0xFFFF;		/* bit[15:0] */
		page_block = (args[0] >> 16) & 0xFF;	/* bit[23:16] */
		/* bit[39:32], value 1 means byte 0 */
		spare_offs = args[1] & 0xFF;
		/* bit[55:48], value 1 means page 0 */
		spare_page = (args[1] >> 16) & 0xFF;
		sprintf(value, "%04d.%02d.%02d.%02d", page_sz,
			page_block, spare_offs, spare_page);
		break;

	default:
		sprintf(value, "NOT IMPLEMENTED");
	}

	return 0;
}

/******************************************************************************
 *	efuse_raw_dump
 ******************************************************************************/
void efuse_raw_dump(void)
{
	printf("Raw eFuse dump is not supported on this platform\n");
	return;
}


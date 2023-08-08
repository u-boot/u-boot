/*
* ***************************************************************************
* Copyright (C) 2017 Marvell International Ltd.
* ***************************************************************************
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* Neither the name of Marvell nor the names of its contributors may be used
* to endorse or promote products derived from this software without specific
* prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
***************************************************************************
*/
#ifndef _A3700_EFUSE_H_
#define _A3700_EFUSE_H_

#define MVEBU_EFUSE_256B_ASCII_LEN	(64)
	/* 256 bit key ASCII representation length */
#define MVEBU_EFUSE_512B_ASCII_LEN	(128)
	/* 256 bit key ASCII representation length */
#define MVEBU_EFUSE_MAX_STRLEN		(MVEBU_EFUSE_512B_ASCII_LEN + 1)
	/* MAX key length in ASCII + \0 */

enum efuse_id {
	EFUSE_ID_BOOT_DEVICE		= 0,
	/* Platform boot device in trusted boot mode */
	EFUSE_ID_KAK_DIGEST		= 1,
	/* KAK key digest (OEM key hash) */
	EFUSE_ID_CSK_INDEX		= 2,
	/* CSK key valid index (0-15) */
	EFUSE_ID_AES_KEY		= 3,
	/* Symmetric key / RKEK (write only) */
	EFUSE_ID_ENCRYPTION_EN		= 4,
	/* Enable boot image encryption */
	EFUSE_ID_JTAG_DIGECT		= 5,
	/* JTAG key digest */
	EFUSE_ID_SEC_JTAG_DIS		= 6,
	/* Secure CPU JTAG disable */
	EFUSE_ID_SEC_JTAG_PERM_DIS	= 7,
	/* Secure CPU permanent JTAG disable */
	EFUSE_ID_AP_JTAG_DIS		= 8,
	/* Application CPU JTAG disable */
	EFUSE_ID_AP_JTAG_PERM_DIS	= 9,
	/* Application CPU permanent JTAG disable */
	EFUSE_ID_SPI_NAND_CFG		= 10,
	/* SPI NAND configuration */
	EFUSE_ID_PIN			= 11,
	/* PIN */
	EFUSE_ID_TOKEN			= 12,
	/* Token */
	EFUSE_ID_SPI_CS			= 13,
	/* SPI chip select (0-4) */
	EFUSE_ID_EMMC_CLOCK		= 14,
	/* EMMC boot clock */
	EFUSE_ID_OPERATION_MODE		= 15,
	/* Operation mode trusted/untrusted/etc. */
	EFUSE_ID_UART_DIS		= 16,
	/* UART boot disable */
	EFUSE_ID_UART_PERM_DIS		= 17,
	/* UART boot permanent disable */
	EFUSE_ID_ESC_SEQ_DIS		= 18,
	/* UART escape sequence disable */
	EFUSE_ID_GPIO_TOGGLE_DIS	= 19,
	/* GPIO toggle disable */
	EFUSE_ID_LONG_KEY_EN		= 20,
	/* Long key enable */
	EFUSE_ID_MAX
};

struct efuse_info {
	char *name;
	char *note;
};

#define MVEBU_EFUSE_INFO	{ \
		{"BOOT_DEVICE", "SPINOR, SPINAND, EMMCNORM, EMMCALT, SATA, UART, AUTO"}, \
		{"KAK_DIGEST", "SHA-256 KAK key digest in HEX format"}, \
		{"CSK_INDEX", "CSK index in range 0 to 15 in DEC format"}, \
		{"AES256_KEY", "AES-256 symmetric encryption key in HEX format"}, \
		{"ENCRYPTION", "Enabe/Disable image encryption (2 bits, binary value):\n" \
				"\t\t\t00 - Encryption is disabled\n" \
				"\t\t\t01 - Encryption is enabled for recovery type images for eMMC only\n" \
				"\t\t\t10 - Encryption is enabled for primary type images\n" \
				"\t\t\t11 - Encryption is enabled for primary and recovery type images"}, \
		{"JTAG_DIGEST", "JTAG KAK key digest in HEX format"}, \
		{"SEC_JTAG_DIS", "Enabe/Disable secure JTAG - 0 or 1"}, \
		{"SEC_JTAG_PR_DIS", "Enabe/Disable secure JTAG permanently - 0 or 1"}, \
		{"AP_JTAG_DIS", "Enabe/Disable application CPU JTAG - 0 or 1"}, \
		{"AP_JTAG_PR_DIS", "Enabe/Disable application CPU JTAG permanently - 0 or 1"}, \
		{"SPI_NAND_CFG", "SPI NAND parameters in format PZ.BP.SO.SN, where (all decimal numbers):\n" \
				"\t\t\tPZ - Page Size (for instance 2048)\n" \
				"\t\t\tBP - Number of Pages per Block (for instance 64)\n" \
				"\t\t\tSO - Spare area byte Offset\n" \
				"\t\t\tSN - Spare area page Number"}, \
		{"PIN_CODE", "64-bit pin code in HEX format"}, \
		{"TOKEN", "\t64-bit token in HEX format"}, \
		{"SPI_CS", "\tSPI chip select 0 to 3"}, \
		{"EMMC_CLOCK", "EMMC clock - 0 - 12.5MHz, 1 - 50MHz"}, \
		{"OPER_MODE", "Operation mode in range of 0 to 3, where:\n" \
				"\t\t\t0 - Non-trusted BootROM, unprogrammed\n" \
				"\t\t\t1 - Non-trusted boot, no security check on the boot device content\n" \
				"\t\t\t2 - Trusted boot, security check is performed on the boot device content\n" \
				"\t\t\t3 - Tamper state; BootROM does not boot device"}, \
		{"UART_DIS", "Enabe/Disable UART port - 0 or 1"}, \
		{"UART_PR_DIS", "Enabe/Disable UART port permanently - 0 or 1"}, \
		{"ESC_SEQ_DIS", "Enabe/Disable Escape sequence in trusted boot mode - 0 or 1"}, \
		{"GPIO_TOGGLE_DIS", "Enabe/Disable GPIO pin 11 and 12 toogle - 0 or 1"}, \
		{"LONG_KEY_EN", "Enabe/Disable long key (512b) support  - 0 or 1"}, \
		{"INVALID", "Invalid ID"} \
	}

int efuse_id_valid(enum efuse_id fid);
int efuse_write(enum efuse_id fid, const char *value);
int efuse_read(enum efuse_id fid, char *value);
void efuse_raw_dump(void);

#endif /* _A3700_EFUSE_H_ */

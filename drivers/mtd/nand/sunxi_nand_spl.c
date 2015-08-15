/*
 * Copyright (c) 2014-2015, Antmicro Ltd <www.antmicro.com>
 * Copyright (c) 2015, AW-SOM Technologies <www.aw-som.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <nand.h>

/* registers */
#define NFC_CTL                    0x00000000
#define NFC_ST                     0x00000004
#define NFC_INT                    0x00000008
#define NFC_TIMING_CTL             0x0000000C
#define NFC_TIMING_CFG             0x00000010
#define NFC_ADDR_LOW               0x00000014
#define NFC_ADDR_HIGH              0x00000018
#define NFC_SECTOR_NUM             0x0000001C
#define NFC_CNT                    0x00000020
#define NFC_CMD                    0x00000024
#define NFC_RCMD_SET               0x00000028
#define NFC_WCMD_SET               0x0000002C
#define NFC_IO_DATA                0x00000030
#define NFC_ECC_CTL                0x00000034
#define NFC_ECC_ST                 0x00000038
#define NFC_DEBUG                  0x0000003C
#define NFC_ECC_CNT0               0x00000040
#define NFC_ECC_CNT1               0x00000044
#define NFC_ECC_CNT2               0x00000048
#define NFC_ECC_CNT3               0x0000004C
#define NFC_USER_DATA_BASE         0x00000050
#define NFC_EFNAND_STATUS          0x00000090
#define NFC_SPARE_AREA             0x000000A0
#define NFC_PATTERN_ID             0x000000A4
#define NFC_RAM0_BASE              0x00000400
#define NFC_RAM1_BASE              0x00000800

#define NFC_CTL_EN                 (1 << 0)
#define NFC_CTL_RESET              (1 << 1)
#define NFC_CTL_RAM_METHOD         (1 << 14)


#define NFC_ECC_EN                 (1 << 0)
#define NFC_ECC_PIPELINE           (1 << 3)
#define NFC_ECC_EXCEPTION          (1 << 4)
#define NFC_ECC_BLOCK_SIZE         (1 << 5)
#define NFC_ECC_RANDOM_EN          (1 << 9)
#define NFC_ECC_RANDOM_DIRECTION   (1 << 10)


#define NFC_ADDR_NUM_OFFSET        16
#define NFC_SEND_ADR               (1 << 19)
#define NFC_ACCESS_DIR             (1 << 20)
#define NFC_DATA_TRANS             (1 << 21)
#define NFC_SEND_CMD1              (1 << 22)
#define NFC_WAIT_FLAG              (1 << 23)
#define NFC_SEND_CMD2              (1 << 24)
#define NFC_SEQ                    (1 << 25)
#define NFC_DATA_SWAP_METHOD       (1 << 26)
#define NFC_ROW_AUTO_INC           (1 << 27)
#define NFC_SEND_CMD3              (1 << 28)
#define NFC_SEND_CMD4              (1 << 29)

#define NFC_CMD_INT_FLAG           (1 << 1)

#define NFC_READ_CMD_OFFSET         0
#define NFC_RANDOM_READ_CMD0_OFFSET 8
#define NFC_RANDOM_READ_CMD1_OFFSET 16

#define NFC_CMD_RNDOUTSTART        0xE0
#define NFC_CMD_RNDOUT             0x05
#define NFC_CMD_READSTART          0x30


#define NFC_PAGE_CMD               (2 << 30)

#define SUNXI_DMA_CFG_REG0              0x300
#define SUNXI_DMA_SRC_START_ADDR_REG0   0x304
#define SUNXI_DMA_DEST_START_ADDRR_REG0 0x308
#define SUNXI_DMA_DDMA_BC_REG0          0x30C
#define SUNXI_DMA_DDMA_PARA_REG0        0x318

#define SUNXI_DMA_DDMA_CFG_REG_LOADING  (1 << 31)
#define SUNXI_DMA_DDMA_CFG_REG_DMA_DEST_DATA_WIDTH_32 (2 << 25)
#define SUNXI_DMA_DDMA_CFG_REG_DDMA_DST_DRQ_TYPE_DRAM (1 << 16)
#define SUNXI_DMA_DDMA_CFG_REG_DMA_SRC_DATA_WIDTH_32 (2 << 9)
#define SUNXI_DMA_DDMA_CFG_REG_DMA_SRC_ADDR_MODE_IO (1 << 5)
#define SUNXI_DMA_DDMA_CFG_REG_DDMA_SRC_DRQ_TYPE_NFC (3 << 0)

#define SUNXI_DMA_DDMA_PARA_REG_SRC_WAIT_CYC (0x0F << 0)
#define SUNXI_DMA_DDMA_PARA_REG_SRC_BLK_SIZE (0x7F << 8)

/* minimal "boot0" style NAND support for Allwinner A20 */

/* random seed used by linux */
const uint16_t random_seed[128] = {
	0x2b75, 0x0bd0, 0x5ca3, 0x62d1, 0x1c93, 0x07e9, 0x2162, 0x3a72,
	0x0d67, 0x67f9, 0x1be7, 0x077d, 0x032f, 0x0dac, 0x2716, 0x2436,
	0x7922, 0x1510, 0x3860, 0x5287, 0x480f, 0x4252, 0x1789, 0x5a2d,
	0x2a49, 0x5e10, 0x437f, 0x4b4e, 0x2f45, 0x216e, 0x5cb7, 0x7130,
	0x2a3f, 0x60e4, 0x4dc9, 0x0ef0, 0x0f52, 0x1bb9, 0x6211, 0x7a56,
	0x226d, 0x4ea7, 0x6f36, 0x3692, 0x38bf, 0x0c62, 0x05eb, 0x4c55,
	0x60f4, 0x728c, 0x3b6f, 0x2037, 0x7f69, 0x0936, 0x651a, 0x4ceb,
	0x6218, 0x79f3, 0x383f, 0x18d9, 0x4f05, 0x5c82, 0x2912, 0x6f17,
	0x6856, 0x5938, 0x1007, 0x61ab, 0x3e7f, 0x57c2, 0x542f, 0x4f62,
	0x7454, 0x2eac, 0x7739, 0x42d4, 0x2f90, 0x435a, 0x2e52, 0x2064,
	0x637c, 0x66ad, 0x2c90, 0x0bad, 0x759c, 0x0029, 0x0986, 0x7126,
	0x1ca7, 0x1605, 0x386a, 0x27f5, 0x1380, 0x6d75, 0x24c3, 0x0f8e,
	0x2b7a, 0x1418, 0x1fd1, 0x7dc1, 0x2d8e, 0x43af, 0x2267, 0x7da3,
	0x4e3d, 0x1338, 0x50db, 0x454d, 0x764d, 0x40a3, 0x42e6, 0x262b,
	0x2d2e, 0x1aea, 0x2e17, 0x173d, 0x3a6e, 0x71bf, 0x25f9, 0x0a5d,
	0x7c57, 0x0fbe, 0x46ce, 0x4939, 0x6b17, 0x37bb, 0x3e91, 0x76db,
};

/* random seed used for syndrome calls */
const uint16_t random_seed_syndrome = 0x4a80;

#define MAX_RETRIES 10

static int check_value_inner(int offset, int expected_bits,
				int max_number_of_retries, int negation)
{
	int retries = 0;
	do {
		int val = readl(offset) & expected_bits;
		if (negation ? !val : val)
			return 1;
		mdelay(1);
		retries++;
	} while (retries < max_number_of_retries);

	return 0;
}

static inline int check_value(int offset, int expected_bits,
				int max_number_of_retries)
{
	return check_value_inner(offset, expected_bits,
					max_number_of_retries, 0);
}

static inline int check_value_negated(int offset, int unexpected_bits,
					int max_number_of_retries)
{
	return check_value_inner(offset, unexpected_bits,
					max_number_of_retries, 1);
}

void nand_init(void)
{
	uint32_t val;

	val = readl(SUNXI_NFC_BASE + NFC_CTL);
	/* enable and reset CTL */
	writel(val | NFC_CTL_EN | NFC_CTL_RESET,
	       SUNXI_NFC_BASE + NFC_CTL);

	if (!check_value_negated(SUNXI_NFC_BASE + NFC_CTL,
				 NFC_CTL_RESET, MAX_RETRIES)) {
		printf("Couldn't initialize nand\n");
	}

	/* reset NAND */
	writel(NFC_SEND_CMD1 | NFC_WAIT_FLAG | NAND_CMD_RESET,
	       SUNXI_NFC_BASE + NFC_CMD);

	if (!check_value(SUNXI_NFC_BASE + NFC_ST, NFC_CMD_INT_FLAG,
			 MAX_RETRIES)) {
		printf("Error timeout waiting for nand reset\n");
		return;
	}
}

static void nand_read_page(unsigned int real_addr, dma_addr_t dst,
			   int syndrome, uint32_t *ecc_errors)
{
	uint32_t val;
	int ecc_off = 0;
	uint16_t ecc_mode = 0;
	uint16_t rand_seed;
	uint32_t page;
	uint16_t column;
	uint32_t oob_offset;

	switch (CONFIG_NAND_SUNXI_SPL_ECC_STRENGTH) {
	case 16:
		ecc_mode = 0;
		ecc_off = 0x20;
		break;
	case 24:
		ecc_mode = 1;
		ecc_off = 0x2e;
		break;
	case 28:
		ecc_mode = 2;
		ecc_off = 0x32;
		break;
	case 32:
		ecc_mode = 3;
		ecc_off = 0x3c;
		break;
	case 40:
		ecc_mode = 4;
		ecc_off = 0x4a;
		break;
	case 48:
		ecc_mode = 4;
		ecc_off = 0x52;
		break;
	case 56:
		ecc_mode = 4;
		ecc_off = 0x60;
		break;
	case 60:
		ecc_mode = 4;
		ecc_off = 0x0;
		break;
	case 64:
		ecc_mode = 4;
		ecc_off = 0x0;
		break;
	default:
		ecc_mode = 0;
		ecc_off = 0;
	}

	if (ecc_off == 0) {
		printf("Unsupported ECC strength (%d)!\n",
		       CONFIG_NAND_SUNXI_SPL_ECC_STRENGTH);
		return;
	}

	page = real_addr / CONFIG_NAND_SUNXI_SPL_PAGE_SIZE;
	column = real_addr % CONFIG_NAND_SUNXI_SPL_PAGE_SIZE;

	if (syndrome)
		column += (column / CONFIG_NAND_SUNXI_SPL_ECC_PAGE_SIZE)
			* ecc_off;

	/* clear ecc status */
	writel(0, SUNXI_NFC_BASE + NFC_ECC_ST);

	/* Choose correct seed */
	if (syndrome)
		rand_seed = random_seed_syndrome;
	else
		rand_seed = random_seed[page % 128];

	writel((rand_seed << 16) | NFC_ECC_RANDOM_EN | NFC_ECC_EN
		| NFC_ECC_PIPELINE | (ecc_mode << 12),
		SUNXI_NFC_BASE + NFC_ECC_CTL);

	val = readl(SUNXI_NFC_BASE + NFC_CTL);
	writel(val | NFC_CTL_RAM_METHOD, SUNXI_NFC_BASE + NFC_CTL);

	if (syndrome) {
		writel(CONFIG_NAND_SUNXI_SPL_ECC_PAGE_SIZE,
		       SUNXI_NFC_BASE + NFC_SPARE_AREA);
	} else {
		oob_offset = CONFIG_NAND_SUNXI_SPL_PAGE_SIZE
			+ (column / CONFIG_NAND_SUNXI_SPL_ECC_PAGE_SIZE)
			* ecc_off;
		writel(oob_offset, SUNXI_NFC_BASE + NFC_SPARE_AREA);
	}

	/* SUNXI_DMA */
	writel(0x0, SUNXI_DMA_BASE + SUNXI_DMA_CFG_REG0); /* clr dma cmd */
	/* read from REG_IO_DATA */
	writel(SUNXI_NFC_BASE + NFC_IO_DATA,
	       SUNXI_DMA_BASE + SUNXI_DMA_SRC_START_ADDR_REG0);
	/* read to RAM */
	writel(dst, SUNXI_DMA_BASE + SUNXI_DMA_DEST_START_ADDRR_REG0);
	writel(SUNXI_DMA_DDMA_PARA_REG_SRC_WAIT_CYC
			| SUNXI_DMA_DDMA_PARA_REG_SRC_BLK_SIZE,
			SUNXI_DMA_BASE + SUNXI_DMA_DDMA_PARA_REG0);
	writel(CONFIG_NAND_SUNXI_SPL_ECC_PAGE_SIZE,
	       SUNXI_DMA_BASE + SUNXI_DMA_DDMA_BC_REG0); /* 1kB */
	writel(SUNXI_DMA_DDMA_CFG_REG_LOADING
		| SUNXI_DMA_DDMA_CFG_REG_DMA_DEST_DATA_WIDTH_32
		| SUNXI_DMA_DDMA_CFG_REG_DDMA_DST_DRQ_TYPE_DRAM
		| SUNXI_DMA_DDMA_CFG_REG_DMA_SRC_DATA_WIDTH_32
		| SUNXI_DMA_DDMA_CFG_REG_DMA_SRC_ADDR_MODE_IO
		| SUNXI_DMA_DDMA_CFG_REG_DDMA_SRC_DRQ_TYPE_NFC,
		SUNXI_DMA_BASE + SUNXI_DMA_CFG_REG0);

	writel((NFC_CMD_RNDOUTSTART << NFC_RANDOM_READ_CMD1_OFFSET)
		| (NFC_CMD_RNDOUT << NFC_RANDOM_READ_CMD0_OFFSET)
		| (NFC_CMD_READSTART | NFC_READ_CMD_OFFSET), SUNXI_NFC_BASE
			+ NFC_RCMD_SET);
	writel(1, SUNXI_NFC_BASE + NFC_SECTOR_NUM);
	writel(((page & 0xFFFF) << 16) | column,
	       SUNXI_NFC_BASE + NFC_ADDR_LOW);
	writel((page >> 16) & 0xFF, SUNXI_NFC_BASE + NFC_ADDR_HIGH);
	writel(NFC_SEND_CMD1 | NFC_SEND_CMD2 | NFC_DATA_TRANS |
		NFC_PAGE_CMD | NFC_WAIT_FLAG | (4 << NFC_ADDR_NUM_OFFSET) |
		NFC_SEND_ADR | NFC_DATA_SWAP_METHOD | (syndrome ? NFC_SEQ : 0),
		SUNXI_NFC_BASE + NFC_CMD);

	if (!check_value(SUNXI_NFC_BASE + NFC_ST, (1 << 2),
			 MAX_RETRIES)) {
		printf("Error while initializing dma interrupt\n");
		return;
	}

	if (!check_value_negated(SUNXI_DMA_BASE + SUNXI_DMA_CFG_REG0,
				 SUNXI_DMA_DDMA_CFG_REG_LOADING, MAX_RETRIES)) {
		printf("Error while waiting for dma transfer to finish\n");
		return;
	}

	if (readl(SUNXI_NFC_BASE + NFC_ECC_ST))
		(*ecc_errors)++;
}

int nand_spl_load_image(uint32_t offs, unsigned int size, void *dest)
{
	void *current_dest;
	uint32_t ecc_errors = 0;

	for (current_dest = dest;
			current_dest < (dest + size);
			current_dest += CONFIG_NAND_SUNXI_SPL_ECC_PAGE_SIZE) {
		nand_read_page(offs, (dma_addr_t)current_dest,
			offs < CONFIG_NAND_SUNXI_SPL_SYNDROME_PARTITIONS_END,
			&ecc_errors);
		offs += CONFIG_NAND_SUNXI_SPL_ECC_PAGE_SIZE;
	}
	return ecc_errors ? -1 : 0;
}

void nand_deselect(void) {}

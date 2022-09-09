/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MediaTek BootROM NAND header definitions
 *
 * Copyright (C) 2022 MediaTek Inc.
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MTK_NAND_HEADERS_H
#define _MTK_NAND_HEADERS_H

#include <stdint.h>
#include <stdbool.h>

struct nand_header_info {
	uint32_t page_size;
	uint32_t spare_size;
	uint32_t gfh_offset;
};

/* AP BROM Header for NAND */
union nand_boot_header {
	struct {
		char name[12];
		char version[4];
		char id[8];
		uint16_t ioif;			/* I/O interface */
		uint16_t pagesize;		/* NAND page size */
		uint16_t addrcycles;		/* Address cycles */
		uint16_t oobsize;		/* NAND page spare size */
		uint16_t pages_of_block;	/* Pages of one block */
		uint16_t numblocks;		/* Total blocks of NAND chip */
		uint16_t writesize_shift;
		uint16_t erasesize_shift;
		uint8_t dummy[60];
		uint8_t ecc_parity[28];		/* ECC parity of this header */
	};

	uint8_t data[0x80];
};

#define NAND_BOOT_NAME		"BOOTLOADER!"
#define NAND_BOOT_VERSION	"V006"
#define NAND_BOOT_ID		"NFIINFO"

/* Find nand header data by name */
const union nand_boot_header *mtk_nand_header_find(const char *name);

/* Device header size using this nand header */
uint32_t mtk_nand_header_size(const union nand_boot_header *hdr_nand);

/* Get nand info from nand header (page size, spare size, ...) */
int mtk_nand_header_info(const void *ptr, struct nand_header_info *info);

/* Whether given header data is valid */
bool is_mtk_nand_header(const void *ptr);

/* Generate Device header using give nand header */
uint32_t mtk_nand_header_put(const union nand_boot_header *hdr_nand, void *ptr);

#endif /* _MTK_NAND_HEADERS_H */

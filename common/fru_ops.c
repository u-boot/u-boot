// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2019 Xilinx, Inc.
 */

#include <common.h>
#include <fdtdec.h>
#include <fru.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>

struct fru_table fru_data  __attribute__((section(".data")));

static u16 fru_cal_area_len(u8 len)
{
	return len * FRU_COMMON_HDR_LEN_MULTIPLIER;
}

static u8 fru_version(u8 ver)
{
	return ver & FRU_COMMON_HDR_VER_MASK;
}

static int fru_check_language(u8 code)
{
	if (code != FRU_LANG_CODE_ENGLISH && code != FRU_LANG_CODE_ENGLISH_1) {
		printf("FRU_ERROR: Only English Language is supported\n");
		return -EINVAL;
	}

	return code;
}

static u8 fru_checksum(u8 *addr, u8 len)
{
	u8 checksum = 0;

	while (len--) {
		checksum += *addr;
		addr++;
	}

	return checksum;
}

static int fru_check_type_len(u8 type_len, u8 language, u8 *type)
{
	int len;

	if (type_len == FRU_TYPELEN_EOF)
		return -EINVAL;

	*type = (type_len & FRU_TYPELEN_CODE_MASK) >> FRU_TYPELEN_TYPE_SHIFT;

	len = type_len & FRU_TYPELEN_LEN_MASK;

	return len;
}

static int fru_parse_board(unsigned long addr)
{
	u8 i, type;
	int len;
	u8 *data, *term;

	memcpy(&fru_data.brd.ver, (void *)addr, 6);
	addr += 6;
	data = (u8 *)&fru_data.brd.manufacturer_type_len;

	for (i = 0; ; i++, data += FRU_BOARD_MAX_LEN) {
		*data++ = *(u8 *)addr;
		len = fru_check_type_len(*(u8 *)addr, fru_data.brd.lang_code,
					 &type);
		/*
		 * Stop cature if it end of fields
		 */
		if (len == -EINVAL)
			break;

		/*
		 * Dont capture data if type is not ASCII8
		 */
		if (type != FRU_TYPELEN_TYPE_ASCII8)
			return 0;

		addr += 1;
		if (!len)
			continue;
		memcpy(data, (u8 *)addr, len);
		term = data + (u8)len;
		*term = 0;
		addr += len;
	}

	if (i < FRU_BOARD_AREA_TOTAL_FIELDS) {
		printf("Board area require minimum %d fields\n",
		       FRU_BOARD_AREA_TOTAL_FIELDS);
		return -EINVAL;
	}

	return 0;
}

int fru_capture(unsigned long addr)
{
	struct fru_common_hdr *hdr;
	u8 checksum = 0;

	checksum = fru_checksum((u8 *)addr, sizeof(struct fru_common_hdr));
	if (checksum) {
		printf("%s Common header CRC error\n", __func__);
		return -EINVAL;
	}

	hdr = (struct fru_common_hdr *)addr;

	memcpy((void *)&fru_data.hdr, (void *)hdr,
	       sizeof(struct fru_common_hdr));

	fru_data.captured = true;

	if (hdr->off_board) {
		addr += fru_cal_area_len(hdr->off_board);
		fru_parse_board(addr);
	}

	env_set_hex("fru_addr", addr);

	return 0;
}

static int fru_display_board(void)
{
	u32 time = 0;
	u8 type;
	int len;
	u8 *data;
	const char *typecode[] = {
		"Binary/Unspecified",
		"BCD plus",
		"6-bit ASCII",
		"8-bit ASCII",
		"2-byte UNICODE"
	};
	const char *boardinfo[] = {
		"Manufacturer Name",
		"Product Name",
		"Serial No",
		"Part Number",
		"File ID"
	};

	printf("*****BOARD INFO*****\n");
	printf("Version:%d\n", fru_version(fru_data.brd.ver));
	printf("Board Area Length:%d\n", fru_cal_area_len(fru_data.brd.len));

	if (fru_check_language(fru_data.brd.lang_code))
		return 0;

	time = fru_data.brd.time[2] << 16 | fru_data.brd.time[1] << 8 |
	       fru_data.brd.time[0];
	printf("Time in Minutes from 0:00hrs 1/1/96 %d\n", time);

	data = (u8 *)&fru_data.brd.manufacturer_type_len;

	for (u8 i = 0; ; i++) {
		len = fru_check_type_len(*data++, fru_data.brd.lang_code,
					 &type);
		if (len == -EINVAL) {
			printf("**** EOF for Board Area ****\n");
			break;
		}

		if (type <= FRU_TYPELEN_TYPE_ASCII8 &&
		    (fru_data.brd.lang_code == FRU_LANG_CODE_ENGLISH ||
		     fru_data.brd.lang_code == FRU_LANG_CODE_ENGLISH_1))
			printf("Type code: %s\n", typecode[type]);
		else
			printf("Type code: %s\n", typecode[type + 1]);

		if (type != FRU_TYPELEN_TYPE_ASCII8) {
			printf("FRU_ERROR: Only ASCII8 type is supported\n");
			return 0;
		}
		if (!len) {
			printf("%s not found\n", boardinfo[i]);
			continue;
		}

		printf("Length: %d\n", len);
		printf("%s: %s\n", boardinfo[i], data);
		data += FRU_BOARD_MAX_LEN;
	}

	return 0;
}

static void fru_display_common_hdr(void)
{
	struct fru_common_hdr *hdr = &fru_data.hdr;

	printf("*****COMMON HEADER*****\n");
	printf("Version:%d\n", fru_version(hdr->version));
	if (hdr->off_internal)
		printf("Internal Use Area Offset:%d\n",
		       fru_cal_area_len(hdr->off_internal));
	else
		printf("*** No Internal Area ***\n");

	if (hdr->off_chassis)
		printf("Chassis Info Area Offset:%d\n",
		       fru_cal_area_len(hdr->off_chassis));
	else
		printf("*** No Chassis Info Area ***\n");

	if (hdr->off_board)
		printf("Board Area Offset:%d\n",
		       fru_cal_area_len(hdr->off_board));
	else
		printf("*** No Board Area ***\n");

	if (hdr->off_product)
		printf("Product Info Area Offset:%d\n",
		       fru_cal_area_len(hdr->off_product));
	else
		printf("*** No Product Info Area ***\n");

	if (hdr->off_multirec)
		printf("MultiRecord Area Offset:%d\n",
		       fru_cal_area_len(hdr->off_multirec));
	else
		printf("*** No MultiRecord Area ***\n");
}

int fru_display(void)
{
	if (!fru_data.captured) {
		printf("FRU data not available please run fru parse\n");
		return -EINVAL;
	}

	fru_display_common_hdr();
	fru_display_board();

	return 0;
}

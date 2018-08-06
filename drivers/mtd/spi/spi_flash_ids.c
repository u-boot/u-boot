// SPDX-License-Identifier: GPL-2.0+
/*
 * SPI Flash ID's.
 *
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 */

#include <common.h>
#include <spi.h>
#include <spi_flash.h>

#include "sf_internal.h"

/* Used when the "_ext_id" is two bytes at most */
#define INFO(_jedec_id, _ext_id, _sector_size, _n_sectors, _flags)	\
		.id = {							\
			((_jedec_id) >> 16) & 0xff,			\
			((_jedec_id) >> 8) & 0xff,			\
			(_jedec_id) & 0xff,				\
			((_ext_id) >> 8) & 0xff,			\
			(_ext_id) & 0xff,				\
			},						\
		.id_len = (!(_jedec_id) ? 0 : (3 + ((_ext_id) ? 2 : 0))),	\
		.sector_size = (_sector_size),				\
		.n_sectors = (_n_sectors),				\
		.page_size = 256,					\
		.flags = (_flags),

#define INFO6(_jedec_id, _ext_id, _sector_size, _n_sectors, _flags)	\
		.id = {							\
			((_jedec_id) >> 16) & 0xff,			\
			((_jedec_id) >> 8) & 0xff,			\
			(_jedec_id) & 0xff,				\
			((_ext_id) >> 16) & 0xff,			\
			((_ext_id) >> 8) & 0xff,			\
			(_ext_id) & 0xff,				\
			},						\
		.id_len = 6,						\
		.sector_size = (_sector_size),				\
		.n_sectors = (_n_sectors),				\
		.page_size = 256,					\
		.flags = (_flags),

const struct spi_flash_info spi_flash_ids[] = {
#ifdef CONFIG_SPI_FLASH_ATMEL		/* ATMEL */
	{"at45db011d",	   INFO(0x1f2200, 0x0, 64 * 1024,     4, SECT_4K) },
	{"at45db021d",	   INFO(0x1f2300, 0x0, 64 * 1024,     8, SECT_4K) },
	{"at45db041d",	   INFO(0x1f2400, 0x0, 64 * 1024,     8, SECT_4K) },
	{"at45db081d",	   INFO(0x1f2500, 0x0, 64 * 1024,    16, SECT_4K) },
	{"at45db161d",	   INFO(0x1f2600, 0x0, 64 * 1024,    32, SECT_4K) },
	{"at45db321d",	   INFO(0x1f2700, 0x0, 64 * 1024,    64, SECT_4K) },
	{"at45db641d",	   INFO(0x1f2800, 0x0, 64 * 1024,   128, SECT_4K) },
	{"at25df321a",     INFO(0x1f4701, 0x0, 64 * 1024,    64, SECT_4K) },
	{"at25df321",      INFO(0x1f4700, 0x0, 64 * 1024,    64, SECT_4K) },
	{"at26df081a",     INFO(0x1f4501, 0x0, 64 * 1024,    16, SECT_4K) },
#endif
#ifdef CONFIG_SPI_FLASH_EON		/* EON */
	{"en25q32b",	   INFO(0x1c3016, 0x0, 64 * 1024,    64, 0) },
	{"en25q64",	   INFO(0x1c3017, 0x0, 64 * 1024,   128, SECT_4K) },
	{"en25q128b",	   INFO(0x1c3018, 0x0, 64 * 1024,   256, 0) },
	{"en25s64",	   INFO(0x1c3817, 0x0, 64 * 1024,   128, 0) },
#endif
#ifdef CONFIG_SPI_FLASH_GIGADEVICE	/* GIGADEVICE */
	{"gd25q16c",	   INFO(0xc84015, 0x0, 64 * 1024,    32, RD_FULL | WR_QPP | SECT_4K) },
	{"gd25q64b",	   INFO(0xc84017, 0x0, 64 * 1024,   128, SECT_4K) },
	{"gd25q32b",       INFO(0xc84016, 0x0, 64 * 1024,    64, SECT_4K) },
	{"gd25lq32",	   INFO(0xc86016, 0x0, 64 * 1024,    64, SECT_4K) },
#endif
#ifdef CONFIG_SPI_FLASH_ISSI		/* ISSI */
	{"is25lq040b",	   INFO(0x9d4013, 0x0, 64 * 1024,    8, 0)  },
	{"is25lp032",	   INFO(0x9d6016, 0x0, 64 * 1024,    64, 0) },
	{"is25lp064",	   INFO(0x9d6017, 0x0, 64 * 1024,   128, 0) },
	{"is25lp128",	   INFO(0x9d6018, 0x0, 64 * 1024,   256, 0) },
	{"is25lp256",	   INFO(0x9d6019, 0x0, 64 * 1024,   512, 0) },
	{"is25wp032",	   INFO(0x9d7016, 0x0, 64 * 1024,    64, RD_FULL | SECT_4K) },
	{"is25wp064",	   INFO(0x9d7017, 0x0, 64 * 1024,   128, RD_FULL | SECT_4K) },
	{"is25wp128",	   INFO(0x9d7018, 0x0, 64 * 1024,   256, RD_FULL | SECT_4K) },
#endif
#ifdef CONFIG_SPI_FLASH_MACRONIX	/* MACRONIX */
	{"mx25l2006e",	   INFO(0xc22012, 0x0, 64 * 1024,     4, 0) },
	{"mx25l4005",	   INFO(0xc22013, 0x0, 64 * 1024,     8, 0) },
	{"mx25l8005",	   INFO(0xc22014, 0x0, 64 * 1024,    16, 0) },
	{"mx25l1605d",	   INFO(0xc22015, 0x0, 64 * 1024,    32, 0) },
	{"mx25l3205d",	   INFO(0xc22016, 0x0, 64 * 1024,    64, 0) },
	{"mx25l6405d",	   INFO(0xc22017, 0x0, 64 * 1024,   128, 0) },
	{"mx25l12805",	   INFO(0xc22018, 0x0, 64 * 1024,   256, RD_FULL | WR_QPP) },
	{"mx25l25635f",	   INFO(0xc22019, 0x0, 64 * 1024,   512, RD_FULL | WR_QPP) },
	{"mx25l51235f",	   INFO(0xc2201a, 0x0, 64 * 1024,  1024, RD_FULL | WR_QPP) },
	{"mx25l1633e",	   INFO(0xc22415, 0x0, 64 * 1024,    32, RD_FULL | WR_QPP | SECT_4K) },
	{"mx25u6435f",	   INFO(0xc22537, 0x0, 64 * 1024,   128, RD_FULL | WR_QPP) },
	{"mx25l12855e",	   INFO(0xc22618, 0x0, 64 * 1024,   256, RD_FULL | WR_QPP) },
	{"mx25u1635e",     INFO(0xc22535, 0x0, 64 * 1024,  32, SECT_4K) },
	{"mx25u25635f",    INFO(0xc22539, 0x0, 64 * 1024,   512, RD_FULL | WR_QPP) },
	{"mx66u51235f",    INFO(0xc2253a, 0x0, 64 * 1024,  1024, RD_FULL | WR_QPP) },
	{"mx66l1g45g",     INFO(0xc2201b, 0x0, 64 * 1024,  2048, RD_FULL | WR_QPP) },
#endif
#ifdef CONFIG_SPI_FLASH_SPANSION	/* SPANSION */
	{"s25fl008a",	   INFO(0x010213, 0x0, 64 * 1024,    16, 0) },
	{"s25fl016a",	   INFO(0x010214, 0x0, 64 * 1024,    32, 0) },
	{"s25fl032a",	   INFO(0x010215, 0x0, 64 * 1024,    64, 0) },
	{"s25fl064a",	   INFO(0x010216, 0x0, 64 * 1024,   128, 0) },
	{"s25fl208k",	   INFO(0x014014, 0x0, 64 * 1024,    16, 0) },
	{"s25fl116k",	   INFO(0x014015, 0x0, 64 * 1024,    32, 0) },
	{"s25fl164k",	   INFO(0x014017, 0x0140,  64 * 1024,   128, 0) },
	{"s25fl128p_256k", INFO(0x012018, 0x0300, 256 * 1024,    64, RD_FULL | WR_QPP) },
	{"s25fl128p_64k",  INFO(0x012018, 0x0301,  64 * 1024,   256, RD_FULL | WR_QPP) },
	{"s25fl032p",	   INFO(0x010215, 0x4d00,  64 * 1024,    64, RD_FULL | WR_QPP) },
	{"s25fl064p",	   INFO(0x010216, 0x4d00,  64 * 1024,   128, RD_FULL | WR_QPP) },
	{"s25fl128s_256k", INFO(0x012018, 0x4d00, 256 * 1024,    64, RD_FULL | WR_QPP) },
	{"s25fl128s_64k",  INFO(0x012018, 0x4d01,  64 * 1024,   256, RD_FULL | WR_QPP) },
	{"s25fl128l",      INFO(0x016018, 0, 64 * 1024,    256, RD_FULL | WR_QPP) },
	{"s25fl256s_256k", INFO(0x010219, 0x4d00, 256 * 1024,   128, RD_FULL | WR_QPP) },
	{"s25fs256s_64k",  INFO6(0x010219, 0x4d0181, 64 * 1024, 512, RD_FULL | WR_QPP | SECT_4K) },
	{"s25fl256s_64k",  INFO(0x010219, 0x4d01,  64 * 1024,   512, RD_FULL | WR_QPP) },
	{"s25fs512s",      INFO6(0x010220, 0x4d0081, 256 * 1024, 256, RD_FULL | WR_QPP | SECT_4K) },
	{"s25fl512s_256k", INFO(0x010220, 0x4d00, 256 * 1024,   256, RD_FULL | WR_QPP) },
	{"s25fl512s_64k",  INFO(0x010220, 0x4d01,  64 * 1024,  1024, RD_FULL | WR_QPP) },
	{"s25fl512s_512k", INFO(0x010220, 0x4f00, 256 * 1024,   256, RD_FULL | WR_QPP) },
#endif
#ifdef CONFIG_SPI_FLASH_STMICRO		/* STMICRO */
	{"m25p10",	   INFO(0x202011, 0x0, 32 * 1024,     4, 0) },
	{"m25p20",	   INFO(0x202012, 0x0, 64 * 1024,     4, 0) },
	{"m25p40",	   INFO(0x202013, 0x0, 64 * 1024,     8, 0) },
	{"m25p80",	   INFO(0x202014, 0x0, 64 * 1024,    16, 0) },
	{"m25p16",	   INFO(0x202015, 0x0, 64 * 1024,    32, 0) },
	{"m25pE16",	   INFO(0x208015, 0x1000, 64 * 1024, 32, 0) },
	{"m25pX16",	   INFO(0x207115, 0x1000, 64 * 1024, 32, RD_QUAD | RD_DUAL) },
	{"m25p32",	   INFO(0x202016, 0x0,  64 * 1024,    64, 0) },
	{"m25p64",	   INFO(0x202017, 0x0,  64 * 1024,   128, 0) },
	{"m25p128",	   INFO(0x202018, 0x0, 256 * 1024,    64, 0) },
	{"m25pX64",	   INFO(0x207117, 0x0,  64 * 1024,   128, SECT_4K) },
	{"n25q016a",       INFO(0x20bb15, 0x0,	64 * 1024,    32, SECT_4K) },
	{"n25q32",	   INFO(0x20ba16, 0x0,  64 * 1024,    64, RD_FULL | WR_QPP | SECT_4K) },
	{"n25q32a",	   INFO(0x20bb16, 0x0,  64 * 1024,    64, RD_FULL | WR_QPP | SECT_4K) },
	{"n25q64",	   INFO(0x20ba17, 0x0,  64 * 1024,   128, RD_FULL | WR_QPP | SECT_4K) },
	{"n25q64a",	   INFO(0x20bb17, 0x0,  64 * 1024,   128, RD_FULL | WR_QPP | SECT_4K) },
	{"n25q128",	   INFO(0x20ba18, 0x0,  64 * 1024,   256, RD_FULL | WR_QPP) },
	{"n25q128a",	   INFO(0x20bb18, 0x0,  64 * 1024,   256, RD_FULL | WR_QPP) },
	{"n25q256",	   INFO(0x20ba19, 0x0,  64 * 1024,   512, RD_FULL | WR_QPP | E_FSR | SECT_4K) },
	{"n25q256a",	   INFO(0x20bb19, 0x0,  64 * 1024,   512, RD_FULL | WR_QPP | E_FSR | SECT_4K) },
	{"n25q512",	   INFO(0x20ba20, 0x0,  64 * 1024,  1024, RD_FULL | WR_QPP | E_FSR | SECT_4K) },
	{"n25q512a",	   INFO(0x20bb20, 0x0,  64 * 1024,  1024, RD_FULL | WR_QPP | E_FSR | SECT_4K) },
	{"n25q1024",	   INFO(0x20ba21, 0x0,  64 * 1024,  2048, RD_FULL | WR_QPP | E_FSR | SECT_4K) },
	{"n25q1024a",	   INFO(0x20bb21, 0x0,  64 * 1024,  2048, RD_FULL | WR_QPP | E_FSR | SECT_4K) },
	{"mt25qu02g",	   INFO(0x20bb22, 0x0,  64 * 1024,  4096, RD_FULL | WR_QPP | E_FSR | SECT_4K) },
	{"mt25ql02g",	   INFO(0x20ba22, 0x0,  64 * 1024,  4096, RD_FULL | WR_QPP | E_FSR | SECT_4K) },
	{"mt35xu512g",	   INFO6(0x2c5b1a, 0x104100,  128 * 1024,  512, E_FSR | SECT_4K) },
#endif
#ifdef CONFIG_SPI_FLASH_SST		/* SST */
	{"sst25vf040b",	   INFO(0xbf258d, 0x0,	64 * 1024,     8, SECT_4K | SST_WR) },
	{"sst25vf080b",	   INFO(0xbf258e, 0x0,	64 * 1024,    16, SECT_4K | SST_WR) },
	{"sst25vf016b",	   INFO(0xbf2541, 0x0,	64 * 1024,    32, SECT_4K | SST_WR) },
	{"sst25vf032b",	   INFO(0xbf254a, 0x0,	64 * 1024,    64, SECT_4K | SST_WR) },
	{"sst25vf064c",	   INFO(0xbf254b, 0x0,	64 * 1024,   128, SECT_4K) },
	{"sst25wf512",	   INFO(0xbf2501, 0x0,	64 * 1024,     1, SECT_4K | SST_WR) },
	{"sst25wf010",	   INFO(0xbf2502, 0x0,	64 * 1024,     2, SECT_4K | SST_WR) },
	{"sst25wf020",	   INFO(0xbf2503, 0x0,	64 * 1024,     4, SECT_4K | SST_WR) },
	{"sst25wf040",	   INFO(0xbf2504, 0x0,	64 * 1024,     8, SECT_4K | SST_WR) },
	{"sst25wf040b",	   INFO(0x621613, 0x0,	64 * 1024,     8, SECT_4K) },
	{"sst25wf080",	   INFO(0xbf2505, 0x0,	64 * 1024,    16, SECT_4K | SST_WR) },
	{"sst26wf016",	   INFO(0xbf2651, 0x0,	64 * 1024,    32, SECT_4K) },
	{"sst26wf032",	   INFO(0xbf2622, 0x0,	64 * 1024,    64, SECT_4K) },
	{"sst26wf064",	   INFO(0xbf2643, 0x0,	64 * 1024,   128, SECT_4K) },
#endif
#ifdef CONFIG_SPI_FLASH_WINBOND		/* WINBOND */
	{"w25p80",	   INFO(0xef2014, 0x0,	64 * 1024,    16, 0) },
	{"w25p16",	   INFO(0xef2015, 0x0,	64 * 1024,    32, 0) },
	{"w25p32",	   INFO(0xef2016, 0x0,	64 * 1024,    64, 0) },
	{"w25x40",	   INFO(0xef3013, 0x0,	64 * 1024,     8, SECT_4K) },
	{"w25x16",	   INFO(0xef3015, 0x0,	64 * 1024,    32, SECT_4K) },
	{"w25x32",	   INFO(0xef3016, 0x0,	64 * 1024,    64, SECT_4K) },
	{"w25x64",	   INFO(0xef3017, 0x0,	64 * 1024,   128, SECT_4K) },
	{"w25q80bl",	   INFO(0xef4014, 0x0,	64 * 1024,    16, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q16cl",	   INFO(0xef4015, 0x0,	64 * 1024,    32, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q32bv",	   INFO(0xef4016, 0x0,	64 * 1024,    64, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q64cv",	   INFO(0xef4017, 0x0,	64 * 1024,   128, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q128bv",	   INFO(0xef4018, 0x0,	64 * 1024,   256, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q256",	   INFO(0xef4019, 0x0,	64 * 1024,   512, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q80bw",	   INFO(0xef5014, 0x0,	64 * 1024,    16, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q16dw",	   INFO(0xef6015, 0x0,	64 * 1024,    32, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q16jv",	   INFO(0xef7015, 0x0,	64 * 1024,    32, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q32dw",	   INFO(0xef6016, 0x0,	64 * 1024,    64, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q32jv",	   INFO(0xef7016, 0x0,	64 * 1024,    64, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q64dw",	   INFO(0xef6017, 0x0,	64 * 1024,   128, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q64jv",	   INFO(0xef7017, 0x0,	64 * 1024,   128, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q128fw",	   INFO(0xef6018, 0x0,	64 * 1024,   256, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q128jv",	   INFO(0xef7018, 0x0,	64 * 1024,   256, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q256fw",	   INFO(0xef6019, 0x0,	64 * 1024,   512, RD_FULL | WR_QPP | SECT_4K) },
	{"w25q256jw",	   INFO(0xef7019, 0x0,	64 * 1024,   512, RD_FULL | WR_QPP | SECT_4K) },
#endif
#ifdef CONFIG_SPI_FLASH_XMC /* Wuhan Xinxin Semiconductor Manufacturing Corp */
	{ "xm25qh64a",	   INFO(0x207017, 0x0, 64 * 1024,    128, SECT_4K | RD_DUAL | RD_QUAD) },
	{ "xm25qh128a",	   INFO(0x207018, 0x0, 64 * 1024,    256, SECT_4K | RD_DUAL | RD_QUAD) },
#endif
	{},	/* Empty entry to terminate the list */
	/*
	 * Note:
	 * Below paired flash devices has similar spi_flash params.
	 * (s25fl129p_64k, s25fl128s_64k)
	 * (w25q80bl, w25q80bv)
	 * (w25q16cl, w25q16dv, w25q16jv)
	 * (w25q32bv, w25q32fv_spi)
	 * (w25q64cv, w25q64fv_spi)
	 * (w25q128bv, w25q128fv_spi)
	 * (w25q32dw, w25q32fv_qpi)
	 * (w25q64dw, w25q64fv_qpi)
	 * (w25q128fw, w25q128fv_qpi)
	 * (w25q256fw, w25q256fv_qpi)
	 */
};

/*
 * SPI flash Params table
 *
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

/* SPI/QSPI flash device params structure */
const struct spi_flash_info spi_flash_ids[] = {
#ifdef CONFIG_SPI_FLASH_ATMEL		/* ATMEL */
	{"AT45DB011D",	   INFO(0x1f2200, 0x0, 64 * 1024,     4, SECT_4K) },
	{"AT45DB021D",	   INFO(0x1f2300, 0x0, 64 * 1024,     8, SECT_4K) },
	{"AT45DB041D",	   INFO(0x1f2400, 0x0, 64 * 1024,     8, SECT_4K) },
	{"AT45DB081D",	   INFO(0x1f2500, 0x0, 64 * 1024,    16, SECT_4K) },
	{"AT45DB161D",	   INFO(0x1f2600, 0x0, 64 * 1024,    32, SECT_4K) },
	{"AT45DB321D",	   INFO(0x1f2700, 0x0, 64 * 1024,    64, SECT_4K) },
	{"AT45DB641D",	   INFO(0x1f2800, 0x0, 64 * 1024,   128, SECT_4K) },
	{"AT25DF321A",     INFO(0x1f4701, 0x0, 64 * 1024,    64, SECT_4K) },
	{"AT25DF321",      INFO(0x1f4700, 0x0, 64 * 1024,    64, SECT_4K) },
	{"AT26DF081A",     INFO(0x1f4501, 0x0, 64 * 1024,    16, SECT_4K) },
#endif
#ifdef CONFIG_SPI_FLASH_EON		/* EON */
	{"EN25Q32B",	   INFO(0x1c3016, 0x0, 64 * 1024,    64, 0) },
	{"EN25Q64",	   INFO(0x1c3017, 0x0, 64 * 1024,   128, SECT_4K) },
	{"EN25Q128B",	   INFO(0x1c3018, 0x0, 64 * 1024,   256, 0) },
	{"EN25S64",	   INFO(0x1c3817, 0x0, 64 * 1024,   128, 0) },
#endif
#ifdef CONFIG_SPI_FLASH_GIGADEVICE	/* GIGADEVICE */
	{"GD25Q64B",	   INFO(0xc84017, 0x0, 64 * 1024,   128, SECT_4K) },
	{"GD25LQ32",	   INFO(0xc86016, 0x0, 64 * 1024,    64, SECT_4K) },
#endif
#ifdef CONFIG_SPI_FLASH_ISSI		/* ISSI */
	{"IS25LP032",	   INFO(0x9d6016, 0x0, 64 * 1024,    64, 0) },
	{"IS25LP064",	   INFO(0x9d6017, 0x0, 64 * 1024,   128, 0) },
	{"IS25LP128",	   INFO(0x9d6018, 0x0, 64 * 1024,   256, 0) },
#endif
#ifdef CONFIG_SPI_FLASH_MACRONIX	/* MACRONIX */
	{"MX25L2006E",	   INFO(0xc22012, 0x0, 64 * 1024,     4, 0) },
	{"MX25L4005",	   INFO(0xc22013, 0x0, 64 * 1024,     8, 0) },
	{"MX25L8005",	   INFO(0xc22014, 0x0, 64 * 1024,    16, 0) },
	{"MX25L1605D",	   INFO(0xc22015, 0x0, 64 * 1024,    32, 0) },
	{"MX25L3205D",	   INFO(0xc22016, 0x0, 64 * 1024,    64, 0) },
	{"MX25L6405D",	   INFO(0xc22017, 0x0, 64 * 1024,   128, 0) },
	{"MX25L12805",	   INFO(0xc22018, 0x0, 64 * 1024,   256, RD_FULL | WR_QPP) },
	{"MX25L25635F",	   INFO(0xc22019, 0x0, 64 * 1024,   512, RD_FULL | WR_QPP) },
	{"MX25L51235F",	   INFO(0xc2201a, 0x0, 64 * 1024,  1024, RD_FULL | WR_QPP) },
	{"MX25L12855E",	   INFO(0xc22618, 0x0, 64 * 1024,   256, RD_FULL | WR_QPP) },
#endif
#ifdef CONFIG_SPI_FLASH_SPANSION	/* SPANSION */
	{"S25FL008A",	   INFO(0x010213, 0x0, 64 * 1024,    16, 0) },
	{"S25FL016A",	   INFO(0x010214, 0x0, 64 * 1024,    32, 0) },
	{"S25FL032A",	   INFO(0x010215, 0x0, 64 * 1024,    64, 0) },
	{"S25FL064A",	   INFO(0x010216, 0x0, 64 * 1024,   128, 0) },
	{"S25FL116K",	   INFO(0x014015, 0x0, 64 * 1024,   128, 0) },
	{"S25FL164K",	   INFO(0x014017, 0x0140,  64 * 1024,   128, 0) },
	{"S25FL128P_256K", INFO(0x012018, 0x0300, 256 * 1024,    64, RD_FULL | WR_QPP) },
	{"S25FL128P_64K",  INFO(0x012018, 0x0301,  64 * 1024,   256, RD_FULL | WR_QPP) },
	{"S25FL032P",	   INFO(0x010215, 0x4d00,  64 * 1024,    64, RD_FULL | WR_QPP) },
	{"S25FL064P",	   INFO(0x010216, 0x4d00,  64 * 1024,   128, RD_FULL | WR_QPP) },
	{"S25FL128S_256K", INFO(0x012018, 0x4d00, 256 * 1024,    64, RD_FULL | WR_QPP) },
	{"S25FL128S_64K",  INFO(0x012018, 0x4d01,  64 * 1024,   256, RD_FULL | WR_QPP) },
	{"S25FL256S_256K", INFO(0x010219, 0x4d00, 256 * 1024,   128, RD_FULL | WR_QPP) },
	{"S25FL256S_64K",  INFO(0x010219, 0x4d01,  64 * 1024,   512, RD_FULL | WR_QPP) },
	{"S25FS256S_64K",  INFO6(0x010219, 0x4d0181, 64 * 1024, 512, RD_FULL | WR_QPP | SECT_4K) },
	{"S25FS512S",      INFO(0x010220, 0x4D00, 128 * 1024,   512, RD_FULL | WR_QPP) },
	{"S25FL512S_256K", INFO(0x010220, 0x4d00, 256 * 1024,   256, RD_FULL | WR_QPP) },
	{"S25FL512S_64K",  INFO(0x010220, 0x4d01,  64 * 1024,  1024, RD_FULL | WR_QPP) },
	{"S25FL512S_512K", INFO(0x010220, 0x4f00, 256 * 1024,   256, RD_FULL | WR_QPP) },
#endif
#ifdef CONFIG_SPI_FLASH_STMICRO		/* STMICRO */
	{"M25P10",	   INFO(0x202011, 0x0, 32 * 1024,     4, 0) },
	{"M25P20",	   INFO(0x202012, 0x0, 64 * 1024,     4, 0) },
	{"M25P40",	   INFO(0x202013, 0x0, 64 * 1024,     8, 0) },
	{"M25P80",	   INFO(0x202014, 0x0, 64 * 1024,    16, 0) },
	{"M25P16",	   INFO(0x202015, 0x0, 64 * 1024,    32, 0) },
	{"M25PE16",	   INFO(0x208015, 0x1000, 64 * 1024, 32, 0) },
	{"M25PX16",	   INFO(0x207115, 0x1000, 64 * 1024, 32, RD_QUAD | RD_DUAL) },
	{"M25P32",	   INFO(0x202016, 0x0,  64 * 1024,    64, 0) },
	{"M25P64",	   INFO(0x202017, 0x0,  64 * 1024,   128, 0) },
	{"M25P128",	   INFO(0x202018, 0x0, 256 * 1024,    64, 0) },
	{"M25PX64",	   INFO(0x207117, 0x0,  64 * 1024,   128, SECT_4K) },
	{"N25Q016A",       INFO(0x20bb15, 0x0,	64 * 1024,    32, SECT_4K) },
	{"N25Q32",	   INFO(0x20ba16, 0x0,  64 * 1024,    64, RD_FULL | WR_QPP | SECT_4K) },
	{"N25Q32A",	   INFO(0x20bb16, 0x0,  64 * 1024,    64, RD_FULL | WR_QPP | SECT_4K) },
	{"N25Q64",	   INFO(0x20ba17, 0x0,  64 * 1024,   128, RD_FULL | WR_QPP | SECT_4K) },
	{"N25Q64A",	   INFO(0x20bb17, 0x0,  64 * 1024,   128, RD_FULL | WR_QPP | SECT_4K) },
	{"N25Q128",	   INFO(0x20ba18, 0x0,  64 * 1024,   256, RD_FULL | WR_QPP) },
	{"N25Q128A",	   INFO(0x20bb18, 0x0,  64 * 1024,   256, RD_FULL | WR_QPP) },
	{"N25Q256",	   INFO(0x20ba19, 0x0,  64 * 1024,   512, RD_FULL | WR_QPP | SECT_4K) },
	{"N25Q256A",	   INFO(0x20bb19, 0x0,  64 * 1024,   512, RD_FULL | WR_QPP | SECT_4K) },
	{"N25Q512",	   INFO(0x20ba20, 0x0,  64 * 1024,  1024, RD_FULL | WR_QPP | E_FSR | SECT_4K) },
	{"N25Q512A",	   INFO(0x20bb20, 0x0,  64 * 1024,  1024, RD_FULL | WR_QPP | E_FSR | SECT_4K) },
	{"N25Q1024",	   INFO(0x20ba21, 0x0,  64 * 1024,  2048, RD_FULL | WR_QPP | E_FSR | SECT_4K) },
	{"N25Q1024A",	   INFO(0x20bb21, 0x0,  64 * 1024,  2048, RD_FULL | WR_QPP | E_FSR | SECT_4K) },
#endif
#ifdef CONFIG_SPI_FLASH_SST		/* SST */
	{"SST25VF040B",	   INFO(0xbf258d, 0x0,	64 * 1024,     8, SECT_4K | SST_WR) },
	{"SST25VF080B",	   INFO(0xbf258e, 0x0,	64 * 1024,    16, SECT_4K | SST_WR) },
	{"SST25VF016B",	   INFO(0xbf2541, 0x0,	64 * 1024,    32, SECT_4K | SST_WR) },
	{"SST25VF032B",	   INFO(0xbf254a, 0x0,	64 * 1024,    64, SECT_4K | SST_WR) },
	{"SST25VF064C",	   INFO(0xbf254b, 0x0,	64 * 1024,   128, SECT_4K) },
	{"SST25WF512",	   INFO(0xbf2501, 0x0,	64 * 1024,     1, SECT_4K | SST_WR) },
	{"SST25WF010",	   INFO(0xbf2502, 0x0,	64 * 1024,     2, SECT_4K | SST_WR) },
	{"SST25WF020",	   INFO(0xbf2503, 0x0,	64 * 1024,     4, SECT_4K | SST_WR) },
	{"SST25WF040",	   INFO(0xbf2504, 0x0,	64 * 1024,     8, SECT_4K | SST_WR) },
	{"SST25WF040B",	   INFO(0x621613, 0x0,	64 * 1024,     8, SECT_4K) },
	{"SST25WF080",	   INFO(0xbf2505, 0x0,	64 * 1024,    16, SECT_4K | SST_WR) },
#endif
#ifdef CONFIG_SPI_FLASH_WINBOND		/* WINBOND */
	{"W25P80",	   INFO(0xef2014, 0x0,	64 * 1024,    16, 0) },
	{"W25P16",	   INFO(0xef2015, 0x0,	64 * 1024,    32, 0) },
	{"W25P32",	   INFO(0xef2016, 0x0,	64 * 1024,    64, 0) },
	{"W25X40",	   INFO(0xef3013, 0x0,	64 * 1024,     8, SECT_4K) },
	{"W25X16",	   INFO(0xef3015, 0x0,	64 * 1024,    32, SECT_4K) },
	{"W25X32",	   INFO(0xef3016, 0x0,	64 * 1024,    64, SECT_4K) },
	{"W25X64",	   INFO(0xef3017, 0x0,	64 * 1024,   128, SECT_4K) },
	{"W25Q80BL",	   INFO(0xef4014, 0x0,	64 * 1024,    16, RD_FULL | WR_QPP | SECT_4K) },
	{"W25Q16CL",	   INFO(0xef4015, 0x0,	64 * 1024,    32, RD_FULL | WR_QPP | SECT_4K) },
	{"W25Q32BV",	   INFO(0xef4016, 0x0,	64 * 1024,    64, RD_FULL | WR_QPP | SECT_4K) },
	{"W25Q64CV",	   INFO(0xef4017, 0x0,	64 * 1024,   128, RD_FULL | WR_QPP | SECT_4K) },
	{"W25Q128BV",	   INFO(0xef4018, 0x0,	64 * 1024,   256, RD_FULL | WR_QPP | SECT_4K) },
	{"W25Q256",	   INFO(0xef4019, 0x0,	64 * 1024,   512, RD_FULL | WR_QPP | SECT_4K) },
	{"W25Q80BW",	   INFO(0xef5014, 0x0,	64 * 1024,    16, RD_FULL | WR_QPP | SECT_4K) },
	{"W25Q16DW",	   INFO(0xef6015, 0x0,	64 * 1024,    32, RD_FULL | WR_QPP | SECT_4K) },
	{"W25Q32DW",	   INFO(0xef6016, 0x0,	64 * 1024,    64, RD_FULL | WR_QPP | SECT_4K) },
	{"W25Q64DW",	   INFO(0xef6017, 0x0,	64 * 1024,   128, RD_FULL | WR_QPP | SECT_4K) },
	{"W25Q128FW",	   INFO(0xef6018, 0x0,	64 * 1024,   256, RD_FULL | WR_QPP | SECT_4K) },
#endif
	{},	/* Empty entry to terminate the list */
	/*
	 * Note:
	 * Below paired flash devices has similar spi_flash params.
	 * (S25FL129P_64K, S25FL128S_64K)
	 * (W25Q80BL, W25Q80BV)
	 * (W25Q16CL, W25Q16DV)
	 * (W25Q32BV, W25Q32FV_SPI)
	 * (W25Q64CV, W25Q64FV_SPI)
	 * (W25Q128BV, W25Q128FV_SPI)
	 * (W25Q32DW, W25Q32FV_QPI)
	 * (W25Q64DW, W25Q64FV_QPI)
	 * (W25Q128FW, W25Q128FV_QPI)
	 */
};

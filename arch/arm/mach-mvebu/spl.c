// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <debug_uart.h>
#include <fdtdec.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#if defined(CONFIG_SPL_SPI_FLASH_SUPPORT) || defined(CONFIG_SPL_MMC_SUPPORT) || defined(CONFIG_SPL_SATA_SUPPORT)

/*
 * When loading U-Boot via SPL from SPI NOR, CONFIG_SYS_SPI_U_BOOT_OFFS must
 * point to the offset of kwbimage main header which is always at offset zero
 * (defined by BootROM). Therefore other values of CONFIG_SYS_SPI_U_BOOT_OFFS
 * makes U-Boot non-bootable.
 */
#ifdef CONFIG_SPL_SPI_FLASH_SUPPORT
#if defined(CONFIG_SYS_SPI_U_BOOT_OFFS) && CONFIG_SYS_SPI_U_BOOT_OFFS != 0
#error CONFIG_SYS_SPI_U_BOOT_OFFS must be set to 0
#endif
#endif

/*
 * When loading U-Boot via SPL from eMMC (in Marvell terminology SDIO), the
 * kwbimage main header is stored at sector 0. U-Boot SPL needs to parse this
 * header and figure out at which sector the U-Boot proper binary is stored.
 * Partition booting is therefore not supported and CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
 * and CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET need to point to the
 * kwbimage main header.
 */
#ifdef CONFIG_SPL_MMC_SUPPORT
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION
#error CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION is unsupported
#endif
#if defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR) && CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR != 0
#error CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR must be set to 0
#endif
#if defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET) && CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET != 0
#error CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET must be set to 0
#endif
#endif

/*
 * When loading U-Boot via SPL from SATA disk, the kwbimage main header is
 * stored at sector 1. Therefore CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR must be
 * set to 1. Otherwise U-Boot SPL would not be able to load U-Boot proper.
 */
#ifdef CONFIG_SPL_SATA_SUPPORT
#if !defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR) || !defined(CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR) || CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR != 1
#error CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR must be set to 1
#endif
#endif

/* Boot Type - block ID */
#define IBR_HDR_I2C_ID			0x4D
#define IBR_HDR_SPI_ID			0x5A
#define IBR_HDR_NAND_ID			0x8B
#define IBR_HDR_SATA_ID			0x78
#define IBR_HDR_PEX_ID			0x9C
#define IBR_HDR_UART_ID			0x69
#define IBR_HDR_SDIO_ID			0xAE

/* Structure of the main header, version 1 (Armada 370/38x/XP) */
struct kwbimage_main_hdr_v1 {
	uint8_t  blockid;               /* 0x0       */
	uint8_t  flags;                 /* 0x1       */
	uint16_t reserved2;             /* 0x2-0x3   */
	uint32_t blocksize;             /* 0x4-0x7   */
	uint8_t  version;               /* 0x8       */
	uint8_t  headersz_msb;          /* 0x9       */
	uint16_t headersz_lsb;          /* 0xA-0xB   */
	uint32_t srcaddr;               /* 0xC-0xF   */
	uint32_t destaddr;              /* 0x10-0x13 */
	uint32_t execaddr;              /* 0x14-0x17 */
	uint8_t  options;               /* 0x18      */
	uint8_t  nandblocksize;         /* 0x19      */
	uint8_t  nandbadblklocation;    /* 0x1A      */
	uint8_t  reserved4;             /* 0x1B      */
	uint16_t reserved5;             /* 0x1C-0x1D */
	uint8_t  ext;                   /* 0x1E      */
	uint8_t  checksum;              /* 0x1F      */
} __packed;

#ifdef CONFIG_SPL_MMC_SUPPORT
u32 spl_mmc_boot_mode(const u32 boot_device)
{
	return MMCSD_MODE_RAW;
}
#endif

int spl_parse_board_header(struct spl_image_info *spl_image,
			   const void *image_header, size_t size)
{
	const struct kwbimage_main_hdr_v1 *mhdr = image_header;

	if (size < sizeof(*mhdr)) {
		/* This should be compile time assert */
		printf("FATAL ERROR: Image header size is too small\n");
		hang();
	}

	/*
	 * Very basic check for image validity. We cannot check mhdr->checksum
	 * as it is calculated also from variable length extended headers
	 * (including SPL content) which is not included in U-Boot image_header.
	 */
	if (mhdr->version != 1 ||
	    ((mhdr->headersz_msb << 16) | mhdr->headersz_lsb) < sizeof(*mhdr) ||
	    (
#ifdef CONFIG_SPL_SPI_FLASH_SUPPORT
	     mhdr->blockid != IBR_HDR_SPI_ID &&
#endif
#ifdef CONFIG_SPL_SATA_SUPPORT
	     mhdr->blockid != IBR_HDR_SATA_ID &&
#endif
#ifdef CONFIG_SPL_MMC_SUPPORT
	     mhdr->blockid != IBR_HDR_SDIO_ID &&
#endif
	     1
	    )) {
		printf("ERROR: Not valid SPI/NAND/SATA/SDIO kwbimage v1\n");
		return -EINVAL;
	}

	spl_image->offset = mhdr->srcaddr;

#ifdef CONFIG_SPL_SATA_SUPPORT
	/*
	 * For SATA srcaddr is specified in number of sectors.
	 * The main header is must be stored at sector number 1.
	 * This expects that sector size is 512 bytes and recalculates
	 * data offset to bytes relative to the main header.
	 */
	if (mhdr->blockid == IBR_HDR_SATA_ID) {
		if (spl_image->offset < 1) {
			printf("ERROR: Wrong SATA srcaddr in kwbimage\n");
			return -EINVAL;
		}
		spl_image->offset -= 1;
		spl_image->offset *= 512;
	}
#endif

#ifdef CONFIG_SPL_MMC_SUPPORT
	/*
	 * For SDIO (eMMC) srcaddr is specified in number of sectors.
	 * This expects that sector size is 512 bytes and recalculates
	 * data offset to bytes.
	 */
	if (mhdr->blockid == IBR_HDR_SDIO_ID)
		spl_image->offset *= 512;
#endif

	spl_image->size = mhdr->blocksize;
	spl_image->entry_point = mhdr->execaddr;
	spl_image->load_addr = mhdr->destaddr;
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = "U-Boot";

	return 0;
}

u32 spl_boot_device(void)
{
	u32 boot_device = get_boot_device();

	switch (boot_device) {
	/*
	 * Return to the BootROM to continue the Marvell xmodem
	 * UART boot protocol. As initiated by the kwboot tool.
	 *
	 * This can only be done by the BootROM since the beginning
	 * of the image is already read and interpreted by the BootROM.
	 * SPL has no chance to receive this information. So we
	 * need to return to the BootROM to enable this xmodem
	 * UART download. Use SPL infrastructure to return to BootROM.
	 */
	case BOOT_DEVICE_UART:
		return BOOT_DEVICE_BOOTROM;

	/*
	 * If SPL is compiled with chosen boot_device support
	 * then use SPL driver for loading U-Boot proper.
	 */
#ifdef CONFIG_SPL_MMC_SUPPORT
	case BOOT_DEVICE_MMC1:
		return BOOT_DEVICE_MMC1;
#endif
#ifdef CONFIG_SPL_SATA_SUPPORT
	case BOOT_FROM_SATA:
		return BOOT_FROM_SATA;
#endif
#ifdef CONFIG_SPL_SPI_FLASH_SUPPORT
	case BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;
#endif

	/*
	 * If SPL is not compiled with chosen boot_device support
	 * then return to the BootROM. BootROM supports loading
	 * U-Boot proper from any valid boot_device present in SAR
	 * register.
	 */
	default:
		return BOOT_DEVICE_BOOTROM;
	}
}

#else

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_BOOTROM;
}

#endif

int board_return_to_bootrom(struct spl_image_info *spl_image,
			    struct spl_boot_device *bootdev)
{
	u32 *regs = *(u32 **)CONFIG_SPL_BOOTROM_SAVE;

	printf("Returning to BootROM (return address 0x%08x)...\n", regs[13]);
	return_to_bootrom();

	/* NOTREACHED - return_to_bootrom() does not return */
	hang();
}

void board_init_f(ulong dummy)
{
	int ret;

	/*
	 * Pin muxing needs to be done before UART output, since
	 * on A38x the UART pins need some re-muxing for output
	 * to work.
	 */
	board_early_init_f();

	/* Example code showing how to enable the debug UART on MVEBU */
#ifdef EARLY_UART
	/*
	 * Debug UART can be used from here if required:
	 *
	 * debug_uart_init();
	 * printch('a');
	 * printhex8(0x1234);
	 * printascii("string");
	 */
#endif

	/*
	 * Use special translation offset for SPL. This needs to be
	 * configured *before* spl_init() is called as this function
	 * calls dm_init() which calls the bind functions of the
	 * device drivers. Here the base address needs to be configured
	 * (translated) correctly.
	 */
	gd->translation_offset = 0xd0000000 - 0xf1000000;

	ret = spl_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	preloader_console_init();

	timer_init();

	/* Armada 375 does not support SerDes and DDR3 init yet */
#if !defined(CONFIG_ARMADA_375)
	/* First init the serdes PHY's */
	serdes_phy_config();

	/* Setup DDR */
	ret = ddr3_init();
	if (ret) {
		debug("ddr3_init() failed: %d\n", ret);
		hang();
	}
#endif

	/* Initialize Auto Voltage Scaling */
	mv_avs_init();

	/* Update read timing control for PCIe */
	mv_rtc_config();
}

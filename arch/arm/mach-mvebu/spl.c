// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
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

#if defined(CONFIG_SPL_SPI_FLASH_SUPPORT) || defined(CONFIG_SPL_MMC) || \
	defined(CONFIG_SPL_SATA)

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
#ifdef CONFIG_SPL_MMC
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION
#error CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION is unsupported
#endif
#if defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR) && CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR != 0
#error CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR must be set to 0
#endif
#if defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET) && \
    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET != 0
#error CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET must be set to 0
#endif
#endif

/*
 * When loading U-Boot via SPL from SATA disk, the kwbimage main header is
 * stored at sector 1. Therefore CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR must be
 * set to 1. Otherwise U-Boot SPL would not be able to load U-Boot proper.
 */
#ifdef CONFIG_SPL_SATA
#if !defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR) || \
    !defined(CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR) || CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR != 1
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

/* Structure of the main header, version 1 (Armada 370/XP/375/38x/39x) */
struct kwbimage_main_hdr_v1 {
	u8  blockid;               /* 0x0       */
	u8  flags;                 /* 0x1       */
	u16 nandpagesize;          /* 0x2-0x3   */
	u32 blocksize;             /* 0x4-0x7   */
	u8  version;               /* 0x8       */
	u8  headersz_msb;          /* 0x9       */
	u16 headersz_lsb;          /* 0xA-0xB   */
	u32 srcaddr;               /* 0xC-0xF   */
	u32 destaddr;              /* 0x10-0x13 */
	u32 execaddr;              /* 0x14-0x17 */
	u8  options;               /* 0x18      */
	u8  nandblocksize;         /* 0x19      */
	u8  nandbadblklocation;    /* 0x1A      */
	u8  reserved4;             /* 0x1B      */
	u16 reserved5;             /* 0x1C-0x1D */
	u8  ext;                   /* 0x1E      */
	u8  checksum;              /* 0x1F      */
} __packed;

#ifdef CONFIG_SPL_MMC
u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	return MMCSD_MODE_RAW;
}
#endif

static u32 checksum32(void *start, u32 len)
{
	u32 csum = 0;
	u32 *p = start;

	while (len > 0) {
		csum += *p++;
		len -= sizeof(u32);
	};

	return csum;
}

int spl_check_board_image(struct spl_image_info *spl_image,
			  const struct spl_boot_device *bootdev)
{
	u32 csum = *(u32 *)(spl_image->load_addr + spl_image->size - 4);

	if (checksum32((void *)spl_image->load_addr,
		       spl_image->size - 4) != csum) {
		printf("ERROR: Invalid data checksum in kwbimage\n");
		return -EINVAL;
	}

	return 0;
}

int spl_parse_board_header(struct spl_image_info *spl_image,
			   const struct spl_boot_device *bootdev,
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
	    ((mhdr->headersz_msb << 16) | mhdr->headersz_lsb) < sizeof(*mhdr)) {
		printf("ERROR: Invalid kwbimage v1\n");
		return -EINVAL;
	}

	if (IS_ENABLED(CONFIG_SPL_SPI_FLASH_SUPPORT) &&
	    bootdev->boot_device == BOOT_DEVICE_SPI &&
	    mhdr->blockid != IBR_HDR_SPI_ID) {
		printf("ERROR: Wrong blockid (0x%x) in SPI kwbimage\n",
		       mhdr->blockid);
		return -EINVAL;
	}

	if (IS_ENABLED(CONFIG_SPL_SATA) &&
	    bootdev->boot_device == BOOT_DEVICE_SATA &&
	    mhdr->blockid != IBR_HDR_SATA_ID) {
		printf("ERROR: Wrong blockid (0x%x) in SATA kwbimage\n",
		       mhdr->blockid);
		return -EINVAL;
	}

	if (IS_ENABLED(CONFIG_SPL_MMC) &&
	    (bootdev->boot_device == BOOT_DEVICE_MMC1 ||
	     bootdev->boot_device == BOOT_DEVICE_MMC2 ||
	     bootdev->boot_device == BOOT_DEVICE_MMC2_2) &&
	    mhdr->blockid != IBR_HDR_SDIO_ID) {
		printf("ERROR: Wrong blockid (0x%x) in SDIO kwbimage\n",
		       mhdr->blockid);
		return -EINVAL;
	}

	spl_image->offset = mhdr->srcaddr;

	/*
	 * For SATA srcaddr is specified in number of sectors.
	 * The main header is must be stored at sector number 1.
	 * This expects that sector size is 512 bytes and recalculates
	 * data offset to bytes relative to the main header.
	 */
	if (IS_ENABLED(CONFIG_SPL_SATA) && mhdr->blockid == IBR_HDR_SATA_ID) {
		if (spl_image->offset < 1) {
			printf("ERROR: Wrong srcaddr (0x%08x) in SATA kwbimage\n",
			       spl_image->offset);
			return -EINVAL;
		}
		spl_image->offset -= 1;
		spl_image->offset *= 512;
	}

	/*
	 * For SDIO (eMMC) srcaddr is specified in number of sectors.
	 * This expects that sector size is 512 bytes and recalculates
	 * data offset to bytes.
	 */
	if (IS_ENABLED(CONFIG_SPL_MMC) && mhdr->blockid == IBR_HDR_SDIO_ID)
		spl_image->offset *= 512;

	if (spl_image->offset % 4 != 0) {
		printf("ERROR: Wrong srcaddr (0x%08x) in kwbimage\n",
		       spl_image->offset);
		return -EINVAL;
	}

	if (mhdr->blocksize <= 4 || mhdr->blocksize % 4 != 0) {
		printf("ERROR: Wrong blocksize (0x%08x) in kwbimage\n",
		       mhdr->blocksize);
		return -EINVAL;
	}

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
#ifdef CONFIG_SPL_MMC
	case BOOT_DEVICE_MMC1:
		return BOOT_DEVICE_MMC1;
#endif
#ifdef CONFIG_SPL_SATA
	case BOOT_DEVICE_SATA:
		return BOOT_DEVICE_SATA;
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

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
	if (spl_boot_list[0] != BOOT_DEVICE_BOOTROM)
		spl_boot_list[1] = BOOT_DEVICE_BOOTROM;
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
	u32 *regs = *(u32 **)(CONFIG_SPL_STACK + 4);

	printf("Returning to BootROM (return address 0x%08x)...\n", regs[13]);
	return_to_bootrom();

	/* NOTREACHED - return_to_bootrom() does not return */
	hang();
}

/*
 * SPI0 CS0 Flash is mapped to address range 0xD4000000 - 0xD7FFFFFF by BootROM.
 * Proper U-Boot removes this direct mapping. So it is available only in SPL.
 */
#if defined(CONFIG_SPL_ENV_IS_IN_SPI_FLASH) && \
    CONFIG_ENV_SPI_BUS == 0 && CONFIG_ENV_SPI_CS == 0 && \
    CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE <= 64*1024*1024
void *env_sf_get_env_addr(void)
{
	return (void *)0xD4000000 + CONFIG_ENV_OFFSET;
}
#endif

void board_init_f(ulong dummy)
{
	int ret;

	/*
	 * Pin muxing needs to be done before UART output, since
	 * on A38x the UART pins need some re-muxing for output
	 * to work.
	 */
	board_early_init_f();

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
		printf("spl_init() failed: %d\n", ret);
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
		printf("ddr3_init() failed: %d\n", ret);
		if (IS_ENABLED(CONFIG_DDR_RESET_ON_TRAINING_FAILURE) &&
		    get_boot_device() != BOOT_DEVICE_UART)
			reset_cpu();
		else
			hang();
	}
#endif

	/* Initialize Auto Voltage Scaling */
	mv_avs_init();

	/* Update read timing control for PCIe */
	mv_rtc_config();
}

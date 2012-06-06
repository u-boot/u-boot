#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_ZED /* Community Board */

#include <configs/zynq_common.h>

/* Default environment */
#define CONFIG_IPADDR   192.168.1.10
/* ETHADDR should pretty much never be in the default env */
#define CONFIG_ETHADDR  00:0a:35:00:01:22
#define CONFIG_SERVERIP 192.168.1.50

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS 	\
	"kernel_size=0x140000\0" 	\
	"ramdisk_size=0x200000\0" 	\
	"qspiboot=sf probe 0 0 0;" \
		"sf read 0x8000 0x100000 0x2c0000; " \
		"sf read 0x1000000 0x3c0000 0x40000; " \
		"sf read 0x800000 0x400000 0x800000; " \
		"go 0x8000\0" \
	"sdboot=echo Copying Linux from SD to RAM...; " \
		"mmcinfo; " \
		"fatload mmc 0 0x8000 zImage; " \
		"fatload mmc 0 0x1000000 devicetree.dtb; " \
		"fatload mmc 0 0x800000 ramdisk8M.image.gz; " \
		"go 0x8000\0" \
	"jtagboot=echo TFTPing Linux to RAM...; " \
		"tftp 0x8000 zImage; " \
		"tftp 0x1000000 devicetree.dtb; " \
		"tftp 0x800000 ramdisk8M.image.gz; " \
		"go 0x8000\0"

#undef CONFIG_PELE_XIL_LQSPI

#undef CONFIG_SYS_NO_FLASH
#define CONFIG_ENV_IS_NOWHERE

#include <config_cmd_default.h>
#define CONFIG_CMD_DATE		/* RTC? */
#define CONFIG_CMD_PING		/* Might be useful for debugging */
#define CONFIG_CMD_SAVEENV	/* Command to save ENV to Flash */
#define CONFIG_REGINFO		/* Again, debugging */
#undef CONFIG_CMD_SETGETDCR	/* README says 4xx only */

#define CONFIG_TIMESTAMP	/* print image timestamp on bootm, etc */

#define CONFIG_PANIC_HANG /* For development/debugging */

#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING

#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT	"zed-boot> "

#ifndef CONFIG_SYS_NO_FLASH

/* FLASH organization */
#define CONFIG_SYS_FLASH_BASE           0xE2000000
#define CONFIG_SYS_FLASH_SIZE           (16 * 1024 * 1024)
#define CONFIG_SYS_MAX_FLASH_BANKS      1
/* max number of sectors/blocks on one chip */
#define CONFIG_SYS_MAX_FLASH_SECT       512
#define CONFIG_SYS_FLASH_ERASE_TOUT     1000
#define CONFIG_SYS_FLASH_WRITE_TOUT     5000

#define CONFIG_FLASH_SHOW_PROGRESS	10

#define CONFIG_SYS_FLASH_CFI
#undef CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_FLASH_CFI_DRIVER

#undef CONFIG_SYS_FLASH_PROTECTION /* don't use hardware protection */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE /* use buffered writes (20x faster) */
#endif

/* HW to use */
#define CONFIG_XDF_UART
#define CONFIG_XDF_ETH
#define CONFIG_XDF_RTC
# define CONFIG_UART1
#define CONFIG_TTC0
#define CONFIG_GEM0
#define CONFIG_XGMAC_PHY_ADDR 0
#define CONFIG_NET_MULTI

/*
 * Physical Memory map
 */
#define PHYS_SDRAM_1_SIZE (256 * 1024 * 1024)

/*
 * SPI Settings
 */
#define CONFIG_ZYNQ_SPI
#define CONFIG_CMD_SPI
#define CONFIG_SF_DEFAULT_SPEED 30000000
#define CONFIG_SPI_FLASH
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH_SPANSION

/* Place a Xilinx Boot ROM header in u-boot image? */
#define CONFIG_PELE_XILINX_FLASH_HEADER

#ifdef CONFIG_PELE_XILINX_FLASH_HEADER
/* Address Xilinx boot rom should use to launch u-boot */
#ifdef CONFIG_PELE_XIL_LQSPI
#define CONFIG_PELE_XIP_START XPSS_QSPI_LIN_BASEADDR
#else
/* NOR */
#define CONFIG_PELE_XIP_START CONFIG_SYS_FLASH_BASE
#endif
#endif

/* Secure Digital */
#define CONFIG_MMC

#ifdef CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_ZYNQ_MMC
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_DOS_PARTITION
#endif

#endif /* __CONFIG_H */

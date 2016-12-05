/*
 * Copyright (C) 2016 Amarula Solutions B.V.
 * Copyright (C) 2016 Engicam S.r.l.
 *
 * Configuration settings for the Engicam i.CoreM6 QDL Starter Kits.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __IMX6QLD_ICORE_CONFIG_H
#define __IMX6QLD_ICORE_CONFIG_H

#include <linux/sizes.h>
#include "mx6_common.h"

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(16 * SZ_1M)

/* Total Size of Environment Sector */
#define CONFIG_ENV_SIZE			SZ_128K

/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Environment */
#ifndef CONFIG_ENV_IS_NOWHERE
/* Environment in MMC */
# if defined(CONFIG_ENV_IS_IN_MMC)
#  define CONFIG_ENV_OFFSET		0x100000
/* Environment in NAND */
# elif defined(CONFIG_ENV_IS_IN_NAND)
#  define CONFIG_ENV_OFFSET		0x400000
#  define CONFIG_ENV_SECT_SIZE		CONFIG_ENV_SIZE
# endif
#endif

/* Default environment */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"splashpos=m,m\0" \
	"image=zImage\0" \
	"console=ttymxc3\0" \
	"fdt_high=0xffffffff\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"fdt_addr=0x18000000\0" \
	"boot_fdt=try\0" \
	"mmcdev=0\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk0p2 rootwait rw\0" \
	"mmcautodetect=yes\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootz; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi\0"

#define CONFIG_BOOTCOMMAND \
	   "mmc dev ${mmcdev};" \
	   "mmc dev ${mmcdev}; if mmc rescan; then " \
		   "if run loadbootscript; then " \
			   "run bootscript; " \
		   "else " \
			"if run loadimage; then " \
				"run mmcboot; " \
			"fi; " \
		   "fi; " \
	   "fi"

/* Miscellaneous configurable options */
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x8000000)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					CONFIG_SYS_INIT_SP_OFFSET)

/* UART */
#ifdef CONFIG_MXC_UART
# define CONFIG_MXC_UART_BASE		UART4_BASE
#endif

/* MMC */
#ifdef CONFIG_FSL_USDHC
# define CONFIG_SYS_MMC_ENV_DEV		0
# define CONFIG_SYS_FSL_USDHC_NUM	1
# define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR
#endif

/* NAND */
#ifdef CONFIG_NAND_MXS
# define CONFIG_SYS_MAX_NAND_DEVICE	1
# define CONFIG_SYS_NAND_BASE		0x40000000
# define CONFIG_SYS_NAND_5_ADDR_CYCLE
# define CONFIG_SYS_NAND_ONFI_DETECTION
# define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
# define CONFIG_SYS_NAND_U_BOOT_OFFS	0x200000

/* MTD device */
# define CONFIG_MTD_DEVICE
# define CONFIG_CMD_MTDPARTS
# define CONFIG_MTD_PARTITIONS
# define MTDIDS_DEFAULT			"nand0=nand"
# define MTDPARTS_DEFAULT		"mtdparts=nand:2m(spl),2m(uboot)," \
					"1m(env),4m(kernel),1m(dtb),-(rootfs)"

# define CONFIG_APBH_DMA
# define CONFIG_APBH_DMA_BURST
# define CONFIG_APBH_DMA_BURST8
#endif

/* Ethernet */
#ifdef CONFIG_FEC_MXC
# define IMX_FEC_BASE			ENET_BASE_ADDR
# define CONFIG_FEC_MXC_PHYADDR		0
# define CONFIG_FEC_XCV_TYPE		RMII
# define CONFIG_ETHPRIME		"FEC"

# define CONFIG_MII
# define CONFIG_PHYLIB
# define CONFIG_PHY_SMSC
#endif

/* Framebuffer */
#ifdef CONFIG_VIDEO_IPUV3
# define CONFIG_IPUV3_CLK		260000000
# define CONFIG_IMX_VIDEO_SKIP

# define CONFIG_SPLASH_SCREEN
# define CONFIG_SPLASH_SCREEN_ALIGN
# define CONFIG_BMP_16BPP
# define CONFIG_VIDEO_BMP_RLE8
# define CONFIG_VIDEO_LOGO
# define CONFIG_VIDEO_BMP_LOGO
#endif

/* SPL */
#ifdef CONFIG_SPL
# ifdef CONFIG_NAND_MXS
#  define CONFIG_SPL_NAND_SUPPORT
# else
#  define CONFIG_SPL_MMC_SUPPORT
# endif

# include "imx6_spl.h"
# ifdef CONFIG_SPL_BUILD
#  undef CONFIG_DM_GPIO
#  undef CONFIG_DM_MMC
# endif
#endif

#endif /* __IMX6QLD_ICORE_CONFIG_H */

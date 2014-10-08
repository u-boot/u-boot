/*
 * Copyright (C) 2010-2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2014 Bachmann electronic GmbH
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"
#define CONFIG_MX6
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#include <asm/arch/imx-regs.h>
#include <asm/imx-common/gpio.h>

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SYS_GENERIC_BOARD

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN           (10 * 1024 * 1024)

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_MISC_INIT_R
#define CONFIG_MXC_GPIO

/* FUSE Configs */
#define CONFIG_CMD_FUSE
#define CONFIG_MXC_OCOTP

/* UART Configs */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE           UART1_BASE

/* SF Configs */
#define CONFIG_CMD_SF
#define CONFIG_SPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SPI_FLASH_SST
#define CONFIG_MXC_SPI
#define CONFIG_SF_DEFAULT_BUS  2
#define CONFIG_SF_DEFAULT_CS   (0|(IMX_GPIO_NR(1, 3)<<8))
#define CONFIG_SF_DEFAULT_SPEED 25000000
#define CONFIG_SF_DEFAULT_MODE (SPI_MODE_0)

/* IO expander */
#define CONFIG_PCA953X
#define CONFIG_SYS_I2C_PCA953X_ADDR	0x20
#define CONFIG_SYS_I2C_PCA953X_WIDTH	{ {0x20, 16} }
#define CONFIG_CMD_PCA953X
#define CONFIG_CMD_PCA953X_INFO

/* I2C Configs */
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_SPEED            100000

/* OCOTP Configs */
#define CONFIG_CMD_IMXOTP
#define CONFIG_IMX_OTP
#define IMX_OTP_BASE                    OCOTP_BASE_ADDR
#define IMX_OTP_ADDR_MAX                0x7F
#define IMX_OTP_DATA_ERROR_VAL          0xBADABADA
#define IMX_OTPWRITE_ENABLED

/* MMC Configs */
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR      0
#define CONFIG_SYS_FSL_USDHC_NUM       2

#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_BOUNCE_BUFFER

#ifdef CONFIG_MX6Q
#define CONFIG_CMD_SATA
#endif

/*
 * SATA Configs
 */
#ifdef CONFIG_CMD_SATA
#define CONFIG_DWC_AHSATA
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_ARB_BASE_ADDR
#define CONFIG_LBA48
#define CONFIG_LIBATA
#endif


#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_FEC_MXC
#define CONFIG_MII
#define IMX_FEC_BASE                    ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE             MII100
#define CONFIG_ETHPRIME                 "FEC"
#define CONFIG_FEC_MXC_PHYADDR          0x5
#define CONFIG_PHYLIB
#define CONFIG_PHY_SMSC

/* Miscellaneous commands */
#define CONFIG_CMD_BMODE
#define CONFIG_CMD_SETEXPR

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX              1
#define CONFIG_BAUDRATE                115200

/* Command definition */
#include <config_cmd_default.h>

#undef CONFIG_CMD_IMLS

#define CONFIG_BOOTDELAY               2

#define CONFIG_PREBOOT                 ""

#define CONFIG_LOADADDR                0x12000000
#define CONFIG_SYS_TEXT_BASE           0x17800000

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_CBSIZE              1024

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS             16
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

#define CONFIG_SYS_LOAD_ADDR           CONFIG_LOADADDR

#define CONFIG_CMDLINE_EDITING

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS           1
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_SIZE                (1u * 1024 * 1024 * 1024)

#define CONFIG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SIZE                 (64 * 1024)	/* 64 kb */
#define CONFIG_ENV_OFFSET               (1024 * 1024)
/* M25P16 has an erase size of 64 KiB */
#define CONFIG_ENV_SECT_SIZE            (64 * 1024)
#define CONFIG_ENV_SPI_BUS              CONFIG_SF_DEFAULT_BUS
#define CONFIG_ENV_SPI_CS               CONFIG_SF_DEFAULT_CS
#define CONFIG_ENV_SPI_MODE             CONFIG_SF_DEFAULT_MODE
#define CONFIG_ENV_SPI_MAX_HZ           CONFIG_SF_DEFAULT_SPEED

#define CONFIG_OF_LIBFDT
#define CONFIG_CMD_BOOTZ

#ifndef CONFIG_SYS_DCACHE_OFF
#define CONFIG_CMD_CACHE
#endif

#define CONFIG_CMD_BOOTZ
#define CONFIG_SUPPORT_RAW_INITRD

/* FS Configs */
#define CONFIG_CMD_EXT3
#define CONFIG_CMD_EXT4
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_FS_GENERIC

#define CONFIG_BOOTP_SERVERIP
#define CONFIG_BOOTP_BOOTFILE

#endif         /* __CONFIG_H */

/*
 * (C) Copyright 2011, Stefano Babic <sbabic@denx.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
 *
 * Configuration for the woodburn board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __WOODBURN_COMMON_CONFIG_H
#define __WOODBURN_COMMON_CONFIG_H

#include <asm/arch/imx-regs.h>

 /* High Level Configuration Options */
#define CONFIG_MX35
#define CONFIG_MX35_HCLK_FREQ	24000000
#define CONFIG_SYS_FSL_CLK

#define CONFIG_SYS_DCACHE_OFF

#define CONFIG_MACH_TYPE		MACH_TYPE_FLEA3

/* This is required to setup the ESDC controller */

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_REVISION_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 1024 * 1024)

/*
 * Hardware drivers
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_SYS_SPD_BUS_NUM		0
#define CONFIG_MXC_SPI
#define CONFIG_MXC_GPIO

/* PMIC Controller */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_FSL
#define CONFIG_POWER_FSL_MC13892
#define CONFIG_SYS_FSL_PMIC_I2C_ADDR	0x8
#define CONFIG_RTC_MC13XXX

/* mmc driver */
#define CONFIG_FSL_ESDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_ESDHC_NUM	1

/*
 * UART (console)
 */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART1_BASE

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX	1

/*
 * Command definition
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_DNS

#define CONFIG_MXC_GPIO

#define CONFIG_NET_RETRY_COUNT	100


#define CONFIG_LOADADDR		0x80800000	/* loadaddr env var */

/*
 * Ethernet on SOC (FEC)
 */
#define CONFIG_FEC_MXC
#define IMX_FEC_BASE	FEC_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR	0x1

#define CONFIG_MII
#define CONFIG_DISCOVER_PHY

#define CONFIG_ARP_TIMEOUT	200UL

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory */
#define CONFIG_CMDLINE_EDITING

#define CONFIG_AUTO_COMPLETE

#define CONFIG_SYS_MEMTEST_START	0	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x10000

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE	(256 * 1024 * 1024)

#define CONFIG_SYS_SDRAM_BASE		CSD0_BASE_ADDR

#define CONFIG_SYS_GBL_DATA_OFFSET	(LOW_LEVEL_SRAM_STACK - \
						IRAM_BASE_ADDR - \
						GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR		(IRAM_BASE_ADDR + \
					CONFIG_SYS_GBL_DATA_OFFSET)

/*
 * MTD Command for mtdparts
 */
#define CONFIG_MTD_DEVICE
#define CONFIG_FLASH_CFI_MTD
#define CONFIG_MTD_PARTITIONS

/*
 * FLASH and environment organization
 */
#define CONFIG_SYS_FLASH_BASE		CS0_BASE_ADDR
#define CONFIG_SYS_MAX_FLASH_BANKS 1	/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT 512	/* max number of sectors on one chip */
/* Monitor at beginning of flash */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)

#define CONFIG_ENV_SECT_SIZE	(128 * 1024)
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE

#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + \
				CONFIG_SYS_MONITOR_LEN)

/*
 * CFI FLASH driver setup
 */
#define CONFIG_SYS_FLASH_CFI		/* Flash memory is CFI compliant */
#define CONFIG_FLASH_CFI_DRIVER

/* A non-standard buffered write algorithm */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	/* faster */
#define CONFIG_SYS_FLASH_PROTECTION	/* Use hardware sector protection */

/*
 * NAND FLASH driver setup
 */
#define CONFIG_NAND_MXC_V1_1
#define CONFIG_MXC_NAND_REGS_BASE	(NFC_BASE_ADDR)
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		(NFC_BASE_ADDR)
#define CONFIG_MXC_NAND_HWECC
#define CONFIG_SYS_NAND_LARGEPAGE

#define CONFIG_SYS_NAND_ONFI_DETECTION

/*
 * Default environment and default scripts
 * to update uboot and load kernel
 */

#define CONFIG_HOSTNAME woodburn
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip_sta=setenv bootargs ${bootargs} "			\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addip_dyn=setenv bootargs ${bootargs} ip=dhcp\0"		\
	"addip=if test -n ${ipdyn};then run addip_dyn;"			\
		"else run addip_sta;fi\0"	\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addtty=setenv bootargs ${bootargs}"				\
		" console=ttymxc0,${baudrate}\0"			\
	"addmisc=setenv bootargs ${bootargs} ${misc}\0"			\
	"loadaddr=80800000\0"						\
	"kernel_addr_r=80800000\0"					\
	"hostname=" __stringify(CONFIG_HOSTNAME) "\0"			\
	"bootfile=" __stringify(CONFIG_HOSTNAME) "/uImage\0"		\
	"ramdisk_file=" __stringify(CONFIG_HOSTNAME) "/uRamdisk\0"	\
	"flash_self=run ramargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"flash_nfs=run nfsargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr}\0"				\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile}; "			\
		"run nfsargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr_r}\0"				\
	"net_self_load=tftp ${kernel_addr_r} ${bootfile};"		\
		"tftp ${ramdisk_addr_r} ${ramdisk_file};\0"		\
	"net_self=if run net_self_load;then "				\
		"run ramargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr_r} ${ramdisk_addr_r};"		\
		"else echo Images not loades;fi\0"			\
	"u-boot=" __stringify(CONFIG_HOSTNAME) "/u-boot.bin\0"		\
	"load=tftp ${loadaddr} ${u-boot}\0"				\
	"uboot_addr=" __stringify(CONFIG_SYS_MONITOR_BASE) "\0"		\
	"update=protect off ${uboot_addr} +80000;"			\
		"erase ${uboot_addr} +80000;"				\
		"cp.b ${loadaddr} ${uboot_addr} ${filesize}\0"		\
	"upd=if run load;then echo Updating u-boot;if run update;"	\
		"then echo U-Boot updated;"				\
			"else echo Error updating u-boot !;"		\
			"echo Board without bootloader !!;"		\
		"fi;"							\
		"else echo U-Boot not downloaded..exiting;fi\0"		\
	"bootcmd=run net_nfs\0"

#endif				/* __CONFIG_H */

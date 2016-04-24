/*
 * Copyright (C) 2013 Stefan Roese <sr@denx.de>
 *
 * Configuration settings for the ProjectionDesign / Barco
 * Titanium board.
 *
 * Based on mx6qsabrelite.h which is:
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

#define CONFIG_MX6Q

#define MACH_TYPE_TITANIUM		3769
#define CONFIG_MACH_TYPE		MACH_TYPE_TITANIUM

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(2 * 1024 * 1024)

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_MISC_INIT_R

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART1_BASE

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_SYS_I2C_SPEED		100000

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_USDHC_NUM	1

#define CONFIG_FEC_MXC
#define CONFIG_MII
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_FEC_MXC_PHYADDR		4
#define CONFIG_PHYLIB
#define CONFIG_PHY_MICREL
#define CONFIG_PHY_MICREL_KSZ9021

/* USB Configs */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX6
#define CONFIG_USB_STORAGE
#define CONFIG_MXC_USB_PORT	1
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

/* Miscellaneous commands */
#define CONFIG_CMD_BMODE

#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + (500 << 20))

#define CONFIG_HOSTNAME			titanium
#define CONFIG_UBI_PART			ubi
#define CONFIG_UBIFS_VOLUME		rootfs0

#define MTDIDS_DEFAULT		"nand0=gpmi-nand"
#define MTDPARTS_DEFAULT	"mtdparts=gpmi-nand:16M(uboot),512k(env1)," \
				"512k(env2),-(ubi)"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"kernel=" __stringify(CONFIG_HOSTNAME) "/uImage\0"		\
	"kernel_fs=/boot/uImage\0"					\
	"kernel_addr=11000000\0"					\
	"dtb=" __stringify(CONFIG_HOSTNAME) "/"				\
		__stringify(CONFIG_HOSTNAME) ".dtb\0"			\
	"dtb_fs=/boot/" __stringify(CONFIG_HOSTNAME) ".dtb\0"		\
	"dtb_addr=12800000\0"						\
	"script=boot.scr\0" \
	"uimage=uImage\0" \
	"console=ttymxc0\0" \
	"baudrate=115200\0" \
	"fdt_high=0xffffffff\0"	  \
	"initrd_high=0xffffffff\0" \
	"mmcdev=0\0" \
	"mmcpart=1\0" \
	"uimage=uImage\0" \
	"loadbootscript=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr}" \
		" ${script}\0" \
	"bootscript=echo Running bootscript from mmc ...; source\0" \
	"loaduimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${uimage}\0" \
	"mmcroot=/dev/mmcblk0p2\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot} rootwait rw\0" \
	"bootmmc=run mmcargs; fatload mmc ${mmcdev}:${mmcpart} ${loadaddr}" \
		" ${uimage}; bootm\0" \
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addcon=setenv bootargs ${bootargs} console=ttymxc0,"		\
		"${baudrate}\0"						\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"rootpath=/opt/eldk-5.3/armv7a/rootfs-minimal-mtdutils\0"	\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ubifs=" __stringify(CONFIG_HOSTNAME) "/ubifs.img\0"		\
	"part=" __stringify(CONFIG_UBI_PART) "\0"			\
	"boot_vol=0\0"							\
	"vol=" __stringify(CONFIG_UBIFS_VOLUME) "\0"			\
	"load_ubifs=tftp ${kernel_addr} ${ubifs}\0"			\
	"update_ubifs=ubi part ${part};ubi write ${kernel_addr} ${vol}"	\
		" ${filesize}\0"					\
	"upd_ubifs=run load_ubifs update_ubifs\0"			\
	"init_ubi=nand erase.part ubi;ubi part ${part};"		\
		"ubi create ${vol} c800000\0"				\
	"mtdids=" MTDIDS_DEFAULT "\0"					\
	"mtdparts=" MTDPARTS_DEFAULT "\0"				\
	"nand_ubifs=run ubifs_mount ubifs_load ubifsargs addip"		\
		" addcon addmtd;"					\
		"bootm ${kernel_addr} - ${dtb_addr}\0"			\
	"ubifsargs=set bootargs ubi.mtd=ubi "				\
		"root=ubi:rootfs${boot_vol} rootfstype=ubifs\0"		\
	"ubifs_mount=ubi part ubi;ubifsmount ubi:rootfs${boot_vol}\0"	\
	"ubifs_load=ubifsload ${kernel_addr} ${kernel_fs};"		\
		"ubifsload ${dtb_addr} ${dtb_fs};\0"			\
	"nand_ubifs=run ubifs_mount ubifs_load ubifsargs addip addcon "	\
		"addmtd;bootm ${kernel_addr} - ${dtb_addr}\0"		\
	"load_kernel=tftp ${kernel_addr} ${kernel}\0"			\
	"load_dtb=tftp ${dtb_addr} ${dtb}\0"				\
	"net_nfs=run load_dtb load_kernel; "				\
		"run nfsargs addip addcon addmtd;"			\
		"bootm ${kernel_addr} - ${dtb_addr}\0"			\
	"delenv=env default -a -f; saveenv; reset\0"

#define CONFIG_BOOTCOMMAND		"run nand_ubifs"

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					 sizeof(CONFIG_SYS_PROMPT) + 16)

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_SIZE			(512 << 20)

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Enable NAND support */
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NAND_TRIMFFS

#ifdef CONFIG_CMD_NAND

/* NAND stuff */
#define CONFIG_NAND_MXS
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION

/* DMA stuff, needed for GPMI/MXS NAND support */
#define CONFIG_APBH_DMA
#define CONFIG_APBH_DMA_BURST
#define CONFIG_APBH_DMA_BURST8

/* Environment in NAND */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		(16 << 20)
#define CONFIG_ENV_SECT_SIZE		(128 << 10)
#define CONFIG_ENV_SIZE			CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + (512 << 10))
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE

#else /* CONFIG_CMD_NAND */

/* Environment in MMC */
#define CONFIG_ENV_SIZE			(8 << 10)
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_ENV_OFFSET		(6 * 64 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV		0

#endif /* CONFIG_CMD_NAND */

/* UBI/UBIFS config options */
#define CONFIG_LZO
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_RBTREE
#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS

#endif			       /* __CONFIG_H */

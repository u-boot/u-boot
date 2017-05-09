/*
 * Copyright (C) 2017 Logic PD, Inc.
 *
 * Configuration settings for the LogicPD i.MX6 SOM.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#ifndef __IMX6LOGIC_CONFIG_H
#define __IMX6LOGIC_CONFIG_H

#define CONFIG_MXC_UART_BASE   UART1_BASE
#define CONSOLE_DEV            "ttymxc0"

#include <config_distro_defaults.h>
#include "mx6_common.h"

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN          (10 * SZ_1M)

#define CONFIG_MXC_UART

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR      0
#define CONFIG_SYS_FSL_USDHC_NUM       2
#define CONFIG_MMCROOT         "/dev/mmcblk1p2" /* Dev kit SD card */

/* Ethernet Configs */
#define CONFIG_MII
#define CONFIG_FEC_XCV_TYPE            RMII
#define CONFIG_ETHPRIME                "FEC"
#define CONFIG_FEC_MXC_PHYADDR         0
#define CONFIG_PHYLIB
#define CONFIG_PHY_SMSC

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"image=zImage\0" \
	"bootm_size=0x10000000\0" \
	"fdt_addr_r=0x18000000\0" \
	"fdt_addr=0x18000000\0" \
	"ramdisk_addr_r=0x13000000\0" \
	"ramdiskaddr=0x13000000\0" \
	"kernel_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	"ramdisk_file=rootfs.cpio.uboot\0" \
	"boot_fdt=try\0" \
	"ip_dyn=yes\0" \
	"console=" CONSOLE_DEV "\0" \
	"mmcdev=1\0" \
	"mmcpart=1\0" \
	"mmcroot=" CONFIG_MMCROOT " rootwait rw\0" \
	"nandroot=ubi0:rootfs rootfstype=ubifs\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate}" \
	" root=${mmcroot} ${mtdparts}\0" \
	"nandargs=setenv bootargs console=${console},${baudrate}" \
	" ubi.mtd=fs root=${nandroot} ${mtdparts}\0" \
	"ramargs=setenv bootargs console=${console},${baudrate}" \
	" root=/dev/ram rw ${mtdparts}\0"                    \
	"loadbootscript=" \
	"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...;" \
	" source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image};" \
	" setenv kernelsize ${filesize}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"loadramdisk=fatload mmc ${mmcdev}:${mmcpart} ${ramdiskaddr}" \
	" ${ramdisk_file}; setenv ramdisksize ${filesize}\0" \
	"mmcboot=echo Booting from mmc...; run mmcargs; run loadimage;" \
	" run loadfdt; bootz ${loadaddr} - ${fdt_addr}\0" \
	"mmcramboot=run ramargs; run loadimage;" \
	" run loadfdt; run loadramdisk;" \
	" bootz ${loadaddr} ${ramdiskaddr} ${fdt_addr}\0" \
	"nandboot=echo Booting from nand ...; " \
	" run nandargs;" \
	" nand read ${loadaddr} kernel ${kernelsize};" \
	" nand read ${fdt_addr} dtb;" \
	" bootz ${loadaddr} - ${fdt_addr}\0" \
	"nandramboot=echo Booting RAMdisk from nand ...; " \
	" nand read ${ramdiskaddr} fs ${ramdisksize};" \
	" nand read ${loadaddr} kernel ${kernelsize};" \
	" nand read ${fdt_addr} dtb;" \
	" run ramargs;" \
	" bootz ${loadaddr} ${ramdiskaddr} ${fdt_addr}\0" \
	"netargs=setenv bootargs console=${console},${baudrate} " \
	"root=/dev/nfs" \
	" ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
	"netboot=echo Booting from net ...; " \
	"run netargs; " \
	"if test ${ip_dyn} = yes; then " \
		"setenv get_cmd dhcp; " \
	"else " \
		"setenv get_cmd tftp; " \
	"fi; " \
	"${get_cmd} ${image}; " \
	"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
		"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
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
	"fi;\0" \
	"autoboot=mmc dev ${mmcdev};" \
	"if mmc rescan; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
		"if run loadimage; then " \
			"run mmcboot; " \
		"else run netboot; " \
		"fi; " \
	"fi; " \
	"else run netboot; fi"
#define CONFIG_BOOTCOMMAND \
	"run autoboot"

#define CONFIG_ARP_TIMEOUT     200UL

#define CONFIG_SYS_MEMTEST_START       0x10000000
#define CONFIG_SYS_MEMTEST_END         0x10010000
#define CONFIG_SYS_MEMTEST_SCRATCH     0x10800000

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS           1
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR
#define CONFIG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Environment organization */
#define CONFIG_ENV_SIZE                        (8 * 1024)
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET             0x400000
#define CONFIG_ENV_SECT_SIZE          CONFIG_ENV_SIZE

/* NAND stuff */
#define CONFIG_CMD_NAND_TRIMFFS
#define CONFIG_SYS_MAX_NAND_DEVICE     1
#define CONFIG_SYS_NAND_BASE           0x40000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x200000

/* MTD device */
# define CONFIG_MTD_DEVICE
# define CONFIG_CMD_MTDPARTS
# define CONFIG_MTD_PARTITIONS
# define MTDIDS_DEFAULT		"nand0=gpmi-nand"
# define MTDPARTS_DEFAULT	"mtdparts=gpmi-nand:4m(uboot)," \
					"1m(env),16m(kernel),1m(dtb),-(fs)"

/* DMA stuff, needed for GPMI/MXS NAND support */
#define CONFIG_APBH_DMA
#define CONFIG_APBH_DMA_BURST
#define CONFIG_APBH_DMA_BURST8

/* EEPROM  contains serial no, MAC addr and other Logic PD info */
#define CONFIG_I2C_EEPROM

#endif                         /* __IMX6LOGIC_CONFIG_H */

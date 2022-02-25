/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 * Copyright (C) 2018 DENX Software Engineering
 * Måns Rullgård, DENX Software Engineering, mans@mansr.com
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */
#ifndef __CONFIGS_XEA_H__
#define __CONFIGS_XEA_H__

#include <linux/sizes.h>

/* SPL */
#define CONFIG_SPL_STACK		0x20000

#define CONFIG_SYS_SPL_ARGS_ADDR	0x44000000

#define CONFIG_SYS_SPI_KERNEL_OFFS	SZ_1M
#define CONFIG_SYS_SPI_ARGS_OFFS	SZ_512K
#define CONFIG_SYS_SPI_ARGS_SIZE	SZ_32K

#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	(SZ_512K / 0x200)
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	(SZ_32K / 0x200)

/* Memory configuration */
#define PHYS_SDRAM_1			0x40000000	/* Base address */
#define PHYS_SDRAM_1_SIZE		0x10000000	/* Max 256 MB RAM */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* Extra Environment */
#define CONFIG_HOSTNAME		"xea"

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"bootmode=update\0"						\
	"bootpri=mmc_mmc\0"						\
	"bootsec=sf_swu\0"						\
	"consdev=ttyAMA0\0"						\
	"baudrate=115200\0"						\
	"dtbaddr=0x44000000\0"						\
	"dtbfile=imx28-xea.dtb\0"					\
	"rootdev=/dev/mmcblk0p2\0"					\
	"netdev=eth0\0"							\
	"rdaddr=0x43000000\0"						\
	"swufile=swupdate.img\0"					\
	"sf_kernel_offset=0x100000\0"					\
	"sf_kernel_size=0x400000\0"					\
	"sf_swu_offset=0x500000\0"					\
	"sf_swu_size=0x800000\0"					\
	"rootpath=/opt/eldk-5.5/armv5te/rootfs-qte-sdk\0"		\
	"do_update_mmc="						\
		"if mmc rescan ; then "					\
		"mmc dev 0 ${update_mmc_part} ; "			\
		"if dhcp ${hostname}/${update_filename} ; then "	\
		"setexpr fw_sz ${filesize} / 0x200 ; "	/* SD block size */ \
		"setexpr fw_sz ${fw_sz} + 1 ; "				\
		"mmc write ${loadaddr} ${update_offset} ${fw_sz} ; "	\
		"fi ; "							\
		"fi\0"							\
	"do_update_sf="							\
		"if sf probe ; then "					\
		"if dhcp ${hostname}/${update_filename} ; then "	\
		"sf erase ${update_offset} +${filesize} ; "		\
		"sf write ${loadaddr} ${update_offset} ${filesize} ; "	\
		"fi ; "							\
		"fi\0"							\
	"update_spl_filename=u-boot.sb\0"				\
	"update_spl="							\
		"setenv update_filename ${update_spl_filename} ; "	\
		"setenv update_offset 0 ; "				\
		"run do_update_sf\0"					\
	"update_uboot_filename=u-boot.img\0"				\
	"update_uboot="							\
		"setenv update_filename ${update_uboot_filename} ; "	\
		"setenv update_offset 0x10000 ; "			\
		"run do_update_sf ; "					\
		"setenv update_mmc_part 1 ; "				\
		"setenv update_offset 0 ; "				\
		"run do_update_mmc\0"					\
	"update_kernel_filename=uImage\0"				\
	"update_kernel="						\
		"setenv update_mmc_part 1 ; "				\
		"setenv update_filename ${update_kernel_filename} ; "	\
		"setenv update_offset 0x800 ; "				\
		"run do_update_mmc ; "					\
		"setenv update_filename ${dtbfile} ; "			\
		"setenv update_offset 0x400 ; "				\
		"run do_update_mmc\0"					\
	"update_sfkernel="						\
		"setenv update_filename fitImage ; "			\
		"setenv update_offset ${sf_kernel_offset} ; "		\
		"run do_update_sf\0"					\
	"update_swu="							\
		"setenv update_filename ${swufile} ; "			\
		"setenv update_offset ${sf_swu_offset} ; "		\
		"run do_update_sf\0"					\
	"addcons="							\
		"setenv bootargs ${bootargs} "				\
		"console=${consdev},${baudrate}\0"			\
	"addip="							\
		"setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:"		\
			"${netmask}:${hostname}:${netdev}:off\0"	\
	"addmisc="							\
		"setenv bootargs ${bootargs} ${miscargs}\0"		\
	"addargs=run addcons addmisc\0"					\
	"mmcload="							\
		"mmc rescan ; "						\
		"mmc dev 0 1 ; "					\
		"mmc read ${loadaddr} 0x800 0x2000 ; "			\
		"mmc read ${dtbaddr} 0x400 0x80\0"			\
	"netload="							\
		"dhcp ${loadaddr} ${hostname}/${bootfile} ; "		\
		"tftp ${dtbaddr} ${hostname}/${dtbfile}\0"		\
	"sfload="							\
		"sf probe ; "						\
		"sf read ${loadaddr} ${sf_kernel_offset} ${sf_kernel_size}\0" \
	"usbload="							\
		"usb start ; "						\
		"load usb 0:1 ${loadaddr} ${bootfile}\0"		\
	"miscargs=panic=1\0"						\
	"mmcargs=setenv bootargs root=${rootdev} rw rootwait\0"		\
	"nfsargs="							\
		"setenv bootargs root=/dev/nfs rw "			\
			"nfsroot=${serverip}:${rootpath},v3,tcp\0"	\
	"mmc_mmc="							\
		"if run mmcload mmcargs addargs ; then "		\
		"bootm ${loadaddr} - ${dtbaddr} ; "			\
		"fi\0"							\
	"mmc_nfs="							\
		"if run mmcload nfsargs addip addargs ; then "		\
		"bootm ${loadaddr} - ${dtbaddr} ; "			\
		"fi\0"							\
	"sf_mmc="							\
		"if run sfload mmcargs addargs ; then "			\
		"bootm ${loadaddr} - ${dtbaddr} ; "			\
		"fi\0"							\
	"sf_swu="							\
		"if run sfload ; then "					\
		"sf read ${rdaddr} ${sf_swu_offset} ${sf_swu_size} ; "	\
		"setenv bootargs root=/dev/ram0 rw ; "			\
		"run addargs ; "					\
		"bootm ${loadaddr} ${rdaddr} ; "		\
		"fi\0"							\
	"net_mmc="							\
		"if run netload mmcargs addargs ; then "		\
		"bootm ${loadaddr} - ${dtbaddr} ; "			\
		"fi\0"							\
	"net_nfs="							\
		"if run netload nfsargs addip addargs ; then "		\
		"bootm ${loadaddr} - ${dtbaddr} ; "			\
		"fi\0"							\
	"prebootcmd="							\
		"if test \"${envsaved}\" != y ; then ; "		\
		"setenv envsaved y ; "					\
		"saveenv ; "						\
		"fi ; "							\
		"if test \"${bootmode}\" = normal ; then "		\
		"setenv bootdelay 0 ; "					\
		"setenv bootpri mmc_mmc ; "				\
		"elif test \"${bootmode}\" = devel ; then "		\
		"setenv bootdelay 3 ; "					\
		"setenv bootpri net_mmc ; "				\
		"else "							\
		"if test \"${bootmode}\" != update ; then "		\
		"echo Warning: unknown bootmode \"${bootmode}\" ; "	\
		"fi ; "							\
		"setenv bootdelay 1 ; "					\
		"setenv bootpri sf_swu ; "				\
		"fi\0"

/* The rest of the configuration is shared */
#include <configs/mxs.h>

#endif /* __CONFIGS_XEA_H__ */

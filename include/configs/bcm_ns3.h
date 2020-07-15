/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom.
 *
 */

#ifndef __BCM_NS3_H
#define __BCM_NS3_H

#include <linux/sizes.h>

#define CONFIG_HOSTNAME			"NS3"

/* Physical Memory Map */
#define V2M_BASE			0x80000000
#define PHYS_SDRAM_1			V2M_BASE

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_LOAD_ADDR		(PHYS_SDRAM_1 + 0x80000)

/*
 * Initial SP before reloaction is placed at end of first DRAM bank,
 * which is 0x1_0000_0000.
 * Just before re-loaction, new SP is updated and re-location happens.
 * So pointing the initial SP to end of 2GB DDR is not a problem
 */
#define CONFIG_SYS_INIT_SP_ADDR		(PHYS_SDRAM_1 + 0x80000000)
/* 12MB Malloc size */
#define CONFIG_SYS_MALLOC_LEN		(SZ_8M + SZ_4M)

/* console configuration */
#define CONFIG_SYS_NS16550_CLK		25000000

#define CONFIG_SYS_CBSIZE		SZ_1K
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					 sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/*
 * Increase max uncompressed/gunzip size, keeping size same as EMMC linux
 * partition.
 */
#define CONFIG_SYS_BOOTM_LEN		0x01800000

/* Env configuration */
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_ENV_PART		0

/* Access eMMC Boot_1 and Boot_2 partitions */
#define CONFIG_SUPPORT_EMMC_BOOT

/* enable 64-bit PCI resources */
#define CONFIG_SYS_PCI_64BIT		1

#define CONSOLE_ARGS "console_args=console=ttyS0,115200n8\0"
#define MAX_CPUS "max_cpus=maxcpus=8\0"
#define OS_LOG_LEVEL "log_level=loglevel=7\0"
#define EXTRA_ARGS "extra_args=earlycon=uart8250,mmio32,0x68A10000 " \
		   "earlyelog=" __stringify(ELOG_AP_UART_LOG_BASE) ",0x10000 " \
		   "crashkernel=512M reboot=w\0"

#define PCIE_ARGS "pcie_args=pci=pcie_bus_safe pcie_ports=native vfio_pci.disable_idle_d3=1\0"

#ifdef CONFIG_BCM_SF2_ETH
#define ETH_ADDR "ethaddr=00:0A:F7:95:65:A4\0"
#define NET_ARGS "bgmac_platform.ethaddr=${ethaddr} " \
	"ip=${ipaddr}::${gatewayip}:${netmask}::${ethif}:off"
#else
#define ETH_ADDR
#define NET_ARGS
#endif

#define RESERVED_MEM "reserved_mem=memmap=0xff000000$0x1000000\0"

#define BASE_ARGS "${console_args} ${extra_args} ${pcie_args}" \
		  " ${max_cpus}  ${log_level} ${reserved_mem}"
#define SETBOOTARGS "setbootargs=setenv bootargs " BASE_ARGS " " NET_ARGS "\0"

#define UPDATEME_FLASH_PARAMS "bcm_compat_level=4\0" \
			      "bcm_need_recovery_rootfs=0\0" \
			      "bcm_bl_flash_pending_rfs_imgs=0\0"

#define KERNEL_LOADADDR_CFG \
	"fit_image_loadaddr=0x90000000\0" \
	"dtb_loadaddr=0x82000000\0"

#define INITRD_ARGS "initrd_args=root=/dev/ram rw\0"
#define INITRD_LOADADDR "initrd_loadaddr=0x92000000\0"
#define INITRD_IMAGE "initrd_image=rootfs-lake-bcm958742t.cpio.gz\0"
#define MMC_DEV "sd_device_number=0\0"
#define EXEC_STATE "exec_state=normal\0"

#define EXT4RD_ARGS "ext4rd_args="\
	"root=/dev/mmcblk${sd_device_number}p${gpt_partition_entry} rw rootwait\0"

#define WDT_CNTRL "wdt_enable=1\0" \
		  "wdt_timeout_sec=0\0"

#define ELOG_SETUP \
	"mbox0_addr=0x66424024\0"\
	"elog_setup="\
	"if logsetup -s ${mbox0_addr}; then "\
	"else "\
		"echo ELOG is not supported by this version of the MCU patch.;"\
		"exit;"\
	"fi;"\
	"if logsetup -c ${mbox0_addr}; then "\
		"echo ELOG is ready;"\
	"else "\
		"echo ELOG is supported, but is not set up.;"\
		"echo Getting setup file from the server ${serverip}...;"\
		"if tftp ${tftp_dir}elog_src.txt; then "\
			"echo Setting up ELOG. Please wait...;"\
			"if logsetup ${loadaddr} ${mbox0_addr} ${filesize}; "\
				"then "\
			"else "\
				"echo [logsetup] ERROR.;"\
			"fi;"\
			"if logsetup -c ${mbox0_addr}; then "\
				"echo ELOG is READY.;"\
			"else "\
				"echo ELOG is NOT SET UP.;"\
			"fi;"\
		"else "\
			"echo ELOG setup file is not available on the server.;"\
		"fi;"\
	"fi \0"

/* eMMC partition for FIT images */
#define FIT_MMC_PARTITION \
	"fit_partitions=" \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=env,size=512K,uuid=${uuid_gpt_env};" \
	"name=Image_rsa.img,size=24MiB,uuid=${uuid_gpt_linux};" \
	"name=Image1_rsa.img,size=24MiB,uuid=${uuid_gpt_linux1};" \
	"name=Image2_rsa.img,size=24MiB,uuid=${uuid_gpt_linux2};" \
	"name=nitro,size=8MiB,uuid=${uuid_gpt_nitro};" \
	"name=recovery,size=940MiB,uuid=${uuid_gpt_recovery};" \
	"name=rootfs,size=-,uuid=${uuid_gpt_prootfs}\0"

#define QSPI_FLASH_NITRO_PARAMS \
	"spi_nitro_img_bin_start=0x400000\0" \
	"spi_nitro_img_bin_mirror_start=0x580000\0" \
	"spi_nitro_bspd_cfg_start=0x700000\0" \
	"spi_nitro_bspd_mirror_cfg_start=0x710000\0" \

#define QSPI_ACCESS_ENABLE \
	"qspi_access_en=" \
	"mw 0x68a403e8 1;" \
	"mw 0x68a403ec 1;" \
	"mw 0x68a403f0 1;" \
	"mw 0x68a403f4 1;" \
	"mw 0x68a403f8 1;" \
	"mw 0x68a403fc 1 \0"

#define FUNC_QSPI_PROBE \
	"func_qspi_probe="\
	"if run qspi_access_en; then "\
	"else "\
		"echo ${errstr} run qspi_access_en ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if sf probe 0; then "\
	"else "\
		"echo echo ${errstr} sf probe command ** FAILED **;"\
		"exit;"\
	"fi \0"

#define NITRO_FW_IMAGES \
	"nitro_bin=nitro.img\0" \
	"nitro_bspd_cfg=nitro_fb_bspd_config.bin\0"

#define FASTBOOT_NITRO_SETUP \
	"nitro_fastboot_type=1\0" \
	"nitro_fastboot_secure=1\0" \
	"nitro_fastboot_img_buffer=0\0" \
	"nitro_fit_img_loc=0x90000000\0"

#define FASTBOOT_SETUP \
	"fastboot_nitro_setup=" \
	"setenv errstr fastboot_setup;" \
	"run func_qspi_probe;" \
	/* first load header only */ \
	"if sf read ${nitro_fit_img_loc} "\
		   "${spi_nitro_img_bin_start} 0x18; then "\
	"else "\
		"echo [fastboot_nitro_setup] sf read "\
		      "${spi_nitro_img_bin_start} ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if spi_nitro_images_addr ${nitro_fit_img_loc} "\
				 "${spi_nitro_img_bin_start}; then "\
	"else "\
		 "echo [fastboot_nitro_setup] spi_nitro_images_addr "\
		       "** FAILED **;"\
		"exit;"\
	"fi \0"

#define CHECK_CHIMP_HS\
	"check_chimp_hs=chimp_hs"\
	"\0"

#define FASTBOOT_NITRO "fastboot_nitro=chimp_ld_secure\0"

#define FIT_IMAGE "fit_image=Image_rsa.img\0"
#define BOOTCMD_MMC_FIT \
	"bootcmd_mmc_fit="\
	"mmc dev ${sd_device_number};"\
	"if test $exec_state = normal; then " \
		"setenv use_rootfs rootfs;"\
	"else " \
		"setenv use_rootfs recovery;"\
	"fi;" \
	"echo used filesystem :${use_rootfs};"\
	"gpt setenv mmc ${sd_device_number} ${use_rootfs};"\
	"setenv bootargs_fs ${setbootargs} ${ext4rd_args}; run bootargs_fs;"\
	"gpt setenv mmc ${sd_device_number} ${fit_image};"\
	"mmc read ${fit_image_loadaddr} ${gpt_partition_addr} "\
	"${gpt_partition_size};"\
	"bootm ${fit_image_loadaddr}\0"

#define BOOTCMD_MMC_FITS \
	"bootcmd_mmc_fits="\
	"setenv mmc_fit0 " \
	"'setenv fit_image Image_rsa.img; run bootcmd_mmc_fit';"\
	"setenv mmc_fit1 " \
	"'setenv fit_image Image1_rsa.img; run bootcmd_mmc_fit';"\
	"setenv mmc_fit2 " \
	"'setenv fit_image Image2_rsa.img; run bootcmd_mmc_fit';"\
	"run mmc_fit0 || run mmc_fit1 || run mmc_fit2\0"

#define USBDEV "usbdev=0\0"
#define BOOTCMD_USB\
	"bootcmd_usb="\
	"setenv usb_image_loadaddr 90000000;"\
	"setenv fit_image Image_rsa.img;"\
	"setenv bootargs_fs ${setbootargs} ${initrd_args}; run bootargs_fs;"\
	"if usb dev ${usbdev}; && usb start; then "\
		"echo Booting from USB...;"\
		"fatload usb ${usbdev} ${usb_image_loadaddr} ${fit_image};"\
		"fatload usb ${usbdev} ${initrd_loadaddr} ${initrd_image};"\
		"bootm ${usb_image_loadaddr} ${initrd_loadaddr}:${filesize};"\
	"fi;"\
	"\0"

#define START_PCI\
	"start_pci=pci e "\
	"\0"

#define BNXT_LOAD\
	"bnxt_load=bnxt 0 probe "\
	"\0"

#define BOOTCMD_PXE\
	"bootcmd_pxe="\
	"run check_chimp_hs && "\
	"run start_pci && "\
	"run bnxt_load;"\
	"setenv ethact bnxt_eth0;"\
	"setenv autoload no;"\
	"setenv bootargs_fs ${setbootargs} ${initrd_args}; run bootargs_fs;"\
	"if dhcp; then "\
		"setenv pxefile_addr_r ${loadaddr};"\
		"if pxe get; then "\
			"setenv ramdisk_addr_r ${initrd_loadaddr};"\
			"setenv kernel_addr_r ${fit_image_loadaddr};"\
			"pxe boot; "\
		"fi;"\
	"fi;"\
	"\0"

#define FLASH_PENDING_RFS_IMGS \
	"flash_pending_rfs_imgs=" \
	"if test $bcm_bl_flash_pending_rfs_imgs = 1; then " \
		"if test $bl_flash_pending_rfs_imgs = rootfs; then " \
			"dhcp;" \
			"run mmc_flash_rootfs;" \
		"fi;" \
		"if test $bl_flash_pending_rfs_imgs = recovery; then " \
			"dhcp;" \
			"run mmc_flash_recovery;" \
		"fi;" \
		"setenv bl_flash_pending_rfs_imgs;" \
	"fi; \0"

#define CONFIG_BOOTCOMMAND "run flash_pending_rfs_imgs;" \
			   "run fastboot_nitro && "\
			   "run bootcmd_mmc_fits || "\
			   "run bootcmd_usb || "\
			   "run bootcmd_pxe"

#define ARCH_ENV_SETTINGS \
	CONSOLE_ARGS \
	MAX_CPUS \
	OS_LOG_LEVEL \
	EXTRA_ARGS \
	PCIE_ARGS \
	ETH_ADDR \
	RESERVED_MEM \
	SETBOOTARGS \
	UPDATEME_FLASH_PARAMS \
	KERNEL_LOADADDR_CFG\
	INITRD_ARGS \
	INITRD_LOADADDR \
	INITRD_IMAGE \
	MMC_DEV \
	EXEC_STATE \
	EXT4RD_ARGS \
	WDT_CNTRL \
	ELOG_SETUP \
	FIT_MMC_PARTITION \
	QSPI_FLASH_NITRO_PARAMS \
	QSPI_ACCESS_ENABLE \
	FUNC_QSPI_PROBE \
	NITRO_FW_IMAGES \
	FASTBOOT_NITRO_SETUP \
	FASTBOOT_SETUP \
	CHECK_CHIMP_HS \
	FASTBOOT_NITRO \
	FIT_IMAGE \
	BOOTCMD_MMC_FIT \
	BOOTCMD_MMC_FITS \
	USBDEV \
	BOOTCMD_USB \
	START_PCI \
	BNXT_LOAD \
	BOOTCMD_PXE \
	FLASH_PENDING_RFS_IMGS

#define CONFIG_EXTRA_ENV_SETTINGS \
	ARCH_ENV_SETTINGS

#endif /* __BCM_NS3_H */

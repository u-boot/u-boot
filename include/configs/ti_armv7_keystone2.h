/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common configuration header file for all Keystone II EVM platforms
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __CONFIG_KS2_EVM_H
#define __CONFIG_KS2_EVM_H

/* U-Boot Build Configuration */

/* SoC Configuration */

/* Memory Configuration */
#define CONFIG_SYS_LPAE_SDRAM_BASE	0x800000000
#define CONFIG_MAX_RAM_BANK_SIZE	(2 << 30)       /* 2GB */

#ifdef CONFIG_SYS_MALLOC_F_LEN
#define SPL_MALLOC_F_SIZE	CONFIG_SYS_MALLOC_F_LEN
#else
#define SPL_MALLOC_F_SIZE	0
#endif

/* SPL SPI Loader Configuration */
#define KEYSTONE_SPL_STACK_SIZE		(8 * 1024)

/* SRAM scratch space entries  */
#define SRAM_SCRATCH_SPACE_ADDR		0xc0c23fc

#define TI_SRAM_SCRATCH_BOARD_EEPROM_START	(SRAM_SCRATCH_SPACE_ADDR)
#define TI_SRAM_SCRATCH_BOARD_EEPROM_END	(SRAM_SCRATCH_SPACE_ADDR + 0x200)
#define KEYSTONE_SRAM_SCRATCH_SPACE_END		(TI_SRAM_SCRATCH_BOARD_EEPROM_END)

/* UART Configuration */
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_COM1		KS2_UART0_BASE
#define CONFIG_SYS_NS16550_COM2		KS2_UART1_BASE

#ifndef CONFIG_SOC_K2G
#define CONFIG_SYS_NS16550_CLK		ks_clk_get_rate(KS2_CLK1_6)
#else
#define CONFIG_SYS_NS16550_CLK		ks_clk_get_rate(uart_pll_clk) / 2
#endif

/* SPI Configuration */
#define CONFIG_SYS_SPI_CLK		ks_clk_get_rate(KS2_CLK1_6)

/* Network Configuration */
#define CONFIG_SYS_SGMII_REFCLK_MHZ	312
#define CONFIG_SYS_SGMII_LINERATE_MHZ	1250
#define CONFIG_SYS_SGMII_RATESCALE	2

/* Keystone net */
#define CONFIG_KSNET_MAC_ID_BASE		KS2_MAC_ID_BASE_ADDR
#define CONFIG_KSNET_NETCP_BASE			KS2_NETCP_BASE
#define CONFIG_KSNET_SERDES_SGMII_BASE		KS2_SGMII_SERDES_BASE
#define CONFIG_KSNET_SERDES_SGMII2_BASE		KS2_SGMII_SERDES2_BASE
#define CONFIG_KSNET_SERDES_LANES_PER_SGMII	KS2_LANES_PER_SGMII_SERDES

/* EEPROM definitions */

/* NAND Configuration */
#define CONFIG_SYS_NAND_MASK_CLE		0x4000
#define CONFIG_SYS_NAND_MASK_ALE		0x2000
#define CONFIG_SYS_NAND_CS			2

#define CONFIG_SYS_NAND_LARGEPAGE
#define CONFIG_SYS_NAND_BASE_LIST		{ 0x30000000, }
#define CONFIG_SYS_NAND_NO_SUBPAGE_WRITE

#define DFU_ALT_INFO_MMC \
	"dfu_alt_info_mmc=" \
	"MLO fat 0 1;" \
	"u-boot.img fat 0 1;" \
	"uEnv.txt fat 0 1\0"

/* DFU settings */
#define DFUARGS \
	"dfu_bufsiz=0x10000\0" \
	DFU_ALT_INFO_MMC \

/* U-Boot general configuration */

/* EDMA3 */

#define KERNEL_MTD_PARTS						\
	"mtdparts="							\
	SPI_MTD_PARTS

#define DEFAULT_FW_INITRAMFS_BOOT_ENV					\
	"name_fw_rd=k2-fw-initrd.cpio.gz\0"				\
	"set_rd_spec=setenv rd_spec ${rdaddr}:${filesize}\0"		\
	"init_fw_rd_net=dhcp ${rdaddr} ${tftp_root}/${name_fw_rd}; "	\
		"run set_rd_spec\0"					\
	"init_fw_rd_nfs=nfs ${rdaddr} ${nfs_root}/boot/${name_fw_rd}; "	\
		"run set_rd_spec\0"					\
	"init_fw_rd_ramfs=setenv rd_spec -\0"				\
	"init_fw_rd_ubi=ubifsload ${rdaddr} ${bootdir}/${name_fw_rd}; "	\
		"run set_rd_spec\0"					\

#define DEFAULT_PMMC_BOOT_ENV						\
	"set_name_pmmc=setenv name_pmmc ti-sci-firmware-${soc_variant}.bin\0" \
	"dev_pmmc=0\0"							\
	"get_pmmc_net=dhcp ${loadaddr} ${tftp_root}/${name_pmmc}\0"	\
	"get_pmmc_nfs=nfs ${loadaddr} ${nfs_root}/boot/${name_pmmc}\0"	\
	"get_pmmc_ramfs=run get_pmmc_net\0"				\
	"get_pmmc_mmc=load mmc ${bootpart} ${loadaddr} "		\
			"${bootdir}/${name_pmmc}\0"			\
	"get_pmmc_ubi=ubifsload ${loadaddr} ${bootdir}/${name_pmmc}\0"	\
	"run_pmmc=rproc init; rproc list; "				\
		"rproc load ${dev_pmmc} ${loadaddr} 0x${filesize}; "	\
		"rproc start ${dev_pmmc}\0"				\

#define CONFIG_EXTRA_ENV_SETTINGS					\
	DEFAULT_LINUX_BOOT_ENV						\
	ENV_KS2_BOARD_SETTINGS						\
	DFUARGS								\
	"bootdir=/boot\0" \
	"tftp_root=/\0"							\
	"nfs_root=/export\0"						\
	"mem_lpae=1\0"							\
	"uinitrd_fixup=1\0"						\
	"addr_ubi=0x82000000\0"						\
	"addr_secdb_key=0xc000000\0"					\
	"name_kern=zImage\0"						\
	"addr_mon=0x87000000\0"						\
	"addr_non_sec_mon=0x0c097fc0\0"					\
	"addr_load_sec_bm=0x0c09c000\0"					\
	"run_mon=mon_install ${addr_mon}\0"				\
	"run_mon_hs=mon_install ${addr_non_sec_mon} "			\
			"${addr_load_sec_bm}\0"				\
	"run_kern=bootz ${loadaddr} ${rd_spec} ${fdtaddr}\0"		\
	"init_net=run args_all args_net\0"				\
	"init_nfs=setenv autoload no; dhcp; run args_all args_net\0"	\
	"init_ubi=run args_all args_ubi; "				\
		"ubi part ubifs; ubifsmount ubi:rootfs;\0"			\
	"get_fdt_net=dhcp ${fdtaddr} ${tftp_root}/${name_fdt}\0"	\
	"get_fdt_nfs=nfs ${fdtaddr} ${nfs_root}/boot/${name_fdt}\0"	\
	"get_fdt_ubi=ubifsload ${fdtaddr} ${bootdir}/${name_fdt}\0"		\
	"get_kern_net=dhcp ${loadaddr} ${tftp_root}/${name_kern}\0"	\
	"get_kern_nfs=nfs ${loadaddr} ${nfs_root}/boot/${name_kern}\0"	\
	"get_kern_ubi=ubifsload ${loadaddr} ${bootdir}/${name_kern}\0"		\
	"get_mon_net=dhcp ${addr_mon} ${tftp_root}/${name_mon}\0"	\
	"get_mon_nfs=nfs ${addr_mon} ${nfs_root}/boot/${name_mon}\0"	\
	"get_mon_ubi=ubifsload ${addr_mon} ${bootdir}/${name_mon}\0"	\
	"get_fit_net=dhcp ${addr_fit} ${tftp_root}/${name_fit}\0"	\
	"get_fit_nfs=nfs ${addr_fit} ${nfs_root}/boot/${name_fit}\0"	\
	"get_fit_ubi=ubifsload ${addr_fit} ${bootdir}/${name_fit}\0"	\
	"get_fit_mmc=load mmc ${bootpart} ${addr_fit} ${bootdir}/${name_fit}\0" \
	"get_uboot_net=dhcp ${loadaddr} ${tftp_root}/${name_uboot}\0"	\
	"get_uboot_nfs=nfs ${loadaddr} ${nfs_root}/boot/${name_uboot}\0" \
	"burn_uboot_spi=sf probe; sf erase 0 0x100000; "		\
		"sf write ${loadaddr} 0 ${filesize}\0"		\
	"burn_uboot_nand=nand erase 0 0x100000; "			\
		"nand write ${loadaddr} 0 ${filesize}\0"		\
	"args_all=setenv bootargs console=ttyS0,115200n8 rootwait "	\
		KERNEL_MTD_PARTS					\
	"args_net=setenv bootargs ${bootargs} rootfstype=nfs "		\
		"root=/dev/nfs rw nfsroot=${serverip}:${nfs_root},"	\
		"${nfs_options} ip=dhcp\0"				\
	"nfs_options=v3,tcp,rsize=4096,wsize=4096\0"			\
	"get_fdt_ramfs=dhcp ${fdtaddr} ${tftp_root}/${name_fdt}\0"	\
	"get_kern_ramfs=dhcp ${loadaddr} ${tftp_root}/${name_kern}\0"	\
	"get_mon_ramfs=dhcp ${addr_mon} ${tftp_root}/${name_mon}\0"	\
	"get_fit_ramfs=dhcp ${addr_fit} ${tftp_root}/${name_fit}\0"	\
	"get_fs_ramfs=dhcp ${rdaddr} ${tftp_root}/${name_fs}\0"	\
	"get_ubi_net=dhcp ${addr_ubi} ${tftp_root}/${name_ubi}\0"	\
	"get_ubi_nfs=nfs ${addr_ubi} ${nfs_root}/boot/${name_ubi}\0"	\
	"burn_ubi=nand erase.part ubifs; "				\
		"nand write ${addr_ubi} ubifs ${filesize}\0"		\
	"init_ramfs=run args_all args_ramfs get_fs_ramfs\0"		\
	"args_ramfs=setenv bootargs ${bootargs} "			\
		"rdinit=/sbin/init rw root=/dev/ram0 "			\
		"initrd=0x808080000,80M\0"				\
	"no_post=1\0"

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

/* we may include files below only after all above definitions */
#include <asm/arch/hardware.h>
#include <asm/arch/clock.h>
#ifndef CONFIG_SOC_K2G
#define CONFIG_SYS_HZ_CLOCK		ks_clk_get_rate(KS2_CLK1_6)
#else
#define CONFIG_SYS_HZ_CLOCK		get_external_clk(sys_clk)
#endif

#endif /* __CONFIG_KS2_EVM_H */

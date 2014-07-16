/*
 * Configuration header file for TI's k2hk-evm
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_K2HK_EVM_H
#define __CONFIG_K2HK_EVM_H

/* Platform type */
#define CONFIG_SOC_K2HK
#define CONFIG_K2HK_EVM

/* U-Boot Build Configuration */
#define CONFIG_SKIP_LOWLEVEL_INIT       /* U-Boot is a 2nd stage loader */
#define CONFIG_SYS_NO_FLASH             /* that is, no *NOR* flash */
#define CONFIG_SYS_CONSOLE_INFO_QUIET
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_SYS_THUMB_BUILD

/* SoC Configuration */
#define CONFIG_ARMV7
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_SYS_ARCH_TIMER
#define CONFIG_SYS_HZ                   1000
#define CONFIG_SYS_TEXT_BASE            0x0c001000
#define CONFIG_SPL_TARGET               "u-boot-spi.gph"
#define CONFIG_SYS_DCACHE_OFF

/* Memory Configuration */
#define CONFIG_NR_DRAM_BANKS            2
#define CONFIG_SYS_SDRAM_BASE           0x80000000
#define CONFIG_SYS_LPAE_SDRAM_BASE      0x800000000
#define CONFIG_MAX_RAM_BANK_SIZE        (2 << 30)       /* 2GB */
#define CONFIG_STACKSIZE                (512 << 10)     /* 512 KiB */
#define CONFIG_SYS_MALLOC_LEN           (4 << 20)       /* 4 MiB */
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_TEXT_BASE - \
					 GENERATED_GBL_DATA_SIZE)

/* SPL SPI Loader Configuration */
#define CONFIG_SPL_TEXT_BASE            0x0c200000
#define CONFIG_SPL_PAD_TO               65536
#define CONFIG_SPL_MAX_SIZE             (CONFIG_SPL_PAD_TO - 8)
#define CONFIG_SPL_BSS_START_ADDR       (CONFIG_SPL_TEXT_BASE + \
					 CONFIG_SPL_MAX_SIZE)
#define CONFIG_SPL_BSS_MAX_SIZE         (32 * 1024)
#define CONFIG_SYS_SPL_MALLOC_START     (CONFIG_SPL_BSS_START_ADDR + \
					 CONFIG_SPL_BSS_MAX_SIZE)
#define CONFIG_SYS_SPL_MALLOC_SIZE      (32 * 1024)
#define CONFIG_SPL_STACK_SIZE           (8 * 1024)
#define CONFIG_SPL_STACK                (CONFIG_SYS_SPL_MALLOC_START + \
					 CONFIG_SYS_SPL_MALLOC_SIZE + \
					 CONFIG_SPL_STACK_SIZE - 4)
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SPL_SPI_BUS              0
#define CONFIG_SPL_SPI_CS               0
#define CONFIG_SYS_SPI_U_BOOT_OFFS      CONFIG_SPL_PAD_TO
#define CONFIG_SPL_FRAMEWORK

/* UART Configuration */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_REG_SIZE     -4
#define CONFIG_SYS_NS16550_COM1         KS2_UART0_BASE
#define CONFIG_SYS_NS16550_COM2         KS2_UART1_BASE
#define CONFIG_SYS_NS16550_CLK          clk_get_rate(K2HK_CLK1_6)
#define CONFIG_CONS_INDEX               1
#define CONFIG_BAUDRATE                 115200

/* SPI Configuration */
#define CONFIG_SPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_DAVINCI_SPI
#define CONFIG_SYS_SPI0
#define CONFIG_SYS_SPI_BASE             K2HK_SPI_BASE
#define CONFIG_SYS_SPI0_NUM_CS          4
#define CONFIG_SYS_SPI1
#define CONFIG_SYS_SPI1_BASE            K2HK_SPI1_BASE
#define CONFIG_SYS_SPI1_NUM_CS          4
#define CONFIG_SYS_SPI2
#define CONFIG_SYS_SPI2_NUM_CS          4
#define CONFIG_SYS_SPI2_BASE            K2HK_SPI2_BASE
#define CONFIG_CMD_SPI
#define CONFIG_SYS_SPI_CLK              clk_get_rate(K2HK_LPSC_EMIF25_SPI)
#define CONFIG_SF_DEFAULT_SPEED         30000000
#define CONFIG_ENV_SPI_MAX_HZ           CONFIG_SF_DEFAULT_SPEED

/* I2C Configuration */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_DAVINCI
#define CONFIG_SYS_DAVINCI_I2C_SPEED    100000
#define CONFIG_SYS_DAVINCI_I2C_SLAVE    0x10 /* SMBus host address */
#define CONFIG_SYS_DAVINCI_I2C_SPEED1   100000
#define CONFIG_SYS_DAVINCI_I2C_SLAVE1   0x10 /* SMBus host address */
#define CONFIG_SYS_DAVINCI_I2C_SPEED2   100000
#define CONFIG_SYS_DAVINCI_I2C_SLAVE2   0x10 /* SMBus host address */
#define I2C_BUS_MAX                     3

/* EEPROM definitions */
#define CONFIG_SYS_I2C_MULTI_EEPROMS
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN         2
#define CONFIG_SYS_I2C_EEPROM_ADDR             0x50
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS      6
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS  20
#define CONFIG_ENV_EEPROM_IS_ON_I2C

/* Network Configuration */
#define CONFIG_DRIVER_TI_KEYSTONE_NET
#define CONFIG_MII
#define CONFIG_BOOTP_DEFAULT
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT                 32
#define CONFIG_NET_MULTI
#define CONFIG_GET_LINK_STATUS_ATTEMPTS        5
#define CONFIG_SYS_SGMII_REFCLK_MHZ            312
#define CONFIG_SYS_SGMII_LINERATE_MHZ          1250
#define CONFIG_SYS_SGMII_RATESCALE             2

/* AEMIF */
#define CONFIG_TI_AEMIF
#define CONFIG_AEMIF_CNTRL_BASE		       KS2_AEMIF_CNTRL_BASE

/* NAND Configuration */
#define CONFIG_NAND_DAVINCI
#define CONFIG_CMD_NAND_ECCLAYOUT
#define CONFIG_SYS_NAND_CS                     2
#define CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_NAND_4BIT_HW_ECC_OOBFIRST
#define CONFIG_SYS_NAND_PAGE_2K
#define CONFIG_SYS_NAND_MASK_CLE		0x4000
#define CONFIG_SYS_NAND_MASK_ALE		0x2000

#define CONFIG_SYS_NAND_LARGEPAGE
#define CONFIG_SYS_NAND_BASE_LIST       { 0x30000000, }
#define CONFIG_SYS_MAX_NAND_DEVICE      1
#define CONFIG_SYS_NAND_MAX_CHIPS       1
#define CONFIG_SYS_NAND_NO_SUBPAGE_WRITE
#define CONFIG_ENV_SIZE                 (256 << 10)  /* 256 KiB */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET               0x100000
#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE
#define CONFIG_RBTREE
#define CONFIG_LZO
#define MTDPARTS_DEFAULT                "mtdparts=davinci_nand.0:" \
					"1024k(bootloader)ro,512k(params)ro," \
					"-(ubifs)"
/* U-Boot command configuration */
#include <config_cmd_default.h>
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_NAND
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_CMD_SF
#define CONFIG_CMD_EEPROM

/* U-Boot general configuration */
#define CONFIG_SYS_GENERIC_BOARD
#define CONFIG_SYS_PROMPT               "K2HK EVM # "
#define CONFIG_SYS_CBSIZE               1024
#define CONFIG_SYS_PBSIZE		2048
#define CONFIG_SYS_MAXARGS              16
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_LONGHELP
#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC
#define CONFIG_CMDLINE_EDITING
#define CONFIG_VERSION_VARIABLE
#define CONFIG_TIMESTAMP

#define CONFIG_BOOTDELAY                3
#define CONFIG_BOOTFILE                 "uImage"
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"boot=ramfs\0"							\
	"tftp_root=/\0"							\
	"nfs_root=/export\0"						\
	"mem_lpae=1\0"							\
	"mem_reserve=512M\0"						\
	"addr_fdt=0x87000000\0"						\
	"addr_kern=0x88000000\0"					\
	"addr_mon=0x0c5f0000\0"						\
	"addr_uboot=0x87000000\0"					\
	"addr_fs=0x82000000\0"						\
	"addr_ubi=0x82000000\0"						\
	"fdt_high=0xffffffff\0"						\
	"name_fdt=uImage-k2hk-evm.dtb\0"				\
	"name_fs=arago-console-image.cpio.gz\0"				\
	"name_kern=uImage-keystone-evm.bin\0"				\
	"name_mon=skern-keystone-evm.bin\0"				\
	"name_uboot=u-boot-spi-keystone-evm.gph\0"			\
	"name_ubi=keystone-evm-ubifs.ubi\0"				\
	"run_mon=mon_install ${addr_mon}\0"				\
	"run_kern=bootm ${addr_kern} - ${addr_fdt}\0"			\
	"init_net=run args_all args_net\0"				\
	"init_ubi=run args_all args_ubi; "				\
		"ubi part ubifs; ubifsmount boot\0"			\
	"get_fdt_net=dhcp ${addr_fdt} ${tftp_root}/${name_fdt}\0"	\
	"get_fdt_ubi=ubifsload ${addr_fdt} ${name_fdt}\0"		\
	"get_kern_net=dhcp ${addr_kern} ${tftp_root}/${name_kern}\0"	\
	"get_kern_ubi=ubifsload ${addr_kern} ${name_kern}\0"		\
	"get_mon_net=dhcp ${addr_mon} ${tftp_root}/${name_mon}\0"	\
	"get_mon_ubi=ubifsload ${addr_mon} ${name_mon}\0"		\
	"get_uboot_net=dhcp ${addr_uboot} ${tftp_root}/${name_uboot}\0"	\
	"burn_uboot=sf probe; sf erase 0 0x100000; "			\
		"sf write ${addr_uboot} 0 ${filesize}\0"		\
	"args_all=setenv bootargs console=ttyS0,115200n8 rootwait=1\0"	\
	"args_ubi=setenv bootargs ${bootargs} rootfstype=ubifs "	\
		"root=ubi0:rootfs rootflags=sync rw ubi.mtd=2,2048\0"	\
	"args_net=setenv bootargs ${bootargs} rootfstype=nfs "		\
		"root=/dev/nfs rw nfsroot=${serverip}:${nfs_root},"	\
		"${nfs_options} ip=dhcp\0"				\
	"nfs_options=v3,tcp,rsize=4096,wsize=4096\0"			\
	"get_fdt_ramfs=dhcp ${addr_fdt} ${tftp_root}/${name_fdt}\0"	\
	"get_kern_ramfs=dhcp ${addr_kern} ${tftp_root}/${name_kern}\0"	\
	"get_mon_ramfs=dhcp ${addr_mon} ${tftp_root}/${name_mon}\0"	\
	"get_fs_ramfs=dhcp ${addr_fs} ${tftp_root}/${name_fs}\0"	\
	"get_ubi_net=dhcp ${addr_ubi} ${tftp_root}/${name_ubi}\0"	\
	"burn_ubi=nand erase.part ubifs; "				\
		"nand write ${addr_ubi} ubifs ${filesize}\0"		\
	"init_ramfs=run args_all args_ramfs get_fs_ramfs\0"		\
	"args_ramfs=setenv bootargs ${bootargs} earlyprintk "		\
		"rdinit=/sbin/init rw root=/dev/ram0 "			\
		"initrd=0x802000000,9M\0"				\
	"no_post=1\0"							\
	"mtdparts=mtdparts=davinci_nand.0:"				\
		"1024k(bootloader)ro,512k(params)ro,522752k(ubifs)\0"
#define CONFIG_BOOTCOMMAND						\
	"run init_${boot} get_fdt_${boot} get_mon_${boot} "		\
		"get_kern_${boot} run_mon run_kern"
#define CONFIG_BOOTARGS							\

/* Linux interfacing */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_OF_LIBFDT                1
#define CONFIG_OF_BOARD_SETUP
#define CONFIG_SYS_BARGSIZE             1024
#define CONFIG_SYS_LOAD_ADDR            (CONFIG_SYS_SDRAM_BASE + 0x08000000)

#define CONFIG_SUPPORT_RAW_INITRD

/* we may include files below only after all above definitions */
#include <asm/arch/hardware.h>
#include <asm/arch/clock.h>
#define CONFIG_SYS_HZ_CLOCK             clk_get_rate(K2HK_CLK1_6)

#endif /* __CONFIG_K2HK_EVM_H */

/*
 * (C) Copyright 2013
 * Texas Instruments Incorporated.
 * Sricharan R	  <r.sricharan@ti.com>
 *
 * Derived from OMAP4 done by:
 *	Aneesh V <aneesh@ti.com>
 *
 * TI OMAP5 AND DRA7XX common configuration settings
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_OMAP5_COMMON_H
#define __CONFIG_OMAP5_COMMON_H

/*
 * High Level Configuration Options
 */
#define CONFIG_OMAP	/* in a TI OMAP core */
#define CONFIG_OMAP54XX	/* which is a 54XX */
#define CONFIG_OMAP_GPIO

/* Get CPU defs */
#include <asm/arch/cpu.h>
#include <asm/arch/omap.h>

/* Display CPU and Board Info */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* Clock Defines */
#define V_OSCK			19200000	/* Clock output from T2 */
#define V_SCLK	V_OSCK

#define CONFIG_MISC_INIT_R

#define CONFIG_OF_LIBFDT
#define CONFIG_CMD_BOOTZ

#define CONFIG_CMDLINE_TAG /* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/*
 * Size of malloc() pool
 * Total Size Environment - 128k
 * Malloc - add 256k
 */
#define CONFIG_ENV_SIZE			(128 << 10)
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (256 << 10))
/* Vector Base */
#define CONFIG_SYS_CA9_VECTOR_BASE	SRAM_ROM_VECT_BASE

/*
 * Hardware drivers
 */

/*
 * serial port - NS16550 compatible
 */
#define V_NS16550_CLK			48000000

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/* CPU */
#define CONFIG_ARCH_CPU_INIT

/* I2C  */
#define CONFIG_HARD_I2C
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		1
#define CONFIG_DRIVER_OMAP34XX_I2C
#define CONFIG_I2C_MULTI_BUS

/* MMC */
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_OMAP_HSMMC
#define CONFIG_DOS_PARTITION

#define CONFIG_SYS_CONSOLE_IS_IN_ENV

/* Flash */
#define CONFIG_SYS_NO_FLASH

/* Cache */
#define CONFIG_SYS_CACHELINE_SIZE	64
#define CONFIG_SYS_CACHELINE_SHIFT	6

/* commands to include */
#include <config_cmd_default.h>

/* Enabled commands */
#define CONFIG_CMD_EXT2		/* EXT2 Support                 */
#define CONFIG_CMD_FAT		/* FAT support                  */
#define CONFIG_CMD_I2C		/* I2C serial bus support	*/
#define CONFIG_CMD_MMC		/* MMC support                  */

/* Disabled commands */
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_FPGA		/* FPGA configuration Support   */
#undef CONFIG_CMD_IMLS		/* List all found images        */

/*
 * Environment setup
 */

#define CONFIG_BOOTDELAY	3
#define CONFIG_ENV_VARS_UBOOT_CONFIG
#define CONFIG_CMD_FS_GENERIC
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4

#define CONFIG_ENV_OVERWRITE

#ifndef PARTS_DEFAULT
#define PARTS_DEFAULT
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"console=ttyO2,115200n8\0" \
	"fdt_high=0xffffffff\0" \
	"fdtaddr=0x80f80000\0" \
	"bootpart=0:2\0" \
	"bootdir=/boot\0" \
	"bootfile=zImage\0" \
	"usbtty=cdc_acm\0" \
	"vram=16M\0" \
	"partitions=" PARTS_DEFAULT "\0" \
	"optargs=\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext4 rootwait\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"vram=${vram} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc${mmcdev} ...; " \
		"source ${loadaddr}\0" \
	"loadbootenv=fatload mmc ${mmcdev} ${loadaddr} uEnv.txt\0" \
	"importbootenv=echo Importing environment from mmc${mmcdev} ...; " \
		"env import -t ${loadaddr} ${filesize}\0" \
	"loadimage=load mmc ${bootpart} ${loadaddr} ${bootdir}/${bootfile}\0" \
	"mmcboot=echo Booting from mmc${mmcdev} ...; " \
		"run mmcargs; " \
		"bootz ${loadaddr} - ${fdtaddr}\0" \
	"findfdt="\
		"if test $board_name = omap5_uevm; then " \
			"setenv fdtfile omap5-uevm.dtb; fi;\0 " \
	"loadfdt=load mmc ${bootpart} ${fdtaddr} ${bootdir}/${fdtfile};\0" \

#define CONFIG_BOOTCOMMAND \
	"run findfdt; " \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loadbootenv; then " \
				"run importbootenv; " \
			"fi;" \
			"if test -n ${uenvcmd}; then " \
				"echo Running uenvcmd ...;" \
				"run uenvcmd;" \
			"fi;" \
		"fi;" \
		"if run loadimage; then " \
			"run loadfdt; " \
			"run mmcboot; " \
		"fi; " \
	"fi"

#define CONFIG_AUTO_COMPLETE		1

/*
 * Miscellaneous configurable options
 */

#define CONFIG_SYS_LONGHELP	/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER	/* use "hush" command parser */
#define CONFIG_SYS_CBSIZE		256
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

/*
 * memtest setup
 */
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + (32 << 20))

/* Default load address */
#define CONFIG_SYS_LOAD_ADDR		0x80000000

/* Use General purpose timer 1 */
#define CONFIG_SYS_TIMERBASE		GPT2_BASE
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			1000

/*
 * SDRAM Memory Map
 * Even though we use two CS all the memory
 * is mapped to one contiguous block
 */
#define CONFIG_NR_DRAM_BANKS	1

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_INIT_SP_ADDR         (NON_SECURE_SRAM_END - \
					 GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_EMIF_PRECALCULATED_TIMING_REGS

/* Defines for SDRAM init */
#ifndef CONFIG_SYS_EMIF_PRECALCULATED_TIMING_REGS
#define CONFIG_SYS_AUTOMATIC_SDRAM_DETECTION
#define CONFIG_SYS_DEFAULT_LPDDR2_TIMINGS
#endif

/* Defines for SPL */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_TEXT_BASE		0x40300350
#define CONFIG_SPL_MAX_SIZE		0x19000	/* 100K */
#define CONFIG_SPL_STACK		CONFIG_SYS_INIT_SP_ADDR
#define CONFIG_SPL_DISPLAY_PRINT

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /* address 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x200 /* 256 KB */
#define CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION	1
#define CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME	"u-boot.img"

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_LDSCRIPT "$(CPUDIR)/omap-common/u-boot-spl.lds"

/*
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 80E7FFC0--0x80E80000 should not be used for any
 * other needs.
 */
#define CONFIG_SYS_TEXT_BASE		0x80E80000

/*
 * BSS and malloc area 64MB into memory to allow enough
 * space for the kernel at the beginning of memory
 */
#define CONFIG_SPL_BSS_START_ADDR	0x84000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x100000	/* 1 MB */
#define CONFIG_SYS_SPL_MALLOC_START	0x84100000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000	/* 1 MB */
#define CONFIG_SPL_GPIO_SUPPORT

#endif /* __CONFIG_OMAP5_COMMON_H */

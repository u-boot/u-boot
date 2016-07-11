/*
 * Copyright (C) 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __LS2_COMMON_H
#define __LS2_COMMON_H

#define CONFIG_REMAKE_ELF
#define CONFIG_FSL_LAYERSCAPE
#define CONFIG_FSL_LSCH3
#define CONFIG_MP
#define CONFIG_GICV3
#define CONFIG_FSL_TZPC_BP147

#include <asm/arch/ls2080a_stream_id.h>
#include <asm/arch/config.h>
#if (defined(CONFIG_SYS_FSL_SRDS_1) || defined(CONFIG_SYS_FSL_SRDS_2))
#define	CONFIG_SYS_HAS_SERDES
#endif

/* Link Definitions */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_FSL_OCRAM_BASE + 0xfff0)

/* We need architecture specific misc initializations */
#define CONFIG_ARCH_MISC_INIT

/* Link Definitions */
#ifdef CONFIG_SPL
#define CONFIG_SYS_TEXT_BASE		0x80400000
#else
#define CONFIG_SYS_TEXT_BASE		0x30100000
#endif

#ifdef CONFIG_EMU
#define CONFIG_SYS_NO_FLASH
#endif

#define CONFIG_SUPPORT_RAW_INITRD

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F	1

#ifndef CONFIG_SPL
#define CONFIG_FSL_DDR_INTERACTIVE	/* Interactive debugging */
#endif
#ifndef CONFIG_SYS_FSL_DDR4
#define CONFIG_SYS_FSL_DDR3		/* Use DDR3 memory */
#define CONFIG_SYS_DDR_RAW_TIMING
#endif

#define CONFIG_SYS_FSL_DDR_INTLV_256B	/* force 256 byte interleaving */

#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000UL
#define CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY	0
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_DDR_BLOCK2_BASE	0x8080000000ULL
#define CONFIG_SYS_FSL_DDR_MAIN_NUM_CTRLS	2

/*
 * SMP Definitinos
 */
#define CPU_RELEASE_ADDR		secondary_boot_func

#define CONFIG_SYS_FSL_OTHER_DDR_NUM_CTRLS
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
#define CONFIG_SYS_DP_DDR_BASE		0x6000000000ULL
/*
 * DDR controller use 0 as the base address for binding.
 * It is mapped to CONFIG_SYS_DP_DDR_BASE for core to access.
 */
#define CONFIG_SYS_DP_DDR_BASE_PHY	0
#define CONFIG_DP_DDR_CTRL		2
#define CONFIG_DP_DDR_NUM_CTRLS		1
#endif

/* Generic Timer Definitions */
/*
 * This is not an accurate number. It is used in start.S. The frequency
 * will be udpated later when get_bus_freq(0) is available.
 */
#define COUNTER_FREQUENCY		25000000	/* 25MHz */

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2048 * 1024)

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_SYS_I2C_MXC_I2C4		/* enable I2C bus 4 */

/* Serial Port */
#define CONFIG_CONS_INDEX       1
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE     1
#define CONFIG_SYS_NS16550_CLK          (get_bus_freq(0)/2)

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* IFC */
#define CONFIG_FSL_IFC

/*
 * During booting, IFC is mapped at the region of 0x30000000.
 * But this region is limited to 256MB. To accommodate NOR, promjet
 * and FPGA. This region is divided as below:
 * 0x30000000 - 0x37ffffff : 128MB : NOR flash
 * 0x38000000 - 0x3BFFFFFF : 64MB  : Promjet
 * 0x3C000000 - 0x40000000 : 64MB  : FPGA etc
 *
 * To accommodate bigger NOR flash and other devices, we will map IFC
 * chip selects to as below:
 * 0x5_1000_0000..0x5_1fff_ffff	Memory Hole
 * 0x5_2000_0000..0x5_3fff_ffff	IFC CSx (FPGA, NAND and others 512MB)
 * 0x5_4000_0000..0x5_7fff_ffff	ASIC or others 1GB
 * 0x5_8000_0000..0x5_bfff_ffff	IFC CS0 1GB (NOR/Promjet)
 * 0x5_C000_0000..0x5_ffff_ffff	IFC CS1 1GB (NOR/Promjet)
 *
 * For e.g. NOR flash at CS0 will be mapped to 0x580000000 after relocation.
 * CONFIG_SYS_FLASH_BASE has the final address (core view)
 * CONFIG_SYS_FLASH_BASE_PHYS has the final address (IFC view)
 * CONFIG_SYS_FLASH_BASE_PHYS_EARLY has the temporary IFC address
 * CONFIG_SYS_TEXT_BASE is linked to 0x30000000 for booting
 */

#define CONFIG_SYS_FLASH_BASE			0x580000000ULL
#define CONFIG_SYS_FLASH_BASE_PHYS		0x80000000
#define CONFIG_SYS_FLASH_BASE_PHYS_EARLY	0x00000000

#define CONFIG_SYS_FLASH1_BASE_PHYS		0xC0000000
#define CONFIG_SYS_FLASH1_BASE_PHYS_EARLY	0x8000000

#ifndef CONFIG_SYS_NO_FLASH
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_SYS_FLASH_QUIET_TEST
#endif

#ifndef __ASSEMBLY__
unsigned long long get_qixis_addr(void);
#endif
#define QIXIS_BASE				get_qixis_addr()
#define QIXIS_BASE_PHYS				0x20000000
#define QIXIS_BASE_PHYS_EARLY			0xC000000
#define QIXIS_STAT_PRES1			0xb
#define QIXIS_SDID_MASK				0x07
#define QIXIS_ESDHC_NO_ADAPTER			0x7

#define CONFIG_SYS_NAND_BASE			0x530000000ULL
#define CONFIG_SYS_NAND_BASE_PHYS		0x30000000

/* Debug Server firmware */
#define CONFIG_FSL_DEBUG_SERVER
/* 2 sec timeout */
#define CONFIG_SYS_DEBUG_SERVER_TIMEOUT			(2 * 1000 * 1000)

/* MC firmware */
#define CONFIG_FSL_MC_ENET
/* TODO Actual DPL max length needs to be confirmed with the MC FW team */
#define CONFIG_SYS_LS_MC_DPC_MAX_LENGTH	    0x20000
#define CONFIG_SYS_LS_MC_DRAM_DPC_OFFSET    0x00F00000
#define CONFIG_SYS_LS_MC_DPL_MAX_LENGTH	    0x20000
#define CONFIG_SYS_LS_MC_DRAM_DPL_OFFSET    0x00F20000
/* For LS2085A */
#define CONFIG_SYS_LS_MC_AIOP_IMG_MAX_LENGTH	0x200000
#define CONFIG_SYS_LS_MC_DRAM_AIOP_IMG_OFFSET	0x07000000

/*
 * Carve out a DDR region which will not be used by u-boot/Linux
 *
 * It will be used by MC and Debug Server. The MC region must be
 * 512MB aligned, so the min size to hide is 512MB.
 */
#if defined(CONFIG_FSL_MC_ENET) || defined(CONFIG_FSL_DEBUG_SERVER)
#define CONFIG_SYS_DEBUG_SERVER_DRAM_BLOCK_MIN_SIZE	(254UL * 1024 * 1024)
#define CONFIG_SYS_LS_MC_DRAM_BLOCK_MIN_SIZE		(512UL * 1024 * 1024)
#define CONFIG_SYS_MC_RSV_MEM_ALIGN			(512UL * 1024 * 1024)
#endif

/* PCIe */
#define CONFIG_PCIE1		/* PCIE controller 1 */
#define CONFIG_PCIE2		/* PCIE controller 2 */
#define CONFIG_PCIE3		/* PCIE controller 3 */
#define CONFIG_PCIE4		/* PCIE controller 4 */
#define CONFIG_PCIE_LAYERSCAPE	/* Use common FSL Layerscape PCIe code */
#ifdef CONFIG_LS2080A
#define FSL_PCIE_COMPAT "fsl,ls2080a-pcie"
#endif

#define CONFIG_SYS_PCI_64BIT

#define CONFIG_SYS_PCIE_CFG0_PHYS_OFF	0x00000000
#define CONFIG_SYS_PCIE_CFG0_SIZE	0x00001000	/* 4k */
#define CONFIG_SYS_PCIE_CFG1_PHYS_OFF	0x00001000
#define CONFIG_SYS_PCIE_CFG1_SIZE	0x00001000	/* 4k */

#define CONFIG_SYS_PCIE_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE_IO_PHYS_OFF	0x00010000
#define CONFIG_SYS_PCIE_IO_SIZE		0x00010000	/* 64k */

#define CONFIG_SYS_PCIE_MEM_BUS		0x40000000
#define CONFIG_SYS_PCIE_MEM_PHYS_OFF	0x40000000
#define CONFIG_SYS_PCIE_MEM_SIZE	0x40000000	/* 1G */

/* Command line configuration */
#define CONFIG_CMD_ENV

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_DDR_SDRAM_BASE + 0x10000000)
#define CONFIG_ARCH_EARLY_INIT_R

/* Physical Memory Map */
/* fixme: these need to be checked against the board */
#define CONFIG_CHIP_SELECTS_PER_CTRL	4

#define CONFIG_NR_DRAM_BANKS		3

#define CONFIG_HWCONFIG
#define HWCONFIG_BUFFER_SIZE		128

#define CONFIG_DISPLAY_CPUINFO

/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"loadaddr=0x80100000\0"			\
	"kernel_addr=0x100000\0"		\
	"ramdisk_addr=0x800000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"initrd_high=0xffffffffffffffff\0"	\
	"kernel_start=0x581200000\0"		\
	"kernel_load=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"console=ttyAMA0,38400n8\0"		\
	"mcinitcmd=fsl_mc start mc 0x580300000"	\
	" 0x580800000 \0"

#define CONFIG_BOOTARGS		"console=ttyS0,115200 root=/dev/ram0 " \
				"earlycon=uart8250,mmio,0x21c0500 " \
				"ramdisk_size=0x2000000 default_hugepagesz=2m" \
				" hugepagesz=2m hugepages=256"
#define CONFIG_BOOTCOMMAND	"fsl_mc apply dpl 0x580700000 &&" \
				" cp.b $kernel_start $kernel_load" \
				" $kernel_size && bootm $kernel_load"
#define CONFIG_BOOTDELAY		10

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE /* Boot args buffer */
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING		1
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_MAXARGS		64	/* max command args */

#define CONFIG_PANIC_HANG	/* do not reset board on panic */

#define CONFIG_SPL_BSS_START_ADDR	0x80100000
#define CONFIG_SPL_BSS_MAX_SIZE		0x00100000
#define CONFIG_SPL_DRIVERS_MISC_SUPPORT
#define CONFIG_SPL_ENV_SUPPORT
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SPL_LDSCRIPT "arch/arm/cpu/armv8/u-boot-spl.lds"
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_MAX_SIZE		0x16000
#define CONFIG_SPL_MPC8XXX_INIT_DDR_SUPPORT
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_STACK		(CONFIG_SYS_FSL_OCRAM_BASE + 0x9ff0)
#define CONFIG_SPL_TARGET		"u-boot-with-spl.bin"
#define CONFIG_SPL_TEXT_BASE		0x1800a000

#define CONFIG_SYS_NAND_U_BOOT_DST	0x80400000
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_NAND_U_BOOT_DST
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x00100000
#define CONFIG_SYS_SPL_MALLOC_START	0x80200000
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)

#define CONFIG_SYS_BOOTM_LEN   (64 << 20)      /* Increase max gunzip size */

#endif /* __LS2_COMMON_H */

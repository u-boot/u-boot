/*
 * Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __CONFIG_SOCFPGA_COMMON_H__
#define __CONFIG_SOCFPGA_COMMON_H__

/* Virtual target or real hardware */
#undef CONFIG_SOCFPGA_VIRTUAL_TARGET

/*
 * High level configuration
 */
#define CONFIG_DISPLAY_BOARDINFO_LATE
#define CONFIG_CLOCKS

#define CONFIG_SYS_BOOTMAPSZ		(64 * 1024 * 1024)

#define CONFIG_TIMESTAMP		/* Print image info with timestamp */

/* add target to build it automatically upon "make" */
#define CONFIG_BUILD_TARGET		"u-boot-with-spl.sfp"

/*
 * Memory configurations
 */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			0x0
#define CONFIG_SYS_MALLOC_LEN		(64 * 1024 * 1024)
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END		PHYS_SDRAM_1_SIZE
#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
#define CONFIG_SYS_INIT_RAM_ADDR	0xFFFF0000
#define CONFIG_SYS_INIT_RAM_SIZE	0x10000
#elif defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
#define CONFIG_SYS_INIT_RAM_ADDR	0xFFE00000
#define CONFIG_SYS_INIT_RAM_SIZE	0x40000 /* 256KB */
#endif
#define CONFIG_SYS_INIT_SP_OFFSET		\
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR			\
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/*
 * U-Boot general configurations
 */
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O buffer size */
						/* Print buffer size */
#define CONFIG_SYS_MAXARGS	32		/* Max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
						/* Boot argument buffer size */

#ifndef CONFIG_SYS_HOSTNAME
#define CONFIG_SYS_HOSTNAME	CONFIG_SYS_BOARD
#endif

/*
 * Cache
 */
#define CONFIG_SYS_L2_PL310
#define CONFIG_SYS_PL310_BASE		SOCFPGA_MPUL2_ADDRESS

/*
 * EPCS/EPCQx1 Serial Flash Controller
 */
#ifdef CONFIG_ALTERA_SPI
#define CONFIG_SF_DEFAULT_SPEED		30000000
/*
 * The base address is configurable in QSys, each board must specify the
 * base address based on it's particular FPGA configuration. Please note
 * that the address here is incremented by  0x400  from the Base address
 * selected in QSys, since the SPI registers are at offset +0x400.
 * #define CONFIG_SYS_SPI_BASE		0xff240400
 */
#endif

/*
 * Ethernet on SoC (EMAC)
 */
#if defined(CONFIG_CMD_NET) && !defined(CONFIG_SOCFPGA_VIRTUAL_TARGET)
#define CONFIG_DW_ALTDESCRIPTOR
#define CONFIG_MII
#endif

/*
 * FPGA Driver
 */
#ifdef CONFIG_CMD_FPGA
#define CONFIG_FPGA_COUNT		1
#endif

/*
 * L4 OSC1 Timer 0
 */
/* This timer uses eosc1, whose clock frequency is fixed at any condition. */
#define CONFIG_SYS_TIMERBASE		SOCFPGA_OSC1TIMER0_ADDRESS
#define CONFIG_SYS_TIMER_COUNTS_DOWN
#define CONFIG_SYS_TIMER_COUNTER	(CONFIG_SYS_TIMERBASE + 0x4)
#ifdef CONFIG_SOCFPGA_VIRTUAL_TARGET
#define CONFIG_SYS_TIMER_RATE		2400000
#else
#define CONFIG_SYS_TIMER_RATE		25000000
#endif

/*
 * L4 Watchdog
 */
#ifdef CONFIG_HW_WATCHDOG
#define CONFIG_DESIGNWARE_WATCHDOG
#define CONFIG_DW_WDT_BASE		SOCFPGA_L4WD0_ADDRESS
#define CONFIG_DW_WDT_CLOCK_KHZ		25000
#define CONFIG_WATCHDOG_TIMEOUT_MSECS	30000
#endif

/*
 * MMC Driver
 */
#ifdef CONFIG_CMD_MMC
#define CONFIG_BOUNCE_BUFFER
/* FIXME */
/* using smaller max blk cnt to avoid flooding the limited stack we have */
#define CONFIG_SYS_MMC_MAX_BLK_COUNT	256	/* FIXME -- SPL only? */
#endif

/*
 * NAND Support
 */
#ifdef CONFIG_NAND_DENALI
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_NAND_REGS_BASE	SOCFPGA_NANDREGS_ADDRESS
#define CONFIG_SYS_NAND_DATA_BASE	SOCFPGA_NANDDATA_ADDRESS
#endif

/*
 * I2C support
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_BASE		SOCFPGA_I2C0_ADDRESS
#define CONFIG_SYS_I2C_BASE1		SOCFPGA_I2C1_ADDRESS
#define CONFIG_SYS_I2C_BASE2		SOCFPGA_I2C2_ADDRESS
#define CONFIG_SYS_I2C_BASE3		SOCFPGA_I2C3_ADDRESS
/* Using standard mode which the speed up to 100Kb/s */
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SPEED1		100000
#define CONFIG_SYS_I2C_SPEED2		100000
#define CONFIG_SYS_I2C_SPEED3		100000
/* Address of device when used as slave */
#define CONFIG_SYS_I2C_SLAVE		0x02
#define CONFIG_SYS_I2C_SLAVE1		0x02
#define CONFIG_SYS_I2C_SLAVE2		0x02
#define CONFIG_SYS_I2C_SLAVE3		0x02
#ifndef __ASSEMBLY__
/* Clock supplied to I2C controller in unit of MHz */
unsigned int cm_get_l4_sp_clk_hz(void);
#define IC_CLK				(cm_get_l4_sp_clk_hz() / 1000000)
#endif

/*
 * QSPI support
 */
/* Enable multiple SPI NOR flash manufacturers */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_SPI_FLASH_MTD
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#endif
/* QSPI reference clock */
#ifndef __ASSEMBLY__
unsigned int cm_get_qspi_controller_clk_hz(void);
#define CONFIG_CQSPI_REF_CLK		cm_get_qspi_controller_clk_hz()
#endif

/*
 * Designware SPI support
 */

/*
 * Serial Driver
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#ifdef CONFIG_SOCFPGA_VIRTUAL_TARGET
#define CONFIG_SYS_NS16550_CLK		1000000
#elif defined(CONFIG_TARGET_SOCFPGA_GEN5)
#define CONFIG_SYS_NS16550_COM1		SOCFPGA_UART0_ADDRESS
#define CONFIG_SYS_NS16550_CLK		100000000
#elif defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
#define CONFIG_SYS_NS16550_COM1        SOCFPGA_UART1_ADDRESS
#define CONFIG_SYS_NS16550_CLK		50000000
#endif
#define CONFIG_CONS_INDEX		1

/*
 * USB
 */

/*
 * USB Gadget (DFU, UMS)
 */
#if defined(CONFIG_CMD_DFU) || defined(CONFIG_CMD_USB_MASS_STORAGE)
#define CONFIG_SYS_DFU_DATA_BUF_SIZE	(16 * 1024 * 1024)
#define DFU_DEFAULT_POLL_TIMEOUT	300

/* USB IDs */
#define CONFIG_G_DNL_UMS_VENDOR_NUM	0x0525
#define CONFIG_G_DNL_UMS_PRODUCT_NUM	0xA4A5
#endif

/*
 * U-Boot environment
 */
#if !defined(CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE			(8 * 1024)
#endif

/* Environment for SDMMC boot */
#if defined(CONFIG_ENV_IS_IN_MMC) && !defined(CONFIG_ENV_OFFSET)
#define CONFIG_SYS_MMC_ENV_DEV		0 /* device 0 */
#define CONFIG_ENV_OFFSET		(34 * 512) /* just after the GPT */
#endif

/* Environment for QSPI boot */
#if defined(CONFIG_ENV_IS_IN_SPI_FLASH) && !defined(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_OFFSET		0x00100000
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#endif

/*
 * mtd partitioning for serial NOR flash
 *
 * device nor0 <ff705000.spi.0>, # parts = 6
 * #: name                size            offset          mask_flags
 * 0: u-boot              0x00100000      0x00000000      0
 * 1: env1                0x00040000      0x00100000      0
 * 2: env2                0x00040000      0x00140000      0
 * 3: UBI                 0x03e80000      0x00180000      0
 * 4: boot                0x00e80000      0x00180000      0
 * 5: rootfs              0x01000000      0x01000000      0
 *
 */

/*
 * SPL
 *
 * SRAM Memory layout:
 *
 * 0xFFFF_0000 ...... Start of SRAM
 * 0xFFFF_xxxx ...... Top of stack (grows down)
 * 0xFFFF_yyyy ...... Malloc area
 * 0xFFFF_zzzz ...... Global Data
 * 0xFFFF_FF00 ...... End of SRAM
 */
#define CONFIG_SPL_TEXT_BASE		CONFIG_SYS_INIT_RAM_ADDR
#define CONFIG_SPL_MAX_SIZE		CONFIG_SYS_INIT_RAM_SIZE

/* SPL SDMMC boot support */
#ifdef CONFIG_SPL_MMC_SUPPORT
#if defined(CONFIG_SPL_FAT_SUPPORT) || defined(CONFIG_SPL_EXT_SUPPORT)
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot-dtb.img"
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#endif
#else
#ifndef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION	1
#endif
#endif

/* SPL QSPI boot support */
#ifdef CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x40000
#endif

/* SPL NAND boot support */
#ifdef CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x40000
#endif

/*
 * Stack setup
 */
#define CONFIG_SPL_STACK		CONFIG_SYS_INIT_SP_ADDR

/* Extra Environment */
#ifndef CONFIG_SPL_BUILD

#ifdef CONFIG_CMD_DHCP
#define BOOT_TARGET_DEVICES_DHCP(func) func(DHCP, dhcp, na)
#else
#define BOOT_TARGET_DEVICES_DHCP(func)
#endif

#ifdef CONFIG_CMD_PXE
#define BOOT_TARGET_DEVICES_PXE(func) func(PXE, pxe, na)
#else
#define BOOT_TARGET_DEVICES_PXE(func)
#endif

#ifdef CONFIG_CMD_MMC
#define BOOT_TARGET_DEVICES_MMC(func) func(MMC, mmc, 0)
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_PXE(func) \
	BOOT_TARGET_DEVICES_DHCP(func)

#include <config_distro_bootcmd.h>

#ifndef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"bootm_size=0xa000000\0" \
	"kernel_addr_r="__stringify(CONFIG_SYS_LOAD_ADDR)"\0" \
	"fdt_addr_r=0x02000000\0" \
	"scriptaddr=0x02100000\0" \
	"pxefile_addr_r=0x02200000\0" \
	"ramdisk_addr_r=0x02300000\0" \
	BOOTENV

#endif
#endif

#endif	/* __CONFIG_SOCFPGA_COMMON_H__ */

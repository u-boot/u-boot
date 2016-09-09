/*
 * Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __CONFIG_SOCFPGA_COMMON_H__
#define __CONFIG_SOCFPGA_COMMON_H__

/* Virtual target or real hardware */
#undef CONFIG_SOCFPGA_VIRTUAL_TARGET

#define CONFIG_SYS_THUMB_BUILD

/*
 * High level configuration
 */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO_LATE
#define CONFIG_ARCH_MISC_INIT
#define CONFIG_ARCH_EARLY_INIT_R
#define CONFIG_SYS_NO_FLASH
#define CONFIG_CLOCKS

#define CONFIG_CRC32_VERIFY

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

#define CONFIG_SYS_INIT_RAM_ADDR	0xFFFF0000
#define CONFIG_SYS_INIT_RAM_SIZE	0x10000
#define CONFIG_SYS_INIT_SP_OFFSET		\
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR			\
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#ifdef CONFIG_SOCFPGA_VIRTUAL_TARGET
#define CONFIG_SYS_TEXT_BASE		0x08000040
#else
#define CONFIG_SYS_TEXT_BASE		0x01000040
#endif

/*
 * U-Boot general configurations
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O buffer size */
#define CONFIG_SYS_PBSIZE	\
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
						/* Print buffer size */
#define CONFIG_SYS_MAXARGS	32		/* Max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
						/* Boot argument buffer size */
#define CONFIG_AUTO_COMPLETE			/* Command auto complete */
#define CONFIG_CMDLINE_EDITING			/* Command history etc */

#ifndef CONFIG_SYS_HOSTNAME
#define CONFIG_SYS_HOSTNAME	CONFIG_SYS_BOARD
#endif

/*
 * Cache
 */
#define CONFIG_SYS_L2_PL310
#define CONFIG_SYS_PL310_BASE		SOCFPGA_MPUL2_ADDRESS

/*
 * SDRAM controller
 */
#define CONFIG_ALTERA_SDRAM

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
#define CONFIG_AUTONEG_TIMEOUT		(15 * CONFIG_SYS_HZ)
#define CONFIG_PHY_GIGE
#endif

/*
 * FPGA Driver
 */
#ifdef CONFIG_CMD_FPGA
#define CONFIG_FPGA
#define CONFIG_FPGA_ALTERA
#define CONFIG_FPGA_SOCFPGA
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
#define CONFIG_HW_WATCHDOG_TIMEOUT_MS	30000
#endif

/*
 * MMC Driver
 */
#ifdef CONFIG_CMD_MMC
#define CONFIG_MMC
#define CONFIG_BOUNCE_BUFFER
#define CONFIG_GENERIC_MMC
#define CONFIG_DWMMC
#define CONFIG_SOCFPGA_DWMMC
#define CONFIG_SOCFPGA_DWMMC_FIFO_DEPTH	1024
/* FIXME */
/* using smaller max blk cnt to avoid flooding the limited stack we have */
#define CONFIG_SYS_MMC_MAX_BLK_COUNT	256	/* FIXME -- SPL only? */
#endif

/*
 * NAND Support
 */
#ifdef CONFIG_NAND_DENALI
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_MAX_CHIPS	1
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_NAND_DENALI_ECC_SIZE	512
#define CONFIG_SYS_NAND_REGS_BASE	SOCFPGA_NANDREGS_ADDRESS
#define CONFIG_SYS_NAND_DATA_BASE	SOCFPGA_NANDDATA_ADDRESS
#define CONFIG_SYS_NAND_BASE		(CONFIG_SYS_NAND_DATA_BASE + 0x10)
#endif

/*
 * I2C support
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_BUS_MAX		4
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
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define MTDIDS_DEFAULT			"nor0=ff705000.spi.0"
#endif
/* QSPI reference clock */
#ifndef __ASSEMBLY__
unsigned int cm_get_qspi_controller_clk_hz(void);
#define CONFIG_CQSPI_REF_CLK		cm_get_qspi_controller_clk_hz()
#endif
#define CONFIG_CQSPI_DECODER		0

/*
 * Designware SPI support
 */

/*
 * Serial Driver
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#define CONFIG_SYS_NS16550_COM1		SOCFPGA_UART0_ADDRESS
#ifdef CONFIG_SOCFPGA_VIRTUAL_TARGET
#define CONFIG_SYS_NS16550_CLK		1000000
#else
#define CONFIG_SYS_NS16550_CLK		100000000
#endif
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/*
 * USB
 */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_DWC2
#endif

/*
 * USB Gadget (DFU, UMS)
 */
#if defined(CONFIG_CMD_DFU) || defined(CONFIG_CMD_USB_MASS_STORAGE)
#define CONFIG_USB_FUNCTION_MASS_STORAGE

#define CONFIG_USB_FUNCTION_DFU
#ifdef CONFIG_DM_MMC
#define CONFIG_DFU_MMC
#endif
#define CONFIG_SYS_DFU_DATA_BUF_SIZE	(32 * 1024 * 1024)
#define DFU_DEFAULT_POLL_TIMEOUT	300

/* USB IDs */
#define CONFIG_G_DNL_UMS_VENDOR_NUM	0x0525
#define CONFIG_G_DNL_UMS_PRODUCT_NUM	0xA4A5
#endif

/*
 * U-Boot environment
 */
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
#define CONFIG_SYS_CONSOLE_ENV_OVERWRITE
#if !defined(CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE			4096
#endif

/* Environment for SDMMC boot */
#if defined(CONFIG_ENV_IS_IN_MMC) && !defined(CONFIG_ENV_OFFSET)
#define CONFIG_SYS_MMC_ENV_DEV		0	/* device 0 */
#define CONFIG_ENV_OFFSET		512	/* just after the MBR */
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
#if defined(CONFIG_CMD_SF) && !defined(MTDPARTS_DEFAULT)
#define MTDPARTS_DEFAULT	"mtdparts=ff705000.spi.0:"\
				"1m(u-boot),"		\
				"256k(env1),"		\
				"256k(env2),"		\
				"14848k(boot),"		\
				"16m(rootfs),"		\
				"-@1536k(UBI)\0"
#endif

/* UBI and UBIFS support */
#if defined(CONFIG_CMD_SF) || defined(CONFIG_CMD_NAND)
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_RBTREE
#define CONFIG_LZO
#endif

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
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_RAM_DEVICE
#define CONFIG_SPL_TEXT_BASE		CONFIG_SYS_INIT_RAM_ADDR
#define CONFIG_SPL_MAX_SIZE		(64 * 1024)

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_WATCHDOG_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#ifdef CONFIG_DM_MMC
#define CONFIG_SPL_MMC_SUPPORT
#endif
#ifdef CONFIG_DM_SPI
#define CONFIG_SPL_SPI_SUPPORT
#endif
#ifdef CONFIG_SPL_NAND_DENALI
#define CONFIG_SPL_NAND_SUPPORT
#endif

/* SPL SDMMC boot support */
#ifdef CONFIG_SPL_MMC_SUPPORT
#if defined(CONFIG_SPL_FAT_SUPPORT) || defined(CONFIG_SPL_EXT_SUPPORT)
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	2
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot-dtb.img"
#define CONFIG_SPL_LIBDISK_SUPPORT
#else
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION	1
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x200 /* offset 512 sect (256k) */
#define CONFIG_SPL_LIBDISK_SUPPORT
#endif
#endif

/* SPL QSPI boot support */
#ifdef CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x40000
#endif

/* SPL NAND boot support */
#ifdef CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x40000
#endif

/*
 * Stack setup
 */
#define CONFIG_SPL_STACK		CONFIG_SYS_INIT_SP_ADDR

#endif	/* __CONFIG_SOCFPGA_COMMON_H__ */

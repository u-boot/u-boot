/*
 * Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __CONFIG_SOCFPGA_CYCLONE5_COMMON_H__
#define __CONFIG_SOCFPGA_CYCLONE5_COMMON_H__

#define CONFIG_SYS_GENERIC_BOARD

/* Virtual target or real hardware */
#undef CONFIG_SOCFPGA_VIRTUAL_TARGET

#define CONFIG_SYS_THUMB_BUILD

/*
 * High level configuration
 */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO_LATE
#define CONFIG_ARCH_EARLY_INIT_R
#define CONFIG_SYS_NO_FLASH
#define CONFIG_CLOCKS

#define CONFIG_FIT
#define CONFIG_OF_LIBFDT
#define CONFIG_SYS_BOOTMAPSZ		(64 * 1024 * 1024)

#define CONFIG_TIMESTAMP		/* Print image info with timestamp */

/*
 * Memory configurations
 */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			0x0
#define CONFIG_SYS_MALLOC_LEN		(64 * 1024 * 1024)
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END		PHYS_SDRAM_1_SIZE

#define CONFIG_SYS_INIT_RAM_ADDR	0xFFFF0000
#define CONFIG_SYS_INIT_RAM_SIZE	(0x10000 - CONFIG_SYS_SPL_MALLOC_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR					\
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE -	\
	GENERATED_GBL_DATA_SIZE)

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
#define CONFIG_VERSION_VARIABLE			/* U-BOOT version */
#define CONFIG_AUTO_COMPLETE			/* Command auto complete */
#define CONFIG_CMDLINE_EDITING			/* Command history etc */
#define CONFIG_SYS_HUSH_PARSER

/*
 * Cache
 */
#define CONFIG_SYS_ARM_CACHE_WRITEALLOC
#define CONFIG_SYS_CACHELINE_SIZE 32
#define CONFIG_SYS_L2_PL310
#define CONFIG_SYS_PL310_BASE		SOCFPGA_MPUL2_ADDRESS

/*
 * EPCS/EPCQx1 Serial Flash Controller
 */
#ifdef CONFIG_ALTERA_SPI
#define CONFIG_CMD_SPI
#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_SPEED		30000000
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_BAR
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
#define CONFIG_PHYLIB
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
#define CONFIG_SOCFPGA_DWMMC_DRVSEL	3
#define CONFIG_SOCFPGA_DWMMC_SMPSEL	0
/* FIXME */
/* using smaller max blk cnt to avoid flooding the limited stack we have */
#define CONFIG_SYS_MMC_MAX_BLK_COUNT	256	/* FIXME -- SPL only? */
#endif

/*
 * I2C support
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_DW
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
#define CONFIG_CMD_I2C

/*
 * QSPI support
 */
#ifdef CONFIG_OF_CONTROL	/* QSPI is controlled via DT */
#define CONFIG_CADENCE_QSPI
/* Enable multiple SPI NOR flash manufacturers */
#define CONFIG_SPI_FLASH_STMICRO	/* Micron/Numonyx flash */
#define CONFIG_SPI_FLASH_SPANSION	/* Spansion flash */
#define CONFIG_SPI_FLASH_MTD
/* QSPI reference clock */
#ifndef __ASSEMBLY__
unsigned int cm_get_qspi_controller_clk_hz(void);
#define CONFIG_CQSPI_REF_CLK		cm_get_qspi_controller_clk_hz()
#endif
#define CONFIG_CQSPI_DECODER		0
#define CONFIG_CMD_SF
#endif

#ifdef CONFIG_OF_CONTROL	/* DW SPI is controlled via DT */
#define CONFIG_DESIGNWARE_SPI
#define CONFIG_CMD_SPI
#endif

/*
 * Serial Driver
 */
#define CONFIG_SYS_NS16550
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
#define CONFIG_USB_STORAGE
/*
 * NOTE: User must define either of the following to select which
 *       of the two USB controllers available on SoCFPGA to use.
 *       The DWC2 driver doesn't support multiple USB controllers.
 * #define CONFIG_USB_DWC2_REG_ADDR	SOCFPGA_USB0_ADDRESS
 * #define CONFIG_USB_DWC2_REG_ADDR	SOCFPGA_USB1_ADDRESS
 */
#endif

/*
 * USB Gadget (DFU, UMS)
 */
#if defined(CONFIG_CMD_DFU) || defined(CONFIG_CMD_USB_MASS_STORAGE)
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_S3C_UDC_OTG
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET_VBUS_DRAW	2

/* USB Composite download gadget - g_dnl */
#define CONFIG_USBDOWNLOAD_GADGET
#define CONFIG_USB_GADGET_MASS_STORAGE

#define CONFIG_DFU_FUNCTION
#define CONFIG_DFU_MMC
#define CONFIG_SYS_DFU_DATA_BUF_SIZE	(32 * 1024 * 1024)
#define DFU_DEFAULT_POLL_TIMEOUT	300

/* USB IDs */
#define CONFIG_G_DNL_VENDOR_NUM		0x0525	/* NetChip */
#define CONFIG_G_DNL_PRODUCT_NUM	0xA4A5	/* Linux-USB File-backed Storage Gadget */
#define CONFIG_G_DNL_UMS_VENDOR_NUM	CONFIG_G_DNL_VENDOR_NUM
#define CONFIG_G_DNL_UMS_PRODUCT_NUM	CONFIG_G_DNL_PRODUCT_NUM
#ifndef CONFIG_G_DNL_MANUFACTURER
#define CONFIG_G_DNL_MANUFACTURER	"Altera"
#endif
#endif

/*
 * U-Boot environment
 */
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
#define CONFIG_SYS_CONSOLE_ENV_OVERWRITE
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			4096

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
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_RAM_DEVICE
#define CONFIG_SPL_TEXT_BASE		CONFIG_SYS_INIT_RAM_ADDR
#define CONFIG_SYS_SPL_MALLOC_START	CONFIG_SYS_INIT_SP_ADDR
#define CONFIG_SYS_SPL_MALLOC_SIZE	(5 * 1024)
#define CONFIG_SPL_MAX_SIZE		(64 * 1024)

#define CHUNKSZ_CRC32			(1 * 1024)	/* FIXME: ewww */
#define CONFIG_CRC32_VERIFY

/* Linker script for SPL */
#define CONFIG_SPL_LDSCRIPT	"arch/arm/mach-socfpga/u-boot-spl.lds"

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_WATCHDOG_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT

/*
 * Stack setup
 */
#define CONFIG_SPL_STACK		CONFIG_SYS_INIT_SP_ADDR

#ifdef CONFIG_SPL_BUILD
#undef CONFIG_PARTITIONS
#endif

#endif	/* __CONFIG_SOCFPGA_CYCLONE5_COMMON_H__ */

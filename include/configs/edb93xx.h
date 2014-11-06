/*
 * U-boot - Configuration file for Cirrus Logic EDB93xx boards
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef CONFIG_MK_edb9301
#define CONFIG_EDB9301
#elif defined(CONFIG_MK_edb9302)
#define CONFIG_EDB9302
#elif defined(CONFIG_MK_edb9302a)
#define CONFIG_EDB9302A
#elif defined(CONFIG_MK_edb9307)
#define CONFIG_EDB9307
#elif defined(CONFIG_MK_edb9307a)
#define CONFIG_EDB9307A
#elif defined(CONFIG_MK_edb9312)
#define CONFIG_EDB9312
#elif defined(CONFIG_MK_edb9315)
#define CONFIG_EDB9315
#elif defined(CONFIG_MK_edb9315a)
#define CONFIG_EDB9315A
#else
#error "no board defined"
#endif

/* Initial environment and monitor configuration options. */
#define CONFIG_BOOTDELAY		2
#define CONFIG_CMDLINE_TAG		1
#define CONFIG_INITRD_TAG		1
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_BOOTARGS		"root=/dev/nfs console=ttyAM0,115200 ip=dhcp"
#define CONFIG_BOOTFILE		"edb93xx.img"

#define CONFIG_SYS_HUSH_PARSER		1
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "


#define CONFIG_SYS_LDSCRIPT	"board/cirrus/edb93xx/u-boot.lds"


#ifdef CONFIG_EDB9301
#define CONFIG_EP9301
#define CONFIG_MACH_TYPE		MACH_TYPE_EDB9301
#define CONFIG_SYS_PROMPT		"EDB9301> "
#define CONFIG_ENV_SECT_SIZE		0x00020000
#elif defined(CONFIG_EDB9302)
#define CONFIG_EP9302
#define CONFIG_MACH_TYPE		MACH_TYPE_EDB9302
#define CONFIG_SYS_PROMPT		"EDB9302> "
#define CONFIG_ENV_SECT_SIZE		0x00020000
#elif defined(CONFIG_EDB9302A)
#define CONFIG_EP9302
#define CONFIG_MACH_TYPE		MACH_TYPE_EDB9302A
#define CONFIG_SYS_PROMPT		"EDB9302A> "
#define CONFIG_ENV_SECT_SIZE		0x00020000
#elif defined(CONFIG_EDB9307)
#define CONFIG_EP9307
#define CONFIG_MACH_TYPE		MACH_TYPE_EDB9307
#define CONFIG_SYS_PROMPT		"EDB9307> "
#define CONFIG_ENV_SECT_SIZE		0x00040000
#elif defined(CONFIG_EDB9307A)
#define CONFIG_EP9307
#define CONFIG_MACH_TYPE		MACH_TYPE_EDB9307A
#define CONFIG_SYS_PROMPT		"EDB9307A> "
#define CONFIG_ENV_SECT_SIZE		0x00020000
#elif defined(CONFIG_EDB9312)
#define CONFIG_EP9312
#define CONFIG_MACH_TYPE		MACH_TYPE_EDB9312
#define CONFIG_SYS_PROMPT		"EDB9312> "
#define CONFIG_ENV_SECT_SIZE		0x00040000
#elif defined(CONFIG_EDB9315)
#define CONFIG_EP9315
#define CONFIG_MACH_TYPE		MACH_TYPE_EDB9315
#define CONFIG_SYS_PROMPT		"EDB9315> "
#define CONFIG_ENV_SECT_SIZE		0x00040000
#elif defined(CONFIG_EDB9315A)
#define CONFIG_EP9315
#define CONFIG_MACH_TYPE		MACH_TYPE_EDB9315A
#define CONFIG_SYS_PROMPT		"EDB9315A> "
#define CONFIG_ENV_SECT_SIZE		0x00020000
#else
#error "no board defined"
#endif

/* High-level configuration options */
#define CONFIG_EP93XX		1		/* This is a Cirrus Logic 93xx SoC */

#define CONFIG_SYS_CLK_FREQ	14745600	/* EP93xx has a 14.7456 clock */
#undef CONFIG_USE_IRQ				/* Don't need IRQ/FIQ */

/* Monitor configuration */
#include <config_cmd_default.h>
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_SETGETDCR
#undef CONFIG_CMD_XIMG

#undef CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_JFFS2

#define CONFIG_SYS_LONGHELP			/* Enable "long" help in mon */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O buffer size */
/* Print buffer size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
/* Boot argument buffer size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
#define CONFIG_SYS_MAXARGS	16		/* Max number of command args */

/* Serial port hardware configuration */
#define CONFIG_PL010_SERIAL
#define CONFIG_CONS_INDEX		0
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, \
                        115200, 230400}
#define CONFIG_SYS_SERIAL0		0x808C0000
#define CONFIG_SYS_SERIAL1		0x808D0000
/*#define CONFIG_PL01x_PORTS	{(void *)CONFIG_SYS_SERIAL0, \
            (void *)CONFIG_SYS_SERIAL1} */

#define CONFIG_PL01x_PORTS	{(void *)CONFIG_SYS_SERIAL0}

/* Status LED */
#define CONFIG_STATUS_LED		1 /* Status LED enabled	*/
#define CONFIG_BOARD_SPECIFIC_LED	1
#define STATUS_LED_GREEN		0
#define STATUS_LED_RED			1
/* Green */
#define STATUS_LED_BIT			STATUS_LED_GREEN
#define STATUS_LED_STATE		STATUS_LED_ON
#define STATUS_LED_PERIOD		(CONFIG_SYS_HZ / 2)
/* Red */
#define STATUS_LED_BIT1			STATUS_LED_RED
#define STATUS_LED_STATE1		STATUS_LED_OFF
#define STATUS_LED_PERIOD1		(CONFIG_SYS_HZ / 2)
/* Optional value */
#define STATUS_LED_BOOT			STATUS_LED_BIT

/* Network hardware configuration */
#define CONFIG_DRIVER_EP93XX_MAC
#define CONFIG_MII_SUPPRESS_PREAMBLE
#define CONFIG_MII
#define CONFIG_PHY_ADDR		1
#define CONFIG_NET_MULTI
#undef CONFIG_NETCONSOLE

/* SDRAM configuration */
#if defined(CONFIG_EDB9301) || defined(CONFIG_EDB9302) || \
    defined(CONFIG_EDB9307) || defined CONFIG_EDB9312 || \
    defined(CONFIG_EDB9315)
/*
 * EDB9301/2 has 4 banks of SDRAM consisting of 1x Samsung K4S561632E-TC75
 * 256 Mbit SDRAM on a 16-bit data bus, for a total of 32MB of SDRAM. We set
 * the SROMLL bit on the processor, resulting in this non-contiguous memory map.
 *
 * The EDB9307, EDB9312, and EDB9315 have 2 banks of SDRAM consisting of
 * 2x Samsung K4S561632E-TC75 256 Mbit on a 32-bit data bus, for a total of
 * 64 MB of SDRAM.
 */

#define CONFIG_EDB93XX_SDCS3

#elif defined(CONFIG_EDB9302A) || \
    defined(CONFIG_EDB9307A) || defined(CONFIG_EDB9315A)
/*
 * EDB9302a has 4 banks of SDRAM consisting of 1x Samsung K4S561632E-TC75
 * 256 Mbit SDRAM on a 16-bit data bus, for a total of 32MB of SDRAM. We set
 * the SROMLL bit on the processor, resulting in this non-contiguous memory map.
 *
 * The EDB9307A and EDB9315A have 2 banks of SDRAM consisting of 2x Samsung
 * K4S561632E-TC75 256 Mbit on a 32-bit data bus, for a total of 64 MB of SDRAM.
 */
#define CONFIG_EDB93XX_SDCS0

#else
#error "no SDCS configuration for this board"
#endif


#if defined(CONFIG_EDB93XX_SDCS3)
#define CONFIG_SYS_LOAD_ADDR	0x01000000	/* Default load address	*/
#define PHYS_SDRAM_1		0x00000000
#elif defined(CONFIG_EDB93XX_SDCS0)
#define CONFIG_SYS_LOAD_ADDR	0xc1000000	/* Default load address	*/
#define PHYS_SDRAM_1		0xc0000000
#endif

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_NR_DRAM_BANKS		8

#define CONFIG_SYS_INIT_SP_ADDR \
    (CONFIG_SYS_SDRAM_BASE + 32*1024 - GENERATED_GBL_DATA_SIZE)


/* Must match kernel config */
#define LINUX_BOOT_PARAM_ADDR	(PHYS_SDRAM_1 + 0x100)

/* Run-time memory allocatons */
#define CONFIG_SYS_GBL_DATA_SIZE	128
#define CONFIG_STACKSIZE		(128 * 1024)

#if defined(CONFIG_USE_IRQ)
#define CONFIG_STACKSIZE_IRQ	(4 * 1024)
#define CONFIG_STACKSIZE_FIQ	(4 * 1024)
#endif

#define CONFIG_SYS_MALLOC_LEN		(512 * 1024)

/* -----------------------------------------------------------------------------
 * FLASH and environment organization
 *
 * The EDB9301, EDB9302(a), EDB9307a, EDB9315a have 1 bank of flash memory at
 * 0x60000000 consisting of 1x Intel TE28F128J3C-150 128 Mbit flash on a 16-bit
 * data bus, for a total of 16 MB of CFI-compatible flash.
 *
 * The EDB9307, EDB9312, and EDB9315 have 1 bank of flash memory at
 * 0x60000000 consisting of 2x Micron MT28F128J3-12 128 Mbit flash on a 32-bit
 * data bus, for a total of 32 MB of CFI-compatible flash.
 *
 *
 *                            EDB9301/02(a)7a/15a    EDB9307/12/15
 * 0x60000000 - 0x0003FFFF    u-boot                 u-boot
 * 0x60040000 - 0x0005FFFF    environment #1         environment #1
 * 0x60060000 - 0x0007FFFF    environment #2         environment #1 (continued)
 * 0x60080000 - 0x0009FFFF    unused                 environment #2
 * 0x600A0000 - 0x000BFFFF    unused                 environment #2 (continued)
 * 0x600C0000 - 0x00FFFFFF    unused                 unused
 * 0x61000000 - 0x01FFFFFF    not present            unused
 */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE


#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	(256+8)

#define CONFIG_SYS_TEXT_BASE		0x60000000
#define PHYS_FLASH_1			CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)

#define CONFIG_ENV_OVERWRITE		/* Vendor params unprotected */
#define CONFIG_ENV_IS_IN_FLASH

#define CONFIG_ENV_ADDR			0x60040000
#define CONFIG_ENV_ADDR_REDUND		(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)

#define CONFIG_ENV_SIZE			CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE

/* Define to enable MMC on SPI support */
/* #define CONFIG_EP93XX_SPI_MMC */

#ifdef CONFIG_EP93XX_SPI_MMC
#define CONFIG_EP93XX_SPI
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC_SPI
#define CONFIG_CMD_MMC
#define CONFIG_MMC_SPI_NPOWER_EGPIO	9
#endif

#define CONFIG_USB_STORAGE
#define CONFIG_USB_OHCI_NEW
#define CONFIG_USB_OHCI_EP93XX
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	3
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"ep93xx-ohci"
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x80020000

#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_FAT
#define CONFIG_CMD_USB

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_CMD_BOOTZ

/* Define to disable flash configuration*/
/* #define CONFIG_EP93XX_NO_FLASH_CFG */

/* Define this for indusrial rated chips */
/* #define CONFIG_EDB93XX_INDUSTRIAL */

#endif /* !defined (__CONFIG_H) */

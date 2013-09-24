/*
 * (C) Copyright 2010
 * Reinhard Meyer, EMK Elektronik, reinhard.meyer@emk-elektronik.de
 *
 * Configuation settings for the TOP9000 CPU module with AT91SAM9XE.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/*
 * top9000 with at91sam9xe256 or at91sam9xe512
 *
 * Initial Bootloader is in embedded flash.
 * Vital Product Data, U-Boot Environment are in I2C-EEPROM.
 * U-Boot is in embedded flash, a backup U-Boot can be in NAND flash.
 * kernel and file system are either in NAND flash or on a micro SD card.
 * NAND flash is optional.
 * I2C EEPROM is never optional.
 * SPI FRAM is optional.
 * SPI ENC28J60 is optional.
 * 16 or 32 bit wide SDRAM.
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/* SoC must be defined first, before hardware.h is included */
#define CONFIG_AT91SAM9XE
#include <asm/hardware.h>

/*
 * Warning: changing CONFIG_SYS_TEXT_BASE requires
 * adapting the initial boot program.
 */
#define CONFIG_SYS_TEXT_BASE		0x20000000	/* start of SDRAM */

/* Command line configuration */
#include <config_cmd_default.h>
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_SETGETDCR
#undef CONFIG_CMD_XIMG
#define CONFIG_CMD_ASKENV
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_PBSIZE \
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_PROMPT		"TOP9000> "
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_CMD_BDI
#define CONFIG_CMD_CACHE

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000	/* main clock xtal */
#define CONFIG_SYS_HZ			1000

/* Misc CPU related */
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_AT91RESET_EXTRST		/* assert external reset */

/* general purpose I/O */
#define CONFIG_ATMEL_LEGACY		/* required until (g)pio is fixed */
#define CONFIG_AT91_GPIO
#define CONFIG_AT91_GPIO_PULLUP	1	/* keep pullups on peripheral pins */

/* serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define	CONFIG_USART_ID			ATMEL_ID_SYS
#define CONFIG_BAUDRATE			115200

/* SD/MMC card */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_GENERIC_ATMEL_MCI
#define CONFIG_SYS_MMC_CD_PIN		AT91_PIN_PC9
#define CONFIG_CMD_MMC

/* Ethernet */
#define CONFIG_MACB
#define CONFIG_SYS_PHY_ID	1
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT	20

/* real time clock */
#define CONFIG_RTC_AT91SAM9_RTT
#define CONFIG_CMD_DATE

#if defined(CONFIG_AT91SAM9XE)
/*
 * NOR flash - use embedded flash of SAM9XE256/512
 * U-Boot will not fit into 128K !
 * 2010.09 will not fit into 256K with all options enabled !
 *
 * Layout:
 * 16kB	1st Bootloader
 * Rest U-Boot
 * the first sector (16kB) of EFLASH cannot be unprotected
 * with u-boot commands
 */
# define CONFIG_AT91_EFLASH
# define CONFIG_SYS_FLASH_BASE		ATMEL_BASE_FLASH
# define CONFIG_SYS_MAX_FLASH_SECT	32
# define CONFIG_SYS_MAX_FLASH_BANKS	1
# define CONFIG_SYS_FLASH_PROTECTION
# define CONFIG_EFLASH_PROTSECTORS	1	/* protect first sector */
#endif

/* SPI */
#define CONFIG_ATMEL_SPI
#define CONFIG_CMD_SPI

/* RAMTRON FRAM */
#define CONFIG_CMD_SF
#define CONFIG_ATMEL_SPI0		/* SPI used for FRAM is SPI0 */
#define FRAM_SPI_BUS		0
#define FRAM_CS_NUM		0
#define CONFIG_SPI_FRAM_RAMTRON
#define CONFIG_SF_DEFAULT_SPEED	1000000	/* be conservative here... */
#define CONFIG_SF_DEFAULT_MODE	SPI_MODE_0
#define CONFIG_SPI_FRAM_RAMTRON_NON_JEDEC	"FM25H20"

/* Microchip ENC28J60 (second LAN) */
#if defined(CONFIG_EVAL9000)
# define CONFIG_ENC28J60
# define CONFIG_ATMEL_SPI1		/* SPI used for ENC28J60 is SPI1 */
# define ENC_SPI_BUS		1
# define ENC_CS_NUM		0
# define ENC_SPI_CLOCK	1000000
#endif /* CONFIG_EVAL9000 */

/*
 * SDRAM: 1 bank, min 32, max 128 MB
 * Initialized before u-boot gets started.
 */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CONFIG_SYS_SDRAM_SIZE		0x08000000
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x01e00000)
#define CONFIG_SYS_LOAD_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 0x01000000)
/*
 * Initial stack pointer: 16k - GENERATED_GBL_DATA_SIZE in internal SRAM,
 * leaving the correct space for initial global data structure above
 * that address while providing maximum stack area below.
 */
#define CONFIG_SYS_INIT_SP_ADDR \
	(ATMEL_BASE_SRAM + 0x4000 - GENERATED_GBL_DATA_SIZE)

/*
 * NAND flash: 256 MB (optional)
 *
 * Layout:
 * 640kB: u-boot (includes space for spare sectors, handled by
 * initial loader)
 * 2MB: kernel
 * rest: file system
 */
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		ATMEL_BASE_CS3
#define CONFIG_SYS_NAND_DBW_8
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN	AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN	AT91_PIN_PC13
#define CONFIG_CMD_NAND

/* USB */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_OHCI_NEW
#define CONFIG_DOS_PARTITION
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE	ATMEL_UHP_BASE
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"top9000"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_USB

/* I2C support must always be enabled */
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SOFT		/* I2C bit-banged */
#define CONFIG_SYS_I2C_SOFT_SPEED	400000
#define CONFIG_SYS_I2C_SOFT_SLAVE	0x7F

#define I2C0_PORT			AT91_PIO_PORTA
#define SDA0_PIN			23
#define SCL0_PIN			24
#define I2C1_PORT			AT91_PIO_PORTB
#define SDA1_PIN			12
#define SCL1_PIN			13
#define I2C_SOFT_DECLARATIONS		void iic_init(void);\
					int iic_read(void);\
					void iic_sda(int);\
					void iic_scl(int);
#define I2C_ACTIVE
#define I2C_TRISTATE
#define I2C_INIT			iic_init()
#define I2C_READ			iic_read()
#define I2C_SDA(bit)			iic_sda(bit)
#define I2C_SCL(bit)			iic_scl(bit)
#define I2C_DELAY			udelay(3)
/* EEPROM configuration */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	5
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2
#define CONFIG_SYS_EEPROM_SIZE		0x2000
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x57
/* later: #define CONFIG_I2C_ENV_EEPROM_BUS	0 */
/* ENV is always in I2C-EEPROM */
#define CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_OFFSET		0x1000
#define CONFIG_ENV_SIZE			0x0f00
/* VPD settings */
#define CONFIG_SYS_I2C_FACT_ADDR	0x57
#define CONFIG_SYS_FACT_OFFSET		0x1F00
#define CONFIG_SYS_FACT_SIZE		0x0100
/* later: #define CONFIG_MISC_INIT_R */
/* define the next only if you want to allow users to enter VPD data */
#define CONFIG_SYS_FACT_ENTRY
#ifndef __ASSEMBLY__
extern void read_factory_r(void);
#endif

/*
 * Only interrupt autoboot if <space> is pressed. Otherwise, garbage
 * data on the serial line may interrupt the boot sequence.
 */
#define CONFIG_BOOTDELAY		1
#define CONFIG_AUTOBOOT
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT \
	"Press SPACE to abort autoboot in %d seconds\n", bootdelay
#define CONFIG_AUTOBOOT_DELAY_STR	"d"
#define CONFIG_AUTOBOOT_STOP_STR	" "

/*
 * add filesystem commands if we have at least 1 storage
 * media with filesystem
 */
#if defined(CONFIG_NAND_ATMEL) \
	|| defined(CONFIG_USB_ATMEL) \
	|| defined(CONFIG_MMC)
# define CONFIG_DOS_PARTITION
# define CONFIG_CMD_FAT
# define CONFIG_CMD_EXT2
/* later: #define CONFIG_CMD_JFFS2 */
#endif

/* add NET commands if we have at least 1 LAN */
#if defined(CONFIG_MACB) || defined(CONFIG_ENC28J60)
# define CONFIG_CMD_PING
# define CONFIG_CMD_DHCP
# define CONFIG_CMD_MII
/* is this really needed ? */
# define CONFIG_RESET_PHY_R
/* BOOTP options */
# define CONFIG_BOOTP_BOOTFILESIZE
# define CONFIG_BOOTP_BOOTPATH
# define CONFIG_BOOTP_GATEWAY
# define CONFIG_BOOTP_HOSTNAME
#endif

/* linux in NAND flash */
#define CONFIG_BOOTCOUNT_LIMIT	1
#define CONFIG_BOOTCOMMAND \
	"nand read 0x21000000 0xA0000 0x200000; bootm"
#define CONFIG_BOOTARGS \
	"console=ttyS0,115200 " \
	"root=/dev/mtdblock2 " \
	"mtdparts=atmel_nand:" \
		"640k(uboot)ro," \
		"2M(linux)," \
		"16M(root)," \
		"-(rest) " \
	"rw "\
	"rootfstype=jffs2"

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN \
	ROUND(3 * CONFIG_ENV_SIZE + 128*1024, 0x1000)

#endif

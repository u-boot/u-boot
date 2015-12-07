/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * This file contains the configuration parameters for the VCT board
 * family:
 *
 * vct_premium
 * vct_premium_small
 * vct_premium_onenand
 * vct_premium_onenand_small
 * vct_platinum
 * vct_platinum_small
 * vct_platinum_onenand
 * vct_platinum_onenand_small
 * vct_platinumavc
 * vct_platinumavc_small
 * vct_platinumavc_onenand
 * vct_platinumavc_onenand_small
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_DISPLAY_BOARDINFO

#define CPU_CLOCK_RATE			324000000 /* Clock for the MIPS core */
#define CONFIG_SYS_MIPS_TIMER_FREQ	(CPU_CLOCK_RATE / 2)

#define CONFIG_SKIP_LOWLEVEL_INIT	/* SDRAM is initialized by the bootstrap code */

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)
#define CONFIG_SYS_MALLOC_LEN		(1 << 20)
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 << 10)
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000

#if !defined(CONFIG_VCT_NAND) && !defined(CONFIG_VCT_ONENAND)
#define CONFIG_VCT_NOR
#else
#define CONFIG_SYS_NO_FLASH
#endif

/*
 * UART
 */
#ifdef CONFIG_VCT_PLATINUMAVC
#define UART_1_BASE		0xBDC30000
#else
#define UART_1_BASE		0xBF89C000
#endif

#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#define CONFIG_SYS_NS16550_COM1		UART_1_BASE
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_CLK		921600
#define CONFIG_BAUDRATE			115200

/*
 * SDRAM
 */
#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_MBYTES_SDRAM		128
#define CONFIG_SYS_MEMTEST_START	0x80200000
#define CONFIG_SYS_MEMTEST_END		0x80400000
#define CONFIG_SYS_LOAD_ADDR		0x80400000	/* default load address */

#if defined(CONFIG_VCT_PREMIUM) || defined(CONFIG_VCT_PLATINUM)
/*
 * SMSC91C11x Network Card
 */
#define CONFIG_SMC911X
#define CONFIG_SMC911X_BASE	0x00000000
#define CONFIG_SMC911X_32_BIT
#define CONFIG_NET_RETRY_COUNT		20
#endif

/*
 * Commands
 */
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ELF
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C

/*
 * Only Premium/Platinum have ethernet support right now
 */
#if (defined(CONFIG_VCT_PREMIUM) || defined(CONFIG_VCT_PLATINUM)) && \
	!defined(CONFIG_VCT_SMALL_IMAGE)
#define CONFIG_CMD_PING
#define CONFIG_CMD_SNTP
#endif

/*
 * Only Premium/Platinum have USB-EHCI support right now
 */
#if (defined(CONFIG_VCT_PREMIUM) || defined(CONFIG_VCT_PLATINUM)) && \
	!defined(CONFIG_VCT_SMALL_IMAGE)
#define CONFIG_CMD_USB
#define CONFIG_CMD_FAT
#endif

#if defined(CONFIG_CMD_USB)
#define CONFIG_USB_STORAGE
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

#define CONFIG_SUPPORT_VFAT

/*
 * USB/EHCI
 */
#define CONFIG_USB_EHCI			/* Enable EHCI USB support	*/
#define CONFIG_USB_EHCI_VCT		/* on VCT platform		*/
#define CONFIG_EHCI_MMIO_BIG_ENDIAN
#define CONFIG_EHCI_DESC_BIG_ENDIAN
#define CONFIG_EHCI_IS_TDI
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET /* re-init HCD after CMD_RESET */
#endif /* CONFIG_CMD_USB */

#if defined(CONFIG_VCT_NAND)
#define CONFIG_CMD_NAND
#endif

#if defined(CONFIG_VCT_ONENAND)
#define CONFIG_CMD_ONENAND
#endif

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_SUBNETMASK

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_CBSIZE	512		/* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + \
				 sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_TIMESTAMP			/* Print image info with timestamp */
#define CONFIG_CMDLINE_EDITING			/* add command line history	*/
#define CONFIG_SYS_CONSOLE_INFO_QUIET		/* don't print console @ startup*/

/*
 * FLASH and environment organization
 */
#if defined(CONFIG_VCT_NOR)
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_FLASH_NOT_MEM_MAPPED

/*
 * We need special accessor functions for the CFI FLASH driver. This
 * can be enabled via the CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS option.
 */
#define CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS

/*
 * For the non-memory-mapped NOR FLASH, we need to define the
 * NOR FLASH area. This can't be detected via the addr2info()
 * function, since we check for flash access in the very early
 * U-Boot code, before the NOR FLASH is detected.
 */
#define CONFIG_FLASH_BASE		0xb0000000
#define CONFIG_FLASH_END		0xbfffffff

/*
 * CFI driver settings
 */
#define CONFIG_SYS_FLASH_CFI			/* The flash is CFI compatible	*/
#define CONFIG_FLASH_CFI_DRIVER		/* Use common CFI driver	*/
#define CONFIG_SYS_FLASH_CFI_AMD_RESET	1	/* Use AMD (Spansion) reset cmd */
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT	/* no byte writes on IXP4xx	*/

#define CONFIG_SYS_FLASH_BASE		0xb0000000
#define CONFIG_SYS_FLASH_BANKS_LIST    { CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x10000		/* size of one complete sector	*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN)
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif /* CONFIG_ENV_IS_IN_FLASH */
#endif /* CONFIG_VCT_NOR */

#if defined(CONFIG_VCT_ONENAND)
#define CONFIG_USE_ONENAND_BOARD_INIT
#define	CONFIG_ENV_IS_IN_ONENAND
#define	CONFIG_SYS_ONENAND_BASE		0x00000000	/* this is not real address */
#define CONFIG_SYS_FLASH_BASE		0x00000000
#define CONFIG_ENV_ADDR			(128 << 10)	/* after compr. U-Boot image */
#define	CONFIG_ENV_SIZE			(128 << 10)	/* erase size */
#endif /* CONFIG_VCT_ONENAND */

/*
 * Cache Configuration
 */
#define CONFIG_SYS_DCACHE_SIZE		16384
#define CONFIG_SYS_ICACHE_SIZE		16384
#define CONFIG_SYS_CACHELINE_SIZE	32

/*
 * I2C/EEPROM
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SOFT		/* I2C bit-banged */
#define CONFIG_SYS_I2C_SOFT_SPEED	83000	/* 83 kHz is supposed to work */
#define CONFIG_SYS_I2C_SOFT_SLAVE	0x7f

/*
 * Software (bit-bang) I2C driver configuration
 */
#define CONFIG_SYS_GPIO_I2C_SCL		11
#define CONFIG_SYS_GPIO_I2C_SDA		10

#ifndef __ASSEMBLY__
int vct_gpio_dir(int pin, int dir);
void vct_gpio_set(int pin, int val);
int vct_gpio_get(int pin);
#endif

#define I2C_INIT	vct_gpio_dir(CONFIG_SYS_GPIO_I2C_SCL, 1)
#define I2C_ACTIVE	vct_gpio_dir(CONFIG_SYS_GPIO_I2C_SDA, 1)
#define I2C_TRISTATE	vct_gpio_dir(CONFIG_SYS_GPIO_I2C_SDA, 0)
#define I2C_READ	vct_gpio_get(CONFIG_SYS_GPIO_I2C_SDA)
#define I2C_SDA(bit)	vct_gpio_set(CONFIG_SYS_GPIO_I2C_SDA, bit)
#define I2C_SCL(bit)	vct_gpio_set(CONFIG_SYS_GPIO_I2C_SCL, bit)
#define I2C_DELAY	udelay(5)	/* 1/4 I2C clock duration */

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
/* CAT24WC32 */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2	/* Bytes of address		*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 5	/* The Catalyst CAT24WC32 has	*/
					/* 32 byte page write mode using*/
					/* last 5 bits of the address	*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10   /* and takes up to 10 msec */

#define CONFIG_BOOTCOMMAND	"run test3"
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

/*
 * UBI configuration
 */
#if defined(CONFIG_VCT_ONENAND)
#define CONFIG_SYS_USE_UBI
#define	CONFIG_CMD_JFFS2
#define	CONFIG_CMD_UBI
#define	CONFIG_RBTREE
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS

#define MTDIDS_DEFAULT		"onenand0=onenand"
#define MTDPARTS_DEFAULT	"mtdparts=onenand:128k(u-boot),"	\
					"128k(env),"		\
					"20m(kernel),"		\
					"-(rootfs)"
#endif

/*
 * We need a small, stripped down image to fit into the first 128k OneNAND
 * erase block (gzipped). This image only needs basic commands for FLASH
 * (NOR/OneNAND) usage and Linux kernel booting.
 */
#if defined(CONFIG_VCT_SMALL_IMAGE)
#undef CONFIG_CMD_ASKENV
#undef CONFIG_CMD_BEDBUG
#undef CONFIG_CMD_CACHE
#undef CONFIG_CMD_DHCP
#undef CONFIG_CMD_EEPROM
#undef CONFIG_CMD_EEPROM
#undef CONFIG_CMD_ELF
#undef CONFIG_CMD_FAT
#undef CONFIG_CMD_I2C
#undef CONFIG_CMD_I2C
#undef CONFIG_CMD_IRQ
#undef CONFIG_CMD_LOADY
#undef CONFIG_CMD_MII
#undef CONFIG_CMD_PING
#undef CONFIG_CMD_REGINFO
#undef CONFIG_CMD_SNTP
#undef CONFIG_CMD_STRINGS
#undef CONFIG_CMD_TERMINAL
#undef CONFIG_CMD_USB

#undef CONFIG_SMC911X
#undef CONFIG_SYS_I2C_SOFT
#undef CONFIG_SOURCE
#undef CONFIG_SYS_LONGHELP
#undef CONFIG_TIMESTAMP
#endif /* CONFIG_VCT_SMALL_IMAGE */

#endif  /* __CONFIG_H */

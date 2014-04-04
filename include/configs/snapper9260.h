/*
 * Bluewater Systems Snapper 9260 and 9G20 modules
 *
 * (C) Copyright 2011 Bluewater Systems
 *   Author: Andre Renaud <andre@bluewatersys.com>
 *   Author: Ryan Mallon <ryan@bluewatersys.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* SoC type is defined in boards.cfg */
#include <asm/hardware.h>
#include <linux/sizes.h>

#define CONFIG_SYS_TEXT_BASE		0x20000000

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000 /* External Crystal, in Hz */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768

/* CPU */
#define CONFIG_ARCH_CPU_INIT

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SKIP_RELOCATE_UBOOT
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_FIT

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CONFIG_SYS_SDRAM_SIZE		(64 * 1024 * 1024) /* 64MB */
#define CONFIG_SYS_INIT_SP_ADDR		(ATMEL_BASE_SRAM1 + 0x1000 - \
					 GENERATED_GBL_DATA_SIZE)

/* Mem test settings */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + (1024 * 1024))

/* NAND Flash */
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		ATMEL_BASE_CS3
#define CONFIG_SYS_NAND_DBW_8
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21) /* AD21 */
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22) /* AD22 */
#define CONFIG_SYS_NAND_ENABLE_PIN	AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN	AT91_PIN_PC13

/* Ethernet */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_RESET_PHY_R
#define CONFIG_AT91_WANTS_COMMON_PHY
#define CONFIG_TFTP_PORT
#define CONFIG_TFTP_TSIZE

/* USB */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_PLLB
#define CONFIG_USB_OHCI_NEW
#define CONFIG_DOS_PARTITION
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE	ATMEL_UHP_BASE
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"at91sam9260"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#define CONFIG_USB_STORAGE

/* GPIOs and IO expander */
#define CONFIG_ATMEL_LEGACY
#define CONFIG_AT91_GPIO
#define CONFIG_AT91_GPIO_PULLUP		1
#define CONFIG_PCA953X
#define CONFIG_SYS_I2C_PCA953X_ADDR	0x28
#define CONFIG_SYS_I2C_PCA953X_WIDTH	{ {0x28, 16} }

/* UARTs/Serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define CONFIG_USART_ID			ATMEL_ID_SYS
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_PROMPT		"Snapper> "

/* I2C - Bit-bashed */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SOFT		/* I2C bit-banged */
#define CONFIG_SYS_I2C_SOFT_SPEED	100000
#define CONFIG_SYS_I2C_SOFT_SLAVE	0x7F
#define CONFIG_SOFT_I2C_READ_REPEATED_START
#define I2C_INIT do {							\
		at91_set_gpio_output(AT91_PIN_PA23, 1);			\
		at91_set_gpio_output(AT91_PIN_PA24, 1);			\
		at91_set_pio_multi_drive(AT91_PIO_PORTA, 23, 1);	\
		at91_set_pio_multi_drive(AT91_PIO_PORTA, 24, 1);	\
	} while (0)
#define I2C_SOFT_DECLARATIONS
#define I2C_ACTIVE
#define I2C_TRISTATE	at91_set_gpio_input(AT91_PIN_PA23, 1);
#define I2C_READ	at91_get_gpio_value(AT91_PIN_PA23);
#define I2C_SDA(bit) do {						\
		if (bit) {						\
			at91_set_gpio_input(AT91_PIN_PA23, 1);		\
		} else {						\
			at91_set_gpio_output(AT91_PIN_PA23, 1);		\
			at91_set_gpio_value(AT91_PIN_PA23, bit);	\
		}							\
	} while (0)
#define I2C_SCL(bit)	at91_set_pio_value(AT91_PIO_PORTA, 24, bit)
#define I2C_DELAY	udelay(2)

/* Boot options */
#define CONFIG_SYS_LOAD_ADDR		0x23000000
#define CONFIG_BOOTDELAY		3
#define CONFIG_ZERO_BOOTDELAY_CHECK

#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/* Environment settings */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		(512 << 10)
#define CONFIG_ENV_SIZE			(256 << 10)
#define CONFIG_ENV_OVERWRITE
#define CONFIG_BOOTARGS			"console=ttyS0,115200 ip=any"

/* Console settings */
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE +		\
					 sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER

/* U-Boot memory settings */
#define CONFIG_SYS_MALLOC_LEN		(1 << 20)

/* Command line configuration */
#include <config_cmd_default.h>
#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_SOURCE

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#undef CONFIG_CMD_GPIO
#define CONFIG_CMD_USB
#define CONFIG_CMD_MII
#define CONFIG_CMD_NAND
#define CONFIG_CMD_PCA953X
#define CONFIG_CMD_PCA953X_INFO

#endif /* __CONFIG_H */

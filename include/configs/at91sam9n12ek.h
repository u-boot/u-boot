/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013 Atmel Corporation.
 * Josh Wu <josh.wu@atmel.com>
 *
 * Configuation settings for the AT91SAM9N12-EK boards.
 */

#ifndef __AT91SAM9N12_CONFIG_H_
#define __AT91SAM9N12_CONFIG_H_

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	16000000	/* main clock xtal */

/* Misc CPU related */

/* LCD */
#define LCD_BPP				LCD_COLOR16
#define LCD_OUTPUT_BPP			24
#define CONFIG_LCD_LOGO
#define CONFIG_LCD_INFO
#define CONFIG_LCD_INFO_BELOW_LOGO
#define CONFIG_ATMEL_LCD_RGB565

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x08000000

/*
 * Initial stack pointer: 4k - GENERATED_GBL_DATA_SIZE in internal SRAM,
 * leaving the correct space for initial global data structure above
 * that address while providing maximum stack area below.
 */
# define CONFIG_SYS_INIT_SP_ADDR \
	(0x00300000 + 16 * 1024 - GENERATED_GBL_DATA_SIZE)

/* DataFlash */

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN	GPIO_PIN_PD(4)
#define CONFIG_SYS_NAND_READY_PIN	GPIO_PIN_PD(5)
#endif

#define CONFIG_EXTRA_ENV_SETTINGS                                       \
	"console=console=ttyS0,115200\0"                                \
	"mtdparts="CONFIG_MTDPARTS_DEFAULT"\0"					\
	"bootargs_nand=rootfstype=ubifs ubi.mtd=7 root=ubi0:rootfs rw\0"\
	"bootargs_mmc=root=/dev/mmcblk0p2 rw rootfstype=ext4 rootwait\0"

/* USB host */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_PLLB
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE	ATMEL_BASE_OHCI
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"at91sam9n12"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	1
#endif

#ifdef CONFIG_SPI_BOOT

/* bootstrap + u-boot + env + linux in dataflash on CS0 */
#define CONFIG_BOOTCOMMAND						\
	"setenv bootargs ${console} ${mtdparts} ${bootargs_nand};"	\
	"sf probe 0; sf read 0x22000000 0x100000 0x300000; "		\
	"bootm 0x22000000"

#elif defined(CONFIG_NAND_BOOT)

/* bootstrap + u-boot + env + linux in nandflash */
#define CONFIG_BOOTCOMMAND						\
	"setenv bootargs ${console} ${mtdparts} ${bootargs_nand};"	\
	"nand read 0x21000000 0x180000 0x080000;"			\
	"nand read 0x22000000 0x200000 0x400000;"			\
	"bootm 0x22000000 - 0x21000000"

#else /* CONFIG_SD_BOOT */

#define CONFIG_BOOTCOMMAND						\
	"setenv bootargs ${console} ${mtdparts} ${bootargs_mmc};"	\
	"fatload mmc 0:1 0x21000000 dtb;"				\
	"fatload mmc 0:1 0x22000000 uImage;"				\
	"bootm 0x22000000 - 0x21000000"

#endif

/* SPL */
#define CONFIG_SPL_MAX_SIZE		0x6000
#define CONFIG_SPL_STACK		0x308000

#define CONFIG_SPL_BSS_START_ADDR	0x20000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000
#define CONFIG_SYS_SPL_MALLOC_START	0x20080000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000

#define CONFIG_SYS_MONITOR_LEN		(512 << 10)

#define CONFIG_SYS_MASTER_CLOCK		132096000
#define CONFIG_SYS_AT91_PLLA		0x20953f03
#define CONFIG_SYS_MCKR			0x1301
#define CONFIG_SYS_MCKR_CSS		0x1302

#ifdef CONFIG_SD_BOOT
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot.img"
#endif

#endif

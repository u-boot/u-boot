/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 David Lechner <david@lechnology.com>
 *
 * Based on da850evm.h
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Based on davinci_dvevm.h. Original Copyrights follow:
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * SoC Configuration
 */
#define CONFIG_SYS_EXCEPTION_VECTORS_HIGH
#define CONFIG_SYS_CLK_FREQ		clk_get(DAVINCI_ARM_CLKID)
#define CONFIG_SYS_OSCIN_FREQ		24000000
#define CONFIG_SYS_TIMERBASE		DAVINCI_TIMER0_BASE
#define CONFIG_SYS_HZ_CLOCK		clk_get(DAVINCI_AUXCLK_CLKID)

/*
 * Memory Info
 */
#define CONFIG_SYS_MALLOC_LEN	(0x10000 + 1*1024*1024) /* malloc() len */
#define PHYS_SDRAM_1		DAVINCI_DDR_EMIF_DATA_BASE /* DDR Start */
#define PHYS_SDRAM_1_SIZE	(64 << 20) /* SDRAM size 64MB */
#define CONFIG_MAX_RAM_BANK_SIZE (512 << 20) /* max size from SPRS586*/

/* memtest start addr */
#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM_1 + 0x2000000)

/* memtest will be run on 16MB */
#define CONFIG_SYS_MEMTEST_END 	(PHYS_SDRAM_1 + 0x2000000 + 16*1024*1024)

#define CONFIG_NR_DRAM_BANKS	1 /* we have 1 bank of DRAM */

/*
 * Serial Driver info
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4	/* NS16550 register size */
#define CONFIG_SYS_NS16550_COM1	DAVINCI_UART1_BASE /* Base address of UART1 */
#define CONFIG_SYS_NS16550_CLK	clk_get(DAVINCI_UART2_CLKID)

#define CONFIG_SYS_SPI_BASE		DAVINCI_SPI0_BASE
#define CONFIG_SYS_SPI_CLK		clk_get(DAVINCI_SPI0_CLKID)
#define CONFIG_SF_DEFAULT_SPEED		50000000
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED

/*
 * I2C Configuration
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_DAVINCI
#define CONFIG_SYS_DAVINCI_I2C_SPEED		400000
#define CONFIG_SYS_DAVINCI_I2C_SLAVE   10 /* Bogus, master-only in U-Boot */

/*
 * U-Boot general configuration
 */
#define CONFIG_BOOTFILE		"uImage" /* Boot file name */
#define CONFIG_SYS_CBSIZE	1024 /* Console I/O Buffer Size	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Args Buffer Size */
#define CONFIG_SYS_LOAD_ADDR	(PHYS_SDRAM_1 + 0x700000)
#define CONFIG_MX_CYCLIC

/*
 * Linux Information
 */
#define LINUX_BOOT_PARAM_ADDR	(PHYS_SDRAM_1 + 0x100)
#define CONFIG_HWCONFIG		/* enable hwconfig */
#define CONFIG_CMDLINE_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SERIAL_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_SETUP_INITRD_TAG
#define CONFIG_BOOTCOMMAND \
	"if mmc rescan; then " \
		"if run loadbootscr; then " \
			"run bootscript; " \
		"else " \
			"if run loadimage; then " \
				"run mmcargs; " \
				"run mmcboot; " \
			"else " \
				"run flashargs; " \
				"run flashboot; " \
			"fi; " \
		"fi; " \
	"else " \
		"run flashargs; " \
		"run flashboot; " \
	"fi"
#define CONFIG_EXTRA_ENV_SETTINGS \
	"memsize=64M\0" \
	"filesyssize=10M\0" \
	"verify=n\0" \
	"console=ttyS1,115200n8\0" \
	"bootscraddr=0xC0600000\0" \
	"loadaddr=0xC0007FC0\0" \
	"filesysaddr=0xC1180000\0" \
	"fwupdateboot=mw 0xFFFF1FFC 0x5555AAAA; reset\0" \
	"mmcargs=setenv bootargs mem=${memsize} console=${console} root=/dev/mmcblk0p2 rw rootwait lpj=747520\0" \
	"mmcboot=bootm ${loadaddr}\0" \
	"flashargs=setenv bootargs mem=${memsize} initrd=${filesysaddr},${filesyssize} root=/dev/ram0 rw rootfstype=squashfs console=${console} lpj=747520\0" \
	"flashboot=sf probe 0; sf read ${loadaddr} 0x50000 0x400000; sf read ${filesysaddr} 0x450000 0xA00000; bootm ${loadaddr}\0" \
	"loadimage=fatload mmc 0 ${loadaddr} uImage\0" \
	"loadbootscr=fatload mmc 0 ${bootscraddr} boot.scr\0" \
	"bootscript=source ${bootscraddr}\0" \

#ifdef CONFIG_CMD_BDI
#define CONFIG_CLOCKS
#endif

#define CONFIG_ENV_SIZE		(16 << 10)

/* additions for new relocation code, must added to all boards */
#define CONFIG_SYS_SDRAM_BASE		0xc0000000

#define CONFIG_SYS_INIT_SP_ADDR		0x80010000

#include <asm/arch/hardware.h>

#endif /* __CONFIG_H */

/*
 * Copyright (C) 2008 Texas Instruments, Inc <www.ti.com>
 *
 * Based on davinci_dvevm.h. Original Copyrights follow:
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/sizes.h>
#include <asm/arch/hardware.h>
#include <asm/arch/clock.h>

/* Architecture, CPU, etc */
#define CONFIG_ARM1176
#define CONFIG_TNETV107X
#define CONFIG_TNETV107X_EVM
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_DISABLE_TCM
#define CONFIG_PERIPORT_REMAP
#define CONFIG_PERIPORT_BASE		0x2000000
#define CONFIG_PERIPORT_SIZE		0x10
#define CONFIG_SYS_CLK_FREQ		clk_get_rate(TNETV107X_LPSC_ARM)

#define CONFIG_SYS_TIMERBASE		TNETV107X_TIMER0_BASE
#define CONFIG_SYS_HZ_CLOCK		clk_get_rate(TNETV107X_LPSC_TIMER0)
#define CONFIG_SYS_HZ			1000

#define CONFIG_PLL_SYS_EXT_FREQ		25000000
#define CONFIG_PLL_TDM_EXT_FREQ		19200000
#define CONFIG_PLL_ETH_EXT_FREQ		25000000

/* Memory Info */
#define CONFIG_SYS_MALLOC_LEN		(0x10000 + 1*1024*1024)
#define PHYS_SDRAM_1			TNETV107X_DDR_EMIF_DATA_BASE
#define PHYS_SDRAM_1_SIZE		0x04000000
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_1 + 16*1024*1024)
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_STACKSIZE		(256*1024)

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

/* Serial Driver Info */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#define CONFIG_SYS_NS16550_COM1		TNETV107X_UART1_BASE
#define CONFIG_SYS_NS16550_CLK		clk_get_rate(TNETV107X_LPSC_UART1)
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* Flash and environment info */
#define CONFIG_SYS_NO_FLASH
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_NAND_DAVINCI
#define CONFIG_ENV_SIZE			(SZ_128K)
#define CONFIG_SYS_NAND_HW_ECC
#define CONFIG_SYS_NAND_1BIT_ECC
#define CONFIG_SYS_NAND_CS		2
#define CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_NAND_BASE		TNETV107X_ASYNC_EMIF_DATA_CE0_BASE
#define CONFIG_SYS_CLE_MASK		0x10
#define CONFIG_SYS_ALE_MASK		0x8
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE
#define CONFIG_JFFS2_NAND
#define NAND_MAX_CHIPS			1
#define CONFIG_ENV_OFFSET		0x180000

/*
 * davinci_nand is a bit of a misnomer since this particular EMIF block is
 * commonly used across multiple TI devices.  Unfortunately, this misnomer
 * (amongst others) carries forward into the kernel too.  Consequently, if we
 * use a different device name here, the mtdparts variable won't be usable as
 * a kernel command-line argument.
 */
#define MTDIDS_DEFAULT			"nand0=davinci_nand.0"
#define MTDPARTS_DEFAULT		"mtdparts=davinci_nand.0:"	\
						"1536k(uboot)ro,"	\
						"128k(params)ro,"	\
						"4m(kernel),"		\
						"-(filesystem)"

/* General U-Boot configuration */
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_SYS_PROMPT		"U-Boot > "
#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP
#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE +		\
					 sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_MEMTEST_START +	\
					 0x700000)
#define LINUX_BOOT_PARAM_ADDR		(CONFIG_SYS_MEMTEST_START + 0x100)
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_BOOTARGS			"mem=32M console=ttyS1,115200n8 " \
					"root=/dev/mmcblk0p1 rw noinitrd"
#define CONFIG_BOOTCOMMAND		""
#define CONFIG_BOOTDELAY		1

#define CONFIG_CMD_BDI
#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_CONSOLE
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_EDITENV
#define CONFIG_CMD_IMI
#define CONFIG_CMD_ITEST
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MISC
#define CONFIG_CMD_RUN
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_SOURCE
#define CONFIG_CMD_ENV
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_NAND
#define CONFIG_CMD_JFFS2

#endif /* __CONFIG_H */

/*
 * Rick Bronson <rick@efn.org>
 *
 * Configuation settings for the AT91RM9200DK board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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

/* ARM asynchronous clock */
#define AT91C_MAIN_CLOCK  179712000  /* from 18.432 MHz crystal (18432000 / 4 * 39) */
#define AT91C_MASTER_CLOCK  59904000  /* peripheral clock (AT91C_MASTER_CLOCK / 3) */
/* #define AT91C_MASTER_CLOCK  44928000 */  /* peripheral clock (AT91C_MASTER_CLOCK / 4) */

#define CONFIG_AT91RM9200DK	 1	/* on an AT91RM9200DK Board      */
#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */
#define CONFIG_CMDLINE_TAG	 1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	 1

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN	(CFG_ENV_SIZE + 128*1024)
#define CONFIG_BAUDRATE 115200
/*
 * Hardware drivers
 */

#undef	CONFIG_HWFLOW			/* don't include RTS/CTS flow control support	*/

#undef	CONFIG_MODEM_SUPPORT		/* disable modem initialization stuff */

#define CONFIG_BOOTDELAY      3  
/* #define CONFIG_ENV_OVERWRITE  1 */

#define CONFIG_COMMANDS		\
		       ((CONFIG_CMD_DFL	| \
			CFG_CMD_DHCP ) & \
                      ~(CFG_CMD_BDI | \
                        CFG_CMD_IMI | \
                        CFG_CMD_AUTOSCRIPT | \
                        CFG_CMD_FPGA | \
                        CFG_CMD_MISC | \
                        CFG_CMD_LOADS ))
                     
/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CFG_MAX_NAND_DEVICE	1	/* Max number of NAND devices		*/
#define SECTORSIZE 512

#define ADDR_COLUMN 1
#define ADDR_PAGE 2
#define ADDR_COLUMN_PAGE 3

#define NAND_ChipID_UNKNOWN 	0x00
#define NAND_MAX_FLOORS 1
#define NAND_MAX_CHIPS 1

#define AT91_SMART_MEDIA_ALE (1 << 22)  /* our ALE is AD22 */
#define AT91_SMART_MEDIA_CLE (1 << 21)  /* our CLE is AD21 */

#define NAND_DISABLE_CE(nand) do { *AT91C_PIOC_SODR = AT91C_PIO_PC0;} while(0)
#define NAND_ENABLE_CE(nand) do { *AT91C_PIOC_CODR = AT91C_PIO_PC0;} while(0)

#define NAND_WAIT_READY(nand) while (!(*AT91C_PIOC_PDSR & AT91C_PIO_PC2))

#define WRITE_NAND_COMMAND(d, adr) do{ *(volatile __u8 *)((unsigned long)adr | AT91_SMART_MEDIA_CLE) = (__u8)(d); } while(0)
#define WRITE_NAND_ADDRESS(d, adr) do{ *(volatile __u8 *)((unsigned long)adr | AT91_SMART_MEDIA_ALE) = (__u8)(d); } while(0)
#define WRITE_NAND(d, adr) do{ *(volatile __u8 *)((unsigned long)adr) = (__u8)d; } while(0)
#define READ_NAND(adr) ((volatile unsigned char)(*(volatile __u8 *)(unsigned long)adr))
/* the following are NOP's in our implementation */
#define NAND_CTL_CLRALE(nandptr)
#define NAND_CTL_SETALE(nandptr)
#define NAND_CTL_CLRCLE(nandptr)
#define NAND_CTL_SETCLE(nandptr)

#define CONFIG_NR_DRAM_BANKS 1
#define PHYS_SDRAM 0x20000000
#define PHYS_SDRAM_SIZE 0x2000000  /* 32 megs */

#define CFG_MEMTEST_START PHYS_SDRAM
#define CFG_MEMTEST_END   CFG_MEMTEST_START + PHYS_SDRAM_SIZE - 262144

#define CONFIG_DRIVER_ETHER
#define CONFIG_NET_RETRY_COUNT 20

#define CONFIG_HAS_DATAFLASH	1
#define CFG_SPI_WRITE_TOUT	CFG_HZ
#define CFG_MAX_DATAFLASH_BANKS 2
#define CFG_MAX_DATAFLASH_PAGES 16384
#define CFG_DATAFLASH_LOGIC_ADDR_CS0	0xC0000000	/* Logical adress for CS0 */
#define CFG_DATAFLASH_LOGIC_ADDR_CS3	0xD0000000	/* Logical adress for CS3 */

#define PHYS_FLASH_1 0x10000000
#define PHYS_FLASH_SIZE 0x200000  /* 2 megs main flash */
#define CFG_FLASH_BASE		PHYS_FLASH_1
#define CFG_MAX_FLASH_BANKS 1
#define CFG_MAX_FLASH_SECT 40
#define CFG_FLASH_ERASE_TOUT	(2*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2*CFG_HZ) /* Timeout for Flash Write */
#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR (PHYS_FLASH_1 + 0xe000)  /* 0x10000 */
#define CFG_ENV_SIZE 0x2000  /* 0x8000 */
#define CFG_LOAD_ADDR 0x21000000  /* default load address */

#define CFG_BOOT_SIZE		0x6000 /* 24 KBytes */
#define CFG_U_BOOT_BASE 	(PHYS_FLASH_1 + 0x10000)
#define CFG_U_BOOT_SIZE		0x10000	/* 64 KBytes */

#define CFG_BAUDRATE_TABLE {115200 , 19200, 38400, 57600, 9600 }

#define CFG_PROMPT "Uboot> " /* Monitor Command Prompt */
#define	CFG_CBSIZE 256 /* Console I/O Buffer Size */
#define CFG_MAXARGS 16 /* max number of command args */
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */

#ifndef __ASSEMBLY__
/*-----------------------------------------------------------------------
 * Board specific extension for bd_info
 *
 * This structure is embedded in the global bd_info (bd_t) structure
 * and can be used by the board specific code (eg board/...)
 */

struct bd_info_ext
{
    /* helper variable for board environment handling
     *
     * env_crc_valid == 0    =>   uninitialised
     * env_crc_valid  > 0    =>   environment crc in flash is valid
     * env_crc_valid  < 0    =>   environment crc in flash is invalid
     */
     int	env_crc_valid;
};
#endif

#define	CFG_HZ AT91C_MASTER_CLOCK/2  /* AT91C_TC0_CMR is implicitly set to
                                        AT91C_TC_TIMER_DIV1_CLOCK */

#define CONFIG_STACKSIZE	(32*1024)	/* regular stack */

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ not supported
#endif

#endif

/*
 * Copyright (C) 2004 Sascha Hauer, Pengutronix
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_ARM920T	   1	 /* this is an ARM920T CPU     */
#define CONFIG_IMX	   1	 /* in a Motorola MC9328MXL Chip */
#define CONFIG_MX1FS2	   1	 /* on a mx1fs2 board */
#undef	CONFIG_USE_IRQ		 /* don't need use IRQ/FIQ    */

/*
 * Select serial console configuration
 */
#undef	_CONFIG_UART1 /* internal uart 1 */
#define _CONFIG_UART2 /* internal uart 2 */
#undef	_CONFIG_UART3 /* internal uart 3 */
#undef	_CONFIG_UART4 /* internal uart 4 */
#undef	CONFIG_SILENT_CONSOLE  /* use this to disable output */


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_JFFS2

#undef CONFIG_CMD_CONSOLE
#undef CONFIG_CMD_DHCP
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_PING
#undef CONFIG_CMD_SOURCE


/*
 * Boot options. Setting delay to -1 stops autostart count down.
 */
#define CONFIG_BOOTDELAY   10
#define CONFIG_BOOTARGS	   "root=/dev/mtdblock4 console=ttySMX0,115200n8 rootfstype=jffs2"
#define CONFIG_BOOTCOMMAND "bootm 10080000"
#define CONFIG_SHOW_BOOT_PROGRESS

/*
 * General options for u-boot. Modify to save memory foot print
 */
#define CONFIG_SYS_LONGHELP				      /* undef saves memory  */
#define CONFIG_SYS_PROMPT		"mx1fs2> "	      /* prompt string	     */
#define CONFIG_SYS_CBSIZE		256		      /* console I/O buffer  */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* print buffer size   */
#define CONFIG_SYS_MAXARGS		16		      /* max command args    */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	      /* boot args buf size  */

#define CONFIG_SYS_MEMTEST_START	0x08100000	      /* memtest test area   */
#define CONFIG_SYS_MEMTEST_END		0x08F00000

#define CONFIG_SYS_HZ			3686400	     /* incrementer freq: 3.6864 MHz */
#define CONFIG_SYS_CPUSPEED		0x141	     /* core clock - register value  */

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_BAUDRATE 115200
/*
 * Definitions related to passing arguments to kernel.
 */
#define CONFIG_CMDLINE_TAG	     1	 /* send commandline to Kernel	     */
#define CONFIG_SETUP_MEMORY_TAGS     1	 /* send memory definition to kernel */
#define	 CONFIG_INITRD_TAG	     1	   /* send initrd params	*/
#undef	CONFIG_VFD			 /* do not send framebuffer setup    */

/*
 * Malloc pool need to host env + 128 Kb reserve for other allocations.
 */
#define CONFIG_SYS_MALLOC_LEN	  (CONFIG_ENV_SIZE + (128<<10) )


#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

#define CONFIG_STACKSIZE	(120<<10)      /* stack size */

#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4<<10)	       /* IRQ stack  */
#define CONFIG_STACKSIZE_FIQ	(4<<10)	       /* FIQ stack  */
#endif

/* SDRAM Setup Values
 * 0x910a8300 Precharge Command CAS 3
 * 0x910a8200 Precharge Command CAS 2
 *
 * 0xa10a8300 AutoRefresh Command CAS 3
 * 0xa10a8200 Set AutoRefresh Command CAS 2
 */
#define PRECHARGE_CMD 0x910a8300
#define AUTOREFRESH_CMD 0xa10a8300

#define BUS32BIT_VERSION
/*
 * SDRAM Memory Map
 */
#define CONFIG_NR_DRAM_BANKS  1			     /* we have 1 bank of SDRAM */
#define MX1FS2_SDRAM_1	      0x08000000	     /* SDRAM bank #1		*/
#ifdef BUS32BIT_VERSION
#define MX1FS2_SDRAM_1_SIZE  (0x04000000 - 0x100000) /* 64 MB - 1M Framebuffer */
#else
#define MX1FS2_SDRAM_1_SIZE  (0x01FC0000 - 0x100000) /* 32 MB - 1M Framebuffer */
#endif
/*
 * Flash Controller settings
 */

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* FLASH banks count (not chip count)*/
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* number of sector in FLASH bank    */

#ifdef BUS32BIT_VERSION
#define MX1FS2_FLASH_BUS_WIDTH	4	/* we use 32 bit FLASH memory...     */
#define MX1FS2_FLASH_INTERLEAVE 2	/* ... made of 2 chips */
#define MX1FS2_FLASH_BANK_SIZE	0x02000000  /* size of one flash bank*/
#define MX1FS2_FLASH_SECT_SIZE	0x00020000  /* size of erase sector */
#else
#define MX1FS2_FLASH_BUS_WIDTH	2	/* we use 16 bit FLASH memory...     */
#define MX1FS2_FLASH_INTERLEAVE 1	/* ... made of 1 chip */
#define MX1FS2_FLASH_BANK_SIZE	0x01000000  /* size of one flash bank*/
#define MX1FS2_FLASH_SECT_SIZE	0x00010000  /* size of erase sector */
#endif
#define MX1FS2_FLASH_BASE	0x10000000  /* location of flash memory */
#define MX1FS2_FLASH_UNLOCK	   1	   /* perform hw unlock first */

/* This should be defined if CFI FLASH device is present. Actually benefit
   is not so clear to me. In other words we can provide more informations
   to user, but this expects more complex flash handling we do not provide
   now.*/
#undef	CONFIG_SYS_FLASH_CFI

#define CONFIG_SYS_FLASH_ERASE_TOUT	(2*CONFIG_SYS_HZ)    /* timeout for Erase operation */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2*CONFIG_SYS_HZ)    /* timeout for Write operation */

#define CONFIG_SYS_FLASH_BASE		MX1FS2_FLASH_BASE

/*
 * This is setting for JFFS2 support in u-boot.
 * Right now there is no gain for user, but later on booting kernel might be
 * possible. Consider using XIP kernel running from flash to save RAM
 * footprint.
 * NOTE: Enable CONFIG_CMD_JFFS2 for JFFS2 support.
 */

/*
 * JFFS2 partitions
 */
/* No command line, one static partition, whole device */
/*
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00050000
*/

/* mtdparts command line support */
/* Note: fake mtd_id used, no linux mtd map file */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define CONFIG_FLASH_CFI_MTD
#define MTDIDS_DEFAULT		"nor0=mx1fs2-0"

#ifdef BUS32BIT_VERSION
#define MTDPARTS_DEFAULT	"mtdparts=mx1fs2-0:2m@5m(part0),5m@9m(part1)"
#else
#define MTDPARTS_DEFAULT	"mtdparts=mx1fs2-0:-@320k(jffs2)"
#endif

/*
 * Environment setup. Definitions of monitor location and size with
 * definition of environment setup ends up in 2 possibilities.
 * 1. Embeded environment - in u-boot code is space for environment
 * 2. Environment is read from predefined sector of flash
 * Right now we support 2. possiblity, but expecting no env placed
 * on mentioned address right now. This also needs to provide whole
 * sector for it - for us 256Kb is really waste of memory. U-boot uses
 * default env. and until kernel parameters could be sent to kernel
 * env. has no sense to us.
 */

#define CONFIG_SYS_MONITOR_BASE	0x10000000
#define CONFIG_SYS_MONITOR_LEN		0x20000		/* 128b ( 1 flash sector )   */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		0x10020000	/* absolute address for now  */
#define CONFIG_ENV_SIZE		0x20000

#define	 CONFIG_ENV_OVERWRITE	1		/* env is not writable now   */

/* Setup CS4 and CS5 */
#define CONFIG_SYS_GIUS_A_VAL		0x0003fffe

/*
 * CSxU_VAL:
 * 63| x	x x x | x x x x | x x  x    x | x x x x | x x x x | x x x x | x x x x | x x x x|32
 *   |DTACK_SEL|0|BCD |	  BCS	| PSZ|PME|SYNC|	 DOL	| CNC|	  WSC	    | 0| WWS  |	  EDC  |
 *
 * CSxL_VAL:
 * 31| x x x x | x x x x | x x x x | x x x x | x x x x |  x x x x | x x	 x x | x x  x	 x| 0
 *   |	 OEA   |   OEN	 |   WEA   |   WEN   |	 CSA   |EBC| DSZ  | 0|SP|0|WP| 0 0|PA|CSEN|
 */

#define CONFIG_SYS_CS0U_VAL 0x00008C00
#define CONFIG_SYS_CS0L_VAL 0x22222601
#define CONFIG_SYS_CS1U_VAL 0x00008C00
#define CONFIG_SYS_CS1L_VAL 0x22222301
#define CONFIG_SYS_CS4U_VAL 0x00008C00
#define CONFIG_SYS_CS4L_VAL 0x22222301
#define CONFIG_SYS_CS5U_VAL 0x00008C00
#define CONFIG_SYS_CS5L_VAL 0x22222301

/* f_{dpll}=2*f{ref}*(MFI+MFN/(MFD+1))/(PD+1)
   f_ref=16,777MHz

   0x002a141f: 191,9944MHz
   0x040b2007: 144MHz
   0x042a141f: 96MHz
   0x0811140d: 64MHz
   0x040e200e: 150MHz
   0x00321431: 200MHz

   0x08001800: 64MHz mit 16er Quarz
   0x04001800: 96MHz mit 16er Quarz
   0x04002400: 144MHz mit 16er Quarz

   31 |x x x x|x x x x|x x x x|x x x x|x x x x|x x x x|x x x x|x x x x| 0
      |XXX|--PD---|-------MFD---------|XXX|--MFI--|-----MFN-----------|	    */

#define CONFIG_SYS_MPCTL0_VAL		0x07E723AD
#define CONFIG_SYS_MPCTL1_VAL		0x00000040
#define CONFIG_SYS_PCDR_VAL		0x00010005
#define CONFIG_SYS_GPCR_VAL		0x00000FFB

#define USE_16M_OSZI /* If you have one, you want to use it
			The internal 32kHz oszillator jitters */
#ifdef USE_16M_OSZI

#define CONFIG_SYS_SPCTL0_VAL		0x04001401
#define CONFIG_SYS_SPCTL1_VAL		0x0C000040
#define CONFIG_SYS_CSCR_VAL		0x07030003
#define CONFIG_SYS_CLK_FREQ	16780000
#define CONFIG_SYSPLL_CLK_FREQ	16000000

#else

#define CONFIG_SYS_SPCTL0_VAL		0x07E716D1
#define CONFIG_SYS_CSCR_VAL		0x06000003
#define CONFIG_SYS_CLK_FREQ	16780000
#define CONFIG_SYSPLL_CLK_FREQ	16780000

#endif

/*
 * Well this has to be defined, but on the other hand it is used differently
 * one may expect. For instance loadb command do not cares :-)
 * So advice is - do not relay on this...
 */
#define CONFIG_SYS_LOAD_ADDR		0x08400000

#define CONFIG_SYS_FMCR_VAL		0x00000003 /* Reset Default */

/* Bit[0:3] contain PERCLK1DIV for UART 1
   0x000b00b ->b<- -> 192MHz/12=16MHz
   0x000b00b ->8<- -> 144MHz/09=16MHz
   0x000b00b ->3<- -> 64MHz/4=16MHz */

#ifdef _CONFIG_UART1
#define CONFIG_IMX_SERIAL
#define CONFIG_IMX_SERIAL1
#elif defined _CONFIG_UART2
#define CONFIG_IMX_SERIAL
#define CONFIG_IMX_SERIAL2
#elif defined _CONFIG_UART3 | defined _CONFIG_UART4
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		3686400
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_CONS_INDEX	1
#ifdef _CONFIG_UART3
#define CONFIG_SYS_NS16550_COM1	0x15000000
#elif defined _CONFIG_UART4
#define CONFIG_SYS_NS16550_COM1	0x16000000
#endif
#endif

#endif	/* __CONFIG_H */

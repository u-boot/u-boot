/*
 * Copyright (C) 2003 ETC s.r.o.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 * Written by Peter Figuli <peposh@etc.sk>, 2003.
 *
 * 2003/13/06 Initial MP10 Support copied from wepep250
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_IMX		1     /* This is a Motorola MC9328MXL Chip */
#define CONFIG_SCB9328		1     /* on a scb9328tronix board */

#define CONFIG_IMX_SERIAL
#define CONFIG_IMX_SERIAL1
/*
 * Select serial console configuration
 */

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

#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP

#undef CONFIG_CMD_CONSOLE
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_SOURCE

/*
 * Boot options. Setting delay to -1 stops autostart count down.
 * NOTE: Sending parameters to kernel depends on kernel version and
 * 2.4.19-rmk6-pxa1 patch used while my u-boot coding didn't accept
 * parameters at all! Do not get confused by them so.
 */
#define CONFIG_BOOTDELAY   -1
#define CONFIG_BOOTARGS	   "console=ttySMX0,115200n8 root=/dev/mtdblock3 rootfstype=jffs2 mtdparts=scb9328_flash:128k(U-boot)ro,128k(U-boot_env),1m(kernel),4m(root),4m(fs) eval_board=evk9328"
#define CONFIG_BOOTCOMMAND "bootm 10040000"
#define CONFIG_SHOW_BOOT_PROGRESS
#define CONFIG_ETHADDR		80:81:82:83:84:85
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		10.10.10.9
#define CONFIG_SERVERIP		10.10.10.10

/*
 * General options for u-boot. Modify to save memory foot print
 */
#define CONFIG_SYS_LONGHELP				      /* undef saves memory  */
#define CONFIG_SYS_PROMPT		"scb9328> "	      /* prompt string	     */
#define CONFIG_SYS_CBSIZE		256		      /* console I/O buffer  */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* print buffer size   */
#define CONFIG_SYS_MAXARGS		16		      /* max command args    */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	      /* boot args buf size  */

#define CONFIG_SYS_MEMTEST_START	0x08100000	      /* memtest test area   */
#define CONFIG_SYS_MEMTEST_END		0x08F00000

#define CONFIG_SYS_CPUSPEED		0x141	     /* core clock - register value  */

#define CONFIG_BAUDRATE 115200
/*
 * Definitions related to passing arguments to kernel.
 */
#define CONFIG_CMDLINE_TAG	     1	 /* send commandline to Kernel	     */
#define CONFIG_SETUP_MEMORY_TAGS     1	 /* send memory definition to kernel */
#define CONFIG_INITRD_TAG	     1	 /* send initrd params		     */

/*
 * Malloc pool need to host env + 128 Kb reserve for other allocations.
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (128<<10) )

/* SDRAM Setup Values
0x910a8300 Precharge Command CAS 3
0x910a8200 Precharge Command CAS 2

0xa10a8300 AutoRefresh Command CAS 3
0xa10a8200 Set AutoRefresh Command CAS 2 */

#define PRECHARGE_CMD 0x910a8200
#define AUTOREFRESH_CMD 0xa10a8200

/*
 * SDRAM Memory Map
 */
/* SH FIXME */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of SDRAM */
#define SCB9328_SDRAM_1		0x08000000	/* SDRAM bank #1	   */
#define SCB9328_SDRAM_1_SIZE	0x01000000	/* 16 MB		   */

#define CONFIG_SYS_TEXT_BASE	0x10000000

#define CONFIG_SYS_SDRAM_BASE	SCB9328_SDRAM_1
#define CONFIG_SYS_INIT_SP_ADDR	(SCB9328_SDRAM_1 + 0xf00000)

/*
 * Configuration for FLASH memory for the Synertronixx board
 */

/* #define SCB9328_FLASH_32M */

/* 32MB */
#ifdef SCB9328_FLASH_32M
#define CONFIG_SYS_MAX_FLASH_BANKS		1	/* FLASH banks count (not chip count)*/
#define CONFIG_SYS_MAX_FLASH_SECT		256	/* number of sector in FLASH bank    */
#define SCB9328_FLASH_BUS_WIDTH		2	/* we use 16 bit FLASH memory...     */
#define SCB9328_FLASH_INTERLEAVE	1	/* ... made of 1 chip		     */
#define SCB9328_FLASH_BANK_SIZE	 0x02000000	/* size of one flash bank	     */
#define SCB9328_FLASH_SECT_SIZE	 0x00020000	/* size of erase sector		     */
#define SCB9328_FLASH_BASE	 0x10000000	/* location of flash memory	     */
#define SCB9328_FLASH_UNLOCK		1	/* perform hw unlock first	     */
#else

/* 16MB */
#define CONFIG_SYS_MAX_FLASH_BANKS		1	/* FLASH banks count (not chip count)*/
#define CONFIG_SYS_MAX_FLASH_SECT		128	/* number of sector in FLASH bank    */
#define SCB9328_FLASH_BUS_WIDTH		2	/* we use 16 bit FLASH memory...     */
#define SCB9328_FLASH_INTERLEAVE	1	/* ... made of 1 chip		     */
#define SCB9328_FLASH_BANK_SIZE	 0x01000000	/* size of one flash bank	     */
#define SCB9328_FLASH_SECT_SIZE	 0x00020000	/* size of erase sector		     */
#define SCB9328_FLASH_BASE	 0x10000000	/* location of flash memory	     */
#define SCB9328_FLASH_UNLOCK		1	/* perform hw unlock first	     */
#endif /* SCB9328_FLASH_32M */

/* This should be defined if CFI FLASH device is present. Actually benefit
   is not so clear to me. In other words we can provide more informations
   to user, but this expects more complex flash handling we do not provide
   now.*/
#undef	CONFIG_SYS_FLASH_CFI

#define CONFIG_SYS_FLASH_ERASE_TOUT	240000    /* timeout for Erase operation */
#define CONFIG_SYS_FLASH_WRITE_TOUT	240000    /* timeout for Write operation */

#define CONFIG_SYS_FLASH_BASE		SCB9328_FLASH_BASE

/*
 * This is setting for JFFS2 support in u-boot.
 * Right now there is no gain for user, but later on booting kernel might be
 * possible. Consider using XIP kernel running from flash to save RAM
 * footprint.
 * NOTE: Enable CONFIG_CMD_JFFS2 for JFFS2 support.
 */
#define CONFIG_SYS_JFFS2_FIRST_BANK		0
#define CONFIG_SYS_JFFS2_FIRST_SECTOR		5
#define CONFIG_SYS_JFFS2_NUM_BANKS		1

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

/* Setup for PA23 which is Reset Default PA23 but has to become
   CS5 */

#define CONFIG_SYS_GPR_A_VAL		0x00800000
#define CONFIG_SYS_GIUS_A_VAL		0x0043fffe

#define CONFIG_SYS_MONITOR_BASE	0x10000000
#define CONFIG_SYS_MONITOR_LEN		0x20000		/* 128b ( 1 flash sector )  */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		0x10020000	/* absolute address for now  */
#define CONFIG_ENV_SIZE		0x20000

#define	 CONFIG_ENV_OVERWRITE  1		/* env is not writable now   */

/*
 * CSxU_VAL:
 * 63| x	x x x | x x x x | x x  x    x | x x x x | x x x x | x x x x | x x x x | x x x x|32
 *   |DTACK_SEL|0|BCD |	  BCS	| PSZ|PME|SYNC|	 DOL	| CNC|	  WSC	    | 0| WWS  |	  EDC  |
 *
 * CSxL_VAL:
 * 31| x x x x | x x x x | x x x x | x x x x | x x x x |  x x x x | x x	 x x | x x  x	 x| 0
 *   |	 OEA   |   OEN	 |   WEA   |   WEN   |	 CSA   |EBC| DSZ  | 0|SP|0|WP| 0 0|PA|CSEN|
 */

#define CONFIG_SYS_CS0U_VAL 0x000F2000
#define CONFIG_SYS_CS0L_VAL 0x11110d01
#define CONFIG_SYS_CS1U_VAL 0x000F0a00
#define CONFIG_SYS_CS1L_VAL 0x11110601
#define CONFIG_SYS_CS2U_VAL 0x0
#define CONFIG_SYS_CS2L_VAL 0x0

#define CONFIG_SYS_CS3U_VAL 0x000FFFFF
#define CONFIG_SYS_CS3L_VAL 0x00000303

#define CONFIG_SYS_CS4U_VAL 0x000F0a00
#define CONFIG_SYS_CS4L_VAL 0x11110301

/* CNC == 3 too long
   #define CONFIG_SYS_CS5U_VAL 0x0000C210 */

/* #define CONFIG_SYS_CS5U_VAL 0x00008400
   mal laenger mahcen, ob der bei 150MHz laenger haelt dann und
   kaum langsamer ist */
/* #define CONFIG_SYS_CS5U_VAL 0x00009400
   #define CONFIG_SYS_CS5L_VAL 0x11010D03 */

#define CONFIG_SYS_CS5U_VAL 0x00008400
#define CONFIG_SYS_CS5L_VAL 0x00000D03

#define CONFIG_DRIVER_DM9000		1
#define CONFIG_DM9000_BASE		0x16000000
#define DM9000_IO			CONFIG_DM9000_BASE
#define DM9000_DATA			(CONFIG_DM9000_BASE+4)

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

#define CPU200

#ifdef CPU200
#define CONFIG_SYS_MPCTL0_VAL 0x00321431
#else
#define CONFIG_SYS_MPCTL0_VAL 0x040e200e
#endif

/* #define BUS64 */
#define BUS72

#ifdef BUS72
#define CONFIG_SYS_SPCTL0_VAL 0x04002400
#endif

#ifdef BUS96
#define CONFIG_SYS_SPCTL0_VAL 0x04001800
#endif

#ifdef BUS64
#define CONFIG_SYS_SPCTL0_VAL 0x08001800
#endif

/* Das ist der BCLK Divider, der aus der System PLL
   BCLK und HCLK erzeugt:
   31 | xxxx xxxx xxxx xxxx xx10 11xx xxxx xxxx | 0
   0x2f008403 : 192MHz/2=96MHz, 144MHz/2=72MHz PRESC=1->BCLKDIV=2
   0x2f008803 : 192MHz/3=64MHz, 240MHz/3=80MHz PRESC=1->BCLKDIV=2
   0x2f001003 : 192MHz/5=38,4MHz
   0x2f000003 : 64MHz/1
   Bit 22: SPLL Restart
   Bit 21: MPLL Restart */

#ifdef BUS64
#define CONFIG_SYS_CSCR_VAL 0x2f030003
#endif

#ifdef BUS72
#define CONFIG_SYS_CSCR_VAL 0x2f030403
#endif

/*
 * Well this has to be defined, but on the other hand it is used differently
 * one may expect. For instance loadb command do not cares :-)
 * So advice is - do not relay on this...
 */
#define CONFIG_SYS_LOAD_ADDR 0x08400000

#define MHZ16QUARZINUSE

#ifdef MHZ16QUARZINUSE
#define CONFIG_SYSPLL_CLK_FREQ 16000000
#else
#define CONFIG_SYSPLL_CLK_FREQ 16780000
#endif

#define CONFIG_SYS_CLK_FREQ 16780000

/* FMCR Bit 0 becomes 0 to make CS3 CS3 :P */
#define CONFIG_SYS_FMCR_VAL 0x00000001

/* Bit[0:3] contain PERCLK1DIV for UART 1
   0x000b00b ->b<- -> 192MHz/12=16MHz
   0x000b00b ->8<- -> 144MHz/09=16MHz
   0x000b00b ->3<- -> 64MHz/4=16MHz */

#ifdef BUS96
#define CONFIG_SYS_PCDR_VAL 0x000b00b5
#endif

#ifdef BUS64
#define CONFIG_SYS_PCDR_VAL 0x000b00b3
#endif

#ifdef BUS72
#define CONFIG_SYS_PCDR_VAL 0x000b00b8
#endif

#endif	/* __CONFIG_H */

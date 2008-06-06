/*
 * Copyright (C) 2003 ETC s.r.o.
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
 *
 * Written by Peter Figuli <peposh@etc.sk>, 2003.
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_PXA250          1        /* this is an PXA250 CPU     */
#define CONFIG_WEPEP250        1        /* config for wepep250 board */
#undef  CONFIG_USE_IRQ                  /* don't need use IRQ/FIQ    */


/*
 * Select serial console configuration
 */
#define CONFIG_BTUART          1       /* BTUART is default on WEP dev board */
#define CONFIG_BAUDRATE   115200


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

#undef CONFIG_CMD_NET
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_CONSOLE
#undef CONFIG_CMD_AUTOSCRIPT


/*
 * Boot options. Setting delay to -1 stops autostart count down.
 * NOTE: Sending parameters to kernel depends on kernel version and
 * 2.4.19-rmk6-pxa1 patch used while my u-boot coding didn't accept
 * parameters at all! Do not get confused by them so.
 */
#define CONFIG_BOOTDELAY   -1
#define CONFIG_BOOTARGS    "root=/dev/mtdblock2 mem=32m console=ttyS01,115200n8"
#define CONFIG_BOOTCOMMAND "bootm 40000"


/*
 * General options for u-boot. Modify to save memory foot print
 */
#define CFG_LONGHELP                                  /* undef saves memory  */
#define CFG_PROMPT              "WEP> "               /* prompt string       */
#define CFG_CBSIZE              256                   /* console I/O buffer  */
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* print buffer size   */
#define CFG_MAXARGS             16                    /* max command args    */
#define CFG_BARGSIZE            CFG_CBSIZE            /* boot args buf size  */

#define CFG_MEMTEST_START       0xa0400000            /* memtest test area   */
#define CFG_MEMTEST_END         0xa0800000

#undef  CFG_CLKS_IN_HZ                       /* use HZ for freq. display     */

#define CFG_HZ                  3686400      /* incrementer freq: 3.6864 MHz */
#define CFG_CPUSPEED            0x141        /* core clock - register value  */

#define CFG_BAUDRATE_TABLE      { 9600, 19200, 38400, 57600, 115200 }

/*
 * Definitions related to passing arguments to kernel.
 */
#define CONFIG_CMDLINE_TAG           1   /* send commandline to Kernel       */
#define CONFIG_SETUP_MEMORY_TAGS     1   /* send memory definition to kernel */
#undef  CONFIG_INITRD_TAG                /* do not send initrd params        */
#undef  CONFIG_VFD                       /* do not send framebuffer setup    */


/*
 * Malloc pool need to host env + 128 Kb reserve for other allocations.
 */
#define CFG_MALLOC_LEN	  (CFG_ENV_SIZE + (128<<10) )
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

#define CONFIG_STACKSIZE        (120<<10)      /* stack size */

#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ    (4<<10)        /* IRQ stack  */
#define CONFIG_STACKSIZE_FIQ    (4<<10)        /* FIQ stack  */
#endif

/*
 * SDRAM Memory Map
 */
#define CONFIG_NR_DRAM_BANKS    1                /* we have 1 bank of SDRAM */
#define WEP_SDRAM_1            0xa0000000        /* SDRAM bank #1           */
#define WEP_SDRAM_1_SIZE       0x02000000        /* 32 MB ( 2 chip )        */
#define WEP_SDRAM_2            0xa2000000        /* SDRAM bank #2           */
#define WEP_SDRAM_2_SIZE       0x00000000        /* 0 MB                    */
#define WEP_SDRAM_3            0xa8000000        /* SDRAM bank #3           */
#define WEP_SDRAM_3_SIZE       0x00000000        /* 0 MB                    */
#define WEP_SDRAM_4            0xac000000        /* SDRAM bank #4           */
#define WEP_SDRAM_4_SIZE       0x00000000        /* 0 MB                    */

#define CFG_DRAM_BASE           0xa0000000
#define CFG_DRAM_SIZE           0x02000000

/* Uncomment used SDRAM chip */
#define WEP_SDRAM_K4S281633
/*#define WEP_SDRAM_K4S561633*/


/*
 * Configuration for FLASH memory
 */
#define CFG_MAX_FLASH_BANKS	1	/* FLASH banks count (not chip count)*/
#define CFG_MAX_FLASH_SECT	128	/* number of sector in FLASH bank    */
#define WEP_FLASH_BUS_WIDTH	4	/* we use 32 bit FLASH memory...     */
#define WEP_FLASH_INTERLEAVE	2	/* ... made of 2 chips */
#define WEP_FLASH_BANK_SIZE  0x2000000  /* size of one flash bank*/
#define WEP_FLASH_SECT_SIZE  0x0040000  /* size of erase sector */
#define WEP_FLASH_BASE       0x0000000  /* location of flash memory */
#define WEP_FLASH_UNLOCK        1       /* perform hw unlock first */


/* This should be defined if CFI FLASH device is present. Actually benefit
   is not so clear to me. In other words we can provide more informations
   to user, but this expects more complex flash handling we do not provide
   now.*/
#undef  CFG_FLASH_CFI

#define CFG_FLASH_ERASE_TOUT    (2*CFG_HZ)    /* timeout for Erase operation */
#define CFG_FLASH_WRITE_TOUT    (2*CFG_HZ)    /* timeout for Write operation */

#define CFG_FLASH_BASE          WEP_FLASH_BASE

/*
 * This is setting for JFFS2 support in u-boot.
 * Right now there is no gain for user, but later on booting kernel might be
 * possible. Consider using XIP kernel running from flash to save RAM
 * footprint.
 * NOTE: Enable CONFIG_CMD_JFFS2 for JFFS2 support.
 */
#define CFG_JFFS2_FIRST_BANK		0
#define CFG_JFFS2_FIRST_SECTOR		5
#define CFG_JFFS2_NUM_BANKS		1

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

#define CFG_MONITOR_BASE	PHYS_FLASH_1
#define CFG_MONITOR_LEN		0x20000		/* 128kb ( 1 flash sector )  */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		0x20000	        /* absolute address for now  */
#define CFG_ENV_SIZE		0x2000

#undef  CONFIG_ENV_OVERWRITE                    /* env is not writable now   */

/*
 * Well this has to be defined, but on the other hand it is used differently
 * one may expect. For instance loadb command do not cares :-)
 * So advice is - do not relay on this...
 */
#define CFG_LOAD_ADDR        0x40000

#endif  /* __CONFIG_H */

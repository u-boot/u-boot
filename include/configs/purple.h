/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/*
 * This file contains the configuration parameters for the PURPLE board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MIPS32		1	/* MIPS 5Kc CPU core	*/
#define CONFIG_PURPLE		1	/* on a PURPLE Board	*/

#define CPU_CLOCK_RATE	125000000   /* 125 MHz clock for the MIPS core */
#define ASC_CLOCK_RATE	 62500000   /* 62.5 MHz ASC clock              */

#define INFINEON_EBU_BOOTCFG	0xE0CC

#define CONFIG_STACKSIZE	(128 * 1024)

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define CONFIG_BAUDRATE		19200

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define	CONFIG_TIMESTAMP		/* Print image info with timestamp */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off\0"				\
	"addmisc=setenv bootargs ${bootargs} "				\
		"console=ttyS0,${baudrate} "				\
		"ethaddr=${ethaddr} "					\
		"panic=1\0"						\
	"flash_nfs=run nfsargs addip addmisc;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addmisc;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 80500000 ${bootfile};"				\
		"run nfsargs addip addmisc;bootm\0"			\
	"rootpath=/opt/eldk/mips_5KC\0"					\
	"bootfile=/tftpboot/purple/uImage\0"				\
	"kernel_addr=B0040000\0"					\
	"ramdisk_addr=B0100000\0"					\
	"u-boot=/tftpboot/purple/u-boot.bin\0"				\
	"load=tftp 80500000 ${u-boot}\0"				\
	"update=protect off 1:0-4;era 1:0-4;"				\
		"cp.b 80500000 B0000000 ${filesize}\0"			\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"


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

#define CONFIG_CMD_ELF


#define CFG_SDRAM_BASE		0x80000000

#define CFG_INIT_SP_OFFSET      0x400000

#define CFG_MALLOC_LEN		128*1024

#define CFG_BOOTPARAMS_LEN	128*1024

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory      */
#define	CFG_PROMPT		"PURPLE # "	/* Monitor Command Prompt    */
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size   */
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)  /* Print Buffer Size */
#define CFG_HZ			(CPU_CLOCK_RATE/2)
#define	CFG_MAXARGS		16		/* max number of command args*/

#define	CFG_LOAD_ADDR		0x80500000	/* default load address	*/

#define CFG_MEMTEST_START	0x80200000
#define CFG_MEMTEST_END		0x80800000

#define	CONFIG_MISC_INIT_R

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	(35)	/* max number of sectors on one chip */

#define PHYS_FLASH_1		0xb0000000 /* Flash Bank #1 */

/* The following #defines are needed to get flash environment right */
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(192 << 10)

#define CFG_FLASH_BASE		PHYS_FLASH_1

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(6 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(6 * CFG_HZ) /* Timeout for Flash Write */

#define	CFG_ENV_IS_IN_FLASH	1

/* Address and size of Primary Environment Sector	*/
#define CFG_ENV_ADDR		0xB0008000
#define CFG_ENV_SIZE		0x4000

#define CONFIG_FLASH_32BIT
#define CONFIG_NR_DRAM_BANKS	1

#define CONFIG_PLB2800_ETHER
#define CONFIG_NET_MULTI

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		16384
#define CFG_ICACHE_SIZE		16384
#define CFG_CACHELINE_SIZE	32

/*
 * Temporary buffer for serial data until the real serial driver
 * is initialised (memtest will destroy this buffer)
 */
#define CFG_SCONSOLE_ADDR     (CFG_SDRAM_BASE + CFG_INIT_SP_OFFSET - \
			       CFG_DCACHE_SIZE / 2)
#define CFG_SCONSOLE_SIZE     (CFG_DCACHE_SIZE / 4)

#endif	/* __CONFIG_H */

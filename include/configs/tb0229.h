/*
 * (C) Copyright 2003
 * Masami Komiya <mkomiya@sonare.it>
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

/*
 * Config header file for TANBAC TB0229 board using an VR4131 CPU module
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MIPS32		1	/* MIPS 4Kc CPU core	*/
#define CONFIG_TB0229		1	/* on a TB0229 Board	*/

#ifndef CPU_CLOCK_RATE
#define CPU_CLOCK_RATE	200000000	/* 200 MHz clock for the MIPS core */
#endif
#define CPU_TCLOCK_RATE 16588800	/* 16.5888 MHz for TClock */

#define CONFIG_CONS_INDEX	1
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define CONFIG_BAUDRATE		115200

#define CONFIG_TIMESTAMP		/* Print image info with timestamp */

#define CONFIG_PREBOOT	"echo;" \
	"echo Type \\\"boot\\\" for the network boot using DHCP, TFTP and NFS;" \
	"echo Type \\\"run netboot_initrd\\\" for the network boot with initrd;" \
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo Type \\\"run flash_local\\\" to mount local root filesystem;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netboot=dhcp;tftp;run netargs; bootm\0"			\
	"nfsargs=setenv bootargs root=/dev/nfs ip=dhcp\0"		\
	"localargs=setenv bootargs root=1F02 ip=dhcp\0"			\
	"addmisc=setenv bootargs ${bootargs} "				\
		"console=ttyS0,${baudrate} "				\
		"read-only=readonly\0"					\
	"netargs=run nfsargs addmisc\0"					\
	"flash_nfs=run nfsargs addmisc;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_local=run localargs addmisc;"				\
		"bootm ${kernel_addr}\0"				\
	"netboot_initrd=dhcp;tftp;tftp 80600000 initrd;"		\
		"setenv bootargs root=/dev/ram ramdisk_size=8192 ip=dhcp;"\
		"run addmisc;"						\
		"bootm 80400000 80600000\0"				\
	"rootpath=/export/miniroot-mipsel\0"				\
	"autoload=no\0"							\
	"kernel_addr=BFC60000\0"					\
	"ramdisk_addr=B0100000\0"					\
	"u-boot=u-boot.bin\0"						\
	"bootfile=uImage\0"						\
	"load=dhcp;tftp 80400000 ${u-boot}\0"				\
	"load_kernel=dhcp;tftp 80400000 ${bootfile}\0"			\
	"update_uboot=run load;"					\
		"protect off BFC00000 BFC3FFFF;"			\
		"erase BFC00000 BFC3FFFF;"				\
		"cp.b 80400000 BFC00000 ${filesize}\0"			\
	"update_kernel=run load_kernel;"				\
		"erase BFC60000 BFD5FFFF;"				\
		"cp.b 80400000 BFC60000 ${filesize}\0"			\
	"initenv=erase bfc40000 bfc5ffff\0"				\
	""
/*#define CONFIG_BOOTCOMMAND	"run flash_local" */
#define CONFIG_BOOTCOMMAND	"run netboot"


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

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_PCI
#define CONFIG_CMD_ELF


/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP				/* undef to save memory	     */
#define CONFIG_SYS_PROMPT		"# "		/* Monitor Command Prompt    */
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size   */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)  /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args*/

#define CONFIG_SYS_MALLOC_LEN		128*1024

#define CONFIG_SYS_BOOTPARAMS_LEN	128*1024

#define CONFIG_SYS_MIPS_TIMER_FREQ	(CPU_TCLOCK_RATE/4)

#define CONFIG_SYS_HZ			1000

#define CONFIG_SYS_SDRAM_BASE		0x80000000

#define CONFIG_SYS_LOAD_ADDR		0x80400000	/* default load address */

#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x80800000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	(128)	/* max number of sectors on one chip */

#define PHYS_FLASH_1		0xbfc00000 /* Flash Bank #1 */

/* The following #defines are needed to get flash environment right */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(192 << 10)

#define CONFIG_SYS_INIT_SP_OFFSET	0x400000

#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(20 * CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2 * CONFIG_SYS_HZ) /* Timeout for Flash Write */

#define CONFIG_ENV_IS_IN_FLASH	1

/* Address and size of Primary Environment Sector	*/
#define CONFIG_ENV_ADDR		0xBFC40000
#define CONFIG_ENV_SIZE		0x20000

#define CONFIG_SYS_DIRECT_FLASH_TFTP

#define CONFIG_NR_DRAM_BANKS	1

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_DCACHE_SIZE		16384
#define CONFIG_SYS_ICACHE_SIZE		16384
#define CONFIG_SYS_CACHELINE_SIZE	16

/*-----------------------------------------------------------------------
 * Serial Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	 1
#define CONFIG_SYS_NS16550_CLK		 18432000
#define CONFIG_SYS_NS16550_COM1	 0xaf000800

/*-----------------------------------------------------------------------
 * PCI stuff
 */
#define CONFIG_PCI
#define CONFIG_PCI_PNP
#define CONFIG_EEPRO100
#define CONFIG_SYS_RX_ETH_BUFFER	8		/* use 8 rx buffer on eepro100	*/

#define CONFIG_RTL8139

#endif	/* __CONFIG_H */

/*
 * (C) Copyright 2009
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __MANROLAND_COMMON_H
#define __MANROLAND_COMMON_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_BOARD_EARLY_INIT_R

/* Partitions */
#define CONFIG_DOS_PARTITION

/*
 * Command line configuration.
 */
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DISPLAY
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_DTT
#define CONFIG_CMD_IDE

/*
 * 8-symbol LED display (can be accessed with 'display' command)
 */
#define CONFIG_PDSP188x

#define	CONFIG_TIMESTAMP	1	/* Print image info with timestamp */

/*
 * Autobooting
 */

#define CONFIG_PREBOOT	"echo;" \
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addwdt=setenv bootargs ${bootargs} wdt=off\0"			\
	"logval=4\0"							\
	"addlog=setenv bootargs ${bootargs} loglevel=${logval}\0"	\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"kernel_addr=ff810000\0"					\
	"fdt_addr="__stringify(CONFIG_SYS_FLASH_BASE)"\0"		\
	"flash_nfs=run nfsargs addip addcon addwdt addlog;"		\
		"bootm ${kernel_addr} - ${fdt_addr}\0"			\
	"rootpath=/opt/eldk/ppc_82xx\0"					\
	"kernel_addr_r=300000\0"					\
	"fdt_addr_r=200000\0"						\
	"fdt_file=" __stringify(CONFIG_HOSTNAME) "/" 			\
		__stringify(CONFIG_HOSTNAME) ".dtb\0"			\
	"kernel_file=" __stringify(CONFIG_HOSTNAME) "/uImage \0" 	\
	"load_fdt=tftp ${fdt_addr_r} ${fdt_file};\0"			\
	"load_kernel=tftp ${kernel_addr_r} ${kernel_file};\0" 		\
	"addcon=setenv bootargs ${bootargs} console=ttyPSC0,${baudrate}\0"\
	"net_nfs=run load_fdt load_kernel; "				\
		"run nfsargs addip addcon addwdt addlog;"		\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"u-boot=" __stringify(CONFIG_HOSTNAME) "/u-boot.bin \0" 	\
	"u-boot_addr_r=200000\0"					\
	"load=tftp ${u-boot_addr_r} ${u-boot}\0"			\
	"update=protect off " __stringify(CONFIG_SYS_TEXT_BASE) " +${filesize};"\
		"erase " __stringify(CONFIG_SYS_TEXT_BASE) " +${filesize};"\
		"cp.b ${u-boot_addr_r} " __stringify(CONFIG_SYS_TEXT_BASE) \
		" ${filesize};"						\
		"protect on " __stringify(CONFIG_SYS_TEXT_BASE) " +${filesize}\0"\
	""

#define CONFIG_BOOTCOMMAND	"run net_nfs"

#define CONFIG_MISC_INIT_R	1

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	    */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_CMDLINE_EDITING		1	/* add command line history */
#define CONFIG_AUTO_COMPLETE		/* add autocompletion support	*/

/* Enable an alternate, more extensive memory test */
#define CONFIG_SYS_ALT_MEMTEST

/*
 * Enable loopw command.
 */

#endif /* __MANROLAND_COMMON_H */

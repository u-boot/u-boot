/*
 * (C) Copyright 2009
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __MANROLAND_COMMON_H
#define __MANROLAND_COMMON_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM		0x02	/* Software reboot			*/

#define CONFIG_BOARD_EARLY_INIT_R

/* Partitions */
#define CONFIG_DOS_PARTITION

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DATE
#define CONFIG_CMD_DISPLAY
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_DTT
#define CONFIG_CMD_IDE
#define CONFIG_CMD_FAT
#define CONFIG_CMD_NFS
#define CONFIG_CMD_MII
#define CONFIG_CMD_SNTP

#define	CONFIG_TIMESTAMP	1	/* Print image info with timestamp */

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#define CONFIG_PREBOOT	"echo;" \
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define xstr(s)	str(s)
#define str(s)	#s

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
	"fdt_addr="xstr(CONFIG_SYS_FLASH_BASE)"\0"			\
	"flash_nfs=run nfsargs addip addcon addwdt addlog;"		\
		"bootm ${kernel_addr} - ${fdt_addr}\0"			\
	"rootpath=/opt/eldk/ppc_82xx\0"					\
	"kernel_addr_r=300000\0"					\
	"fdt_addr_r=200000\0"						\
	"fdt_file=" xstr(CONFIG_HOSTNAME) "/" 				\
		xstr(CONFIG_HOSTNAME) ".dtb\0"				\
	"kernel_file=" xstr(CONFIG_HOSTNAME) "/uImage \0" 		\
	"load_fdt=tftp ${fdt_addr_r} ${fdt_file};\0"			\
	"load_kernel=tftp ${kernel_addr_r} ${kernel_file};\0" 		\
	"addcon=setenv bootargs ${bootargs} console=ttyPSC0,${baudrate}\0"\
	"net_nfs=run load_fdt load_kernel; "				\
		"run nfsargs addip addcon addwdt addlog;"		\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"u-boot=" xstr(CONFIG_HOSTNAME) "/u-boot.bin \0" 		\
	"u-boot_addr_r=200000\0"					\
	"load=tftp ${u-boot_addr_r} ${u-boot}\0"			\
	"update=protect off " xstr(TEXT_BASE) " +${filesize};"		\
		"erase " xstr(TEXT_BASE) " +${filesize};"		\
		"cp.b ${u-boot_addr_r} " xstr(TEXT_BASE) 		\
		" ${filesize};"						\
		"protect on " xstr(TEXT_BASE) " +${filesize}\0"		\
	""

#define CONFIG_BOOTCOMMAND	"run net_nfs"

#define CONFIG_MISC_INIT_R	1

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	    */
#define CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/* Enable an alternate, more extensive memory test */
#define CONFIG_SYS_ALT_MEMTEST

/*
 * Enable loopw command.
 */
#define CONFIG_LOOPW

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#endif /* __MANROLAND_COMMON_H */

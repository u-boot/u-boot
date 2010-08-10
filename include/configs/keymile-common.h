/*
 * (C) Copyright 2008
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

#ifndef __CONFIG_KEYMILE_H
#define __CONFIG_KEYMILE_H

/* Do boardspecific init for all boards */
#define CONFIG_BOARD_EARLY_INIT_R       1

#define CONFIG_BOOTCOUNT_LIMIT

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_DTT
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_JFFS2
#define CONFIG_JFFS2_CMDLINE
#define CONFIG_CMD_MTDPARTS

#undef	CONFIG_WATCHDOG			/* disable platform specific watchdog */

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */
#undef	CONFIG_BOOTARGS			/* the boot command will set bootargs */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_LONGHELP			/* undef to save memory	    */
#define CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size  */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size  */
#define CONFIG_CMDLINE_EDITING		1	/* add command line history     */
#define CONFIG_AUTO_COMPLETE		/* add autocompletion support	*/

#define CONFIG_HUSH_INIT_VAR	1

#define CONFIG_SYS_ALT_MEMTEST		/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */

#define CONFIG_SYS_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * How to get access to the slot ID.  Put this here to make it easy
 * to modify in a centralized location.  This is used in the HDLC
 * driver to set the MAC.
*/
#define CONFIG_CHECK_ETHERNET_PRESENT	1
#define CONFIG_SYS_SLOT_ID_BASE		CONFIG_SYS_PIGGY_BASE
#define CONFIG_SYS_SLOT_ID_OFF		(0x07)	/* register offset */
#define CONFIG_SYS_SLOT_ID_MASK		(0x3f)	/* mask for slot ID bits */

#define CONFIG_I2C_MULTI_BUS	1
#define CONFIG_SYS_MAX_I2C_BUS		1
#define CONFIG_SYS_I2C_INIT_BOARD	1
#define CONFIG_I2C_MUX		1

/* EEprom support */
#define CONFIG_SYS_I2C_MULTI_EEPROMS	1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_ENABLE
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10

/* Support the IVM EEprom */
#define	CONFIG_SYS_IVM_EEPROM_ADR	0x50
#define CONFIG_SYS_IVM_EEPROM_MAX_LEN	0x400
#define CONFIG_SYS_IVM_EEPROM_PAGE_LEN	0x100

#define	CONFIG_SYS_FLASH_PROTECTION 1

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

#define CONFIG_ENV_SIZE		0x04000 /* Size of Environment */

#define CONFIG_SYS_MALLOC_LEN	(4 * 1024 * 1024)

/* UBI Support for all Keymile boards */
#define CONFIG_CMD_UBI
#define CONFIG_RBTREE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_FLASH_CFI_MTD
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_CONCAT

/* define this to use the keymile's io muxing feature */
/*#define CONFIG_IO_MUXING */

#ifdef CONFIG_IO_MUXING
#define	CONFIG_KM_DEF_ENV_IOMUX \
	"nc=setenv ethact HDLC \0" \
	"nce=setenv ethact SCC \0"	\
	"stderr=serial,nc \0"	\
	"stdin=serial,nc \0" \
	"stdout=serial,nc \0" \
	"tftpsrcp=69 \0" \
	"tftpdstp=69 \0"
#else
#define	CONFIG_KM_DEF_ENV_IOMUX \
	"stderr=serial \0" \
	"stdin=serial \0"	 \
	"stdout=serial \0"
#endif

#ifndef CONFIG_KM_DEF_ENV_PRIVATE
#define	CONFIG_KM_DEF_ENV_PRIVATE \
	"kmprivate=empty\0"
#endif

#define xstr(s)	str(s)
#define str(s)	#s

#ifndef CONFIG_KM_DEF_ENV
#define CONFIG_KM_DEF_ENV	\
	"netdev=eth0\0"							\
	"u-boot_addr_r=100000\0"					\
	"kernel_addr_r=200000\0"					\
	"fdt_addr_r=600000\0"						\
	"ram_ws=800000 \0"						\
	"script_ws=780000 \0"						\
	"fdt_file=" xstr(CONFIG_HOSTNAME) "/" 				\
		xstr(CONFIG_HOSTNAME) ".dtb\0"				\
	"u-boot=" xstr(CONFIG_HOSTNAME) "/u-boot.bin \0" 		\
	"kernel_file=" xstr(CONFIG_HOSTNAME) "/uImage \0" 		\
	"load=tftp ${u-boot_addr_r} ${u-boot}\0"			\
	"update=protect off " xstr(BOOTFLASH_START) " +${filesize};"	\
		"erase " xstr(BOOTFLASH_START) "  +${filesize};"	\
		"cp.b ${u-boot_addr_r} " xstr(BOOTFLASH_START) 		\
		"  ${filesize};"					\
		"protect on " xstr(BOOTFLASH_START) "  +${filesize}\0"	\
	"load_fdt=tftp ${fdt_addr_r} ${fdt_file}; "			\
		"setenv actual_fdt_addr ${fdt_addr_r} \0" 		\
	"load_kernel=tftp ${kernel_addr_r} ${kernel_file}; " 		\
		"setenv actual_kernel_addr ${kernel_addr_r} \0" 	\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"mtdargs=setenv bootargs root=${actual_rootfs} rw "		\
		"rootfstype=jffs2 \0" 					\
	"altmtdargs=setenv bootargs root=${backup_rootfs} rw "		\
		"rootfstype=jffs2 \0" 					\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addboardid=setenv bootargs ${bootargs} " 			\
		"hwKey=${IVM_HWKey} boardId=0x${IVM_BoardId} \0" 	\
	"addpram=setenv bootargs ${bootargs} "				\
		"mem=${mem} pram=${pram}\0"				\
	"pram=" xstr(CONFIG_PRAM) "k\0"					\
	"net_nfs=tftp ${kernel_addr_r} ${kernel_file}; "		\
		"tftp ${fdt_addr_r} ${fdt_file}; "			\
		"run nfsargs addip addcon addboardid addpram;"		\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"net_self=tftp ${kernel_addr_r} ${kernel_file}; "		\
		"tftp ${fdt_addr_r} ${fdt_file}; "			\
		"tftp ${ramdisk_addr} ${ramdisk_file}; "		\
		"run ramargs addip addboardid addpram; "		\
		"bootm ${kernel_addr_r} ${ramdisk_addr} ${fdt_addr_r}\0"\
	"flash_nfs=run nfsargs addip addcon;"				\
		"bootm ${kernel_addr} - ${fdt_addr}\0"			\
	"flash_self=run ramargs addip addcon addboardid addpram;"	\
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdt_addr}\0"	\
	"bootcmd=run mtdargs addip addcon addboardid addpram; "		\
		"bootm ${actual_kernel_addr} - ${actual_fdt_addr} \0"	\
	"altbootcmd=run altmtdargs addip addcon addboardid addpram; "	\
		"bootm ${backup_kernel_addr} - ${backup_fdt_addr} \0"	\
	"actual0=setenv actual_bank 0; setenv actual_kernel_addr "	\
		"${bank0_kernel_addr}; "				\
		"setenv actual_fdt_addr ${bank0_fdt_addr}; "		\
		"setenv actual_rootfs ${bank0_rootfs} \0" 		\
	"actual1=setenv actual_bank 1; setenv actual_kernel_addr "	\
		"${bank1_kernel_addr}; "				\
		"setenv actual_fdt_addr ${bank1_fdt_addr}; "		\
		"setenv actual_rootfs ${bank1_rootfs} \0" 		\
	"backup0=setenv backup_bank 0; setenv backup_kernel_addr " 	\
		"${bank0_kernel_addr}; "				\
		"setenv backup_fdt_addr ${bank0_fdt_addr}; "		\
		"setenv backup_rootfs ${bank0_rootfs} \0"		\
	"backup1=setenv backup_bank 1; setenv backup_kernel_addr "	\
		"${bank1_kernel_addr}; "				\
		"setenv backup_fdt_addr ${bank1_fdt_addr}; " 		\
		"setenv backup_rootfs ${bank1_rootfs} \0" 		\
	"setbank0=run actual0 backup1 \0" 				\
	"setbank1=run actual1 backup0 \0" 				\
	"release=setenv bootcmd "					\
		"\'run mtdargs addip addcon addboardid addpram;" 	\
		"bootm ${actual_kernel_addr} - ${actual_fdt_addr} \'; "	\
		"saveenv \0"						\
	"develop=setenv bootcmd "					\
		"\'run nfsargs addip addcon addboardid addpram;" 	\
		"bootm ${actual_kernel_addr} - ${actual_fdt_addr} \'; "	\
		"saveenv \0"						\
	"developall=setenv bootcmd "					\
		"\'run load_fdt load_kernel nfsargs "			\
		"addip addcon addboardid addpram; "			\
		"bootm ${actual_kernel_addr} - ${actual_fdt_addr} \'; "	\
		"saveenv \0"						\
	"set_new_esw_script=setenv new_esw_script "			\
		"new_esw_0x${IVM_BoardId}_0x${IVM_HWKey}.scr \0"	\
	"new_esw=run set_new_esw_script; "				\
		"tftp ${script_ws} ${new_esw_script}; "			\
		"iminfo ${script_ws}; source ${script_ws} \0"		\
	"bootlimit=0 \0" 						\
	CONFIG_KM_DEF_ENV_IOMUX						\
	CONFIG_KM_DEF_ENV_PRIVATE					\
	""
#endif /* CONFIG_KM_DEF_ENV */

#define CONFIG_VERSION_VARIABLE 	/* include version env variable */

#endif /* __CONFIG_KEYMILE_H */

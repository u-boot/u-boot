/*
 * (C) Copyright 2008-2011
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
#define CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_LAST_STAGE_INIT

#define CONFIG_BOOTCOUNT_LIMIT

/*
 * By default kwbimage.cfg from board specific folder is used
 * If for some board, different configuration file need to be used,
 * CONFIG_SYS_KWD_CONFIG should be defined in board specific header file
 */
#ifndef CONFIG_SYS_KWD_CONFIG
#define	CONFIG_SYS_KWD_CONFIG	$(SRCTREE)/$(CONFIG_BOARDDIR)/kwbimage.cfg
#endif /* CONFIG_SYS_KWD_CONFIG */

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
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_SETEXPR

#undef	CONFIG_WATCHDOG		/* disable platform specific watchdog */

#define CONFIG_BOOTDELAY	2 /* autoboot after 2 seconds */
#undef	CONFIG_BOOTARGS		/* the boot command will set bootargs */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_LONGHELP			/* undef to save memory	  */
#define CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS		32 /* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE

#define CONFIG_HUSH_INIT_VAR

#define CONFIG_SYS_ALT_MEMTEST		/* memory test, takes time */

#define CONFIG_SYS_HZ			1000	/* decr. freq: 1 ms ticks */

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CONFIG_LOADS_ECHO
#define CONFIG_SYS_LOADS_BAUD_CHANGE
#define CONFIG_SYS_BOARD_DRAM_INIT	/* Used board specific dram_init */

/*
 * How to get access to the slot ID.  Put this here to make it easy
 * to modify in a centralized location.  This is used in the HDLC
 * driver to set the MAC.
*/
#define CONFIG_CHECK_ETHERNET_PRESENT
#define CONFIG_SYS_SLOT_ID_BASE		CONFIG_SYS_KMBEC_FPGA_BASE
#define CONFIG_SYS_SLOT_ID_OFF		(0x07)	/* register offset */
#define CONFIG_SYS_SLOT_ID_MASK		(0x3f)	/* mask for slot ID bits */

#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_MAX_I2C_BUS		1
#define CONFIG_SYS_I2C_INIT_BOARD
#define CONFIG_I2C_MUX

/* EEprom support */
#define CONFIG_SYS_I2C_MULTI_EEPROMS
#define CONFIG_SYS_EEPROM_PAGE_WRITE_ENABLE
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10

/* Support the IVM EEprom */
#define	CONFIG_SYS_IVM_EEPROM_ADR	0x50
#define CONFIG_SYS_IVM_EEPROM_MAX_LEN	0x400
#define CONFIG_SYS_IVM_EEPROM_PAGE_LEN	0x100

#define	CONFIG_SYS_FLASH_PROTECTION

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)

/* UBI Support for all Keymile boards */
#define CONFIG_CMD_UBI
#define CONFIG_RBTREE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_CONCAT

/* common powerpc specific env settings */
#ifndef CONFIG_KM_DEF_ENV_BOOTPARAMS
#define CONFIG_KM_DEF_ENV_BOOTPARAMS \
	"bootparams=empty\0"	\
	"initial_boot_bank=0\0"
#endif

#ifndef CONFIG_KM_DEF_NETDEV
#define CONFIG_KM_DEF_NETDEV	\
	"netdev=eth0\0"
#endif

#ifndef CONFIG_KM_UBI_PARTITION_NAME
#define CONFIG_KM_UBI_PARTITION_NAME	"ubi0"
#endif
#ifndef CONFIG_KM_UBI_LINUX_MTD_NAME
#define CONFIG_KM_UBI_LINUX_MTD_NAME	"ubi0"
#endif

#define xstr(s)	str(s)
#define str(s)	#s

/*
 * bootrunner
 * - run all commands in 'subbootcmds'
 * - on error, stop running the remaing commands
 */
#define CONFIG_KM_DEF_ENV_BOOTRUNNER					\
	"bootrunner="							\
		"break=0; "						\
		"for subbootcmd in ${subbootcmds}; do "			\
		"if test ${break} -eq 0; then; "			\
		"echo \"[INFO] running \\c\"; "				\
		"print ${subbootcmd}; "					\
		"run ${subbootcmd} || break=1; "			\
		"if test ${break} -eq 1; then; "			\
		"echo \"[ERR] failed \\c\"; "				\
		"print ${subbootcmd}; "					\
		"fi; "							\
		"fi; "							\
		"done\0"						\
	""

/*
 * boottargets
 * - set 'subbootcmds' for the bootrunner
 * - set 'bootcmd' and 'altbootcmd'
 * available targets:
 * - 'release': for a standalone system		kernel/rootfs from flash
 * - 'develop': for development			kernel(tftp)/rootfs(NFS)
 * - 'ramfs': rootfilesystem in RAM		kernel(tftp)/rootfs(RAM)
 *
 * - 'commonargs': bootargs common to all targets
 */
#define CONFIG_KM_DEF_ENV_BOOTTARGETS					\
	"commonargs="							\
		"addip "						\
		"addtty "						\
		"addmem "						\
		"addinit "						\
		"addvar "						\
		"addmtdparts "						\
		"addbootcount "						\
		"\0"							\
	"develop="							\
		"setenv subbootcmds \""					\
		"tftpfdt tftpkernel "					\
		"nfsargs ${commonargs} "				\
		"printbootargs boot "					\
		"\" && "						\
		"setenv bootcmd \'"					\
		"run bootrunner"					\
		"\' && "						\
		"setenv altbootcmd \'"					\
		"run bootcmd"						\
		"\' && "						\
		"run setboardid && "					\
		"saveenv && "						\
		"reset\0"						\
	"ramfs="							\
		"setenv actual_bank -1 && "				\
		"setenv subbootcmds \""					\
		"tftpfdt tftpkernel "					\
		"setrootfsaddr tftpramfs "				\
		"flashargs ${commonargs} "				\
		"addpanic addramfs "					\
		"printbootargs boot "					\
		"\" && "						\
		"setenv bootcmd \'"					\
		"run bootrunner"					\
		"\' && "						\
		"setenv altbootcmd \'"					\
		"run bootcmd"						\
		"\' && "						\
		"run setboardid && "					\
		"run setramfspram && "					\
		"saveenv && "						\
		"reset\0"						\
	"release="							\
		"setenv actual_bank ${initial_boot_bank} && "		\
		"setenv subbootcmds \""					\
		"checkboardidlist "					\
		"checkboardid "						\
		"ubiattach ubicopy "					\
		"cramfsloadfdt cramfsloadkernel "			\
		"flashargs ${commonargs} "				\
		"addpanic "						\
		"printbootargs boot "					\
		"\" && "						\
		"setenv bootcmd \'"					\
		"run actual bootrunner; reset"				\
		"\' && "						\
		"setenv altbootcmd \'"					\
		"run backup bootrunner; reset"				\
		"\' && "						\
		"saveenv && "						\
		"reset\0"						\
	""

/*
 * bootargs
 * - modify 'bootargs'
 *
 * - 'addip': add ip configuration
 * - 'addmem': limit kernel memory mem=
 * - 'addpanic': add kernel panic options
 * - 'addramfs': add phram device for the rootfilesysten in ram
 * - 'addtty': add console=...
 * - 'addvar': add phram device for /var
 * - 'nfsargs': default arguments for nfs boot
 * - 'flashargs': defaults arguments for flash base boot
 *
 * processor specific settings
 * - 'addbootcount': add boot counter
 * - 'addmtdparts': add mtd partition information
 */
#define CONFIG_KM_DEF_ENV_BOOTARGS					\
	"addinit="							\
		"setenv bootargs ${bootargs} init=${init}\0"		\
	"addip="							\
		"setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off\0"				\
	"addmem="							\
		"setenv bootargs ${bootargs} mem=0x${pnvramaddr}\0"	\
	"addpanic="							\
		"setenv bootargs ${bootargs} "				\
		"panic=1 panic_on_oops=1\0"				\
	"addramfs="							\
		"setenv bootargs \""					\
		"${bootargs} phram.phram="				\
		"rootfs${boot_bank},${rootfsaddr},${rootfssize}\"\0"	\
	"addtty="							\
		"setenv bootargs ${bootargs}"				\
		" console=" CONFIG_KM_CONSOLE_TTY ",${baudrate}\0"	\
	"addvar="							\
		"setenv bootargs ${bootargs} phram.phram=phvar,"	\
		"${varaddr},0x" xstr(CONFIG_KM_PHRAM) "\0"		\
	"nfsargs="							\
		"setenv bootargs "					\
		"ubi.mtd=" CONFIG_KM_UBI_LINUX_MTD_NAME " "		\
		"root=/dev/nfs rw "					\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"flashargs="							\
		"setenv bootargs "					\
		"ubi.mtd=" CONFIG_KM_UBI_LINUX_MTD_NAME " "		\
		"root=mtdblock:rootfs${boot_bank} "			\
		"rootfstype=squashfs ro\0"				\
	""

/*
 * compute_addr
 * - compute addresses and sizes
 * - addresses are calculated form the end of memory 'memsize'
 *
 * - 'setramfspram': compute PRAM size for ramfs target
 * - 'setrootfsaddr': compute rootfilesystem address for phram
 */
#define CONFIG_KM_DEF_ENV_COMPUTE_ADDR					\
	"setboardid="							\
		"if test \"x${boardId}\" = \"x\"; then; "		\
		"setenv boardId ${IVM_BoardId} && "			\
		"setenv hwKey ${IVM_HWKey}; "				\
		"else; "						\
		"echo \\\\c; "						\
		"fi\0"							\
	"setramfspram="							\
		"setexpr value ${rootfssize} / 0x400 && "		\
		"setexpr value 0x${value} + ${pram} && "		\
		"setenv pram 0x${value}\0"				\
	"setrootfsaddr="						\
		"setexpr value ${pnvramaddr} - ${rootfssize} && "	\
		"setenv rootfsaddr 0x${value}\0"			\
	""

/*
 * flash_boot
 * - commands for booting from flash
 *
 * - 'cramfsaddr': address to the cramfs (in ram)
 * - 'cramfsloadkernel': copy kernel from a cramfs to ram
 * - 'ubiattach': attach ubi partition
 * - 'ubicopy': copy ubi volume to ram
 *              - volume names: bootfs0, bootfs1, bootfs2, ...
 * - 'ubiparition': mtd parition name for ubi
 *
 * processor specific settings
 * - 'cramfsloadfdt': copy fdt from a cramfs to ram
 */
#define CONFIG_KM_DEF_ENV_FLASH_BOOT					\
	"cramfsaddr="xstr(CONFIG_KM_CRAMFS_ADDR) "\0"			\
	"cramfsloadkernel="						\
		"cramfsload ${kernel_addr_r} uImage && "		\
		"setenv actual_kernel_addr ${kernel_addr_r}\0"		\
	"ubiattach=ubi part ${ubipartition}\0"				\
	"ubicopy=ubi read ${cramfsaddr} bootfs${boot_bank}\0"		\
	"ubipartition=" CONFIG_KM_UBI_PARTITION_NAME "\0"		\
	""

/*
 * net_boot
 * - commands for booting over the network
 *
 * - 'tftpkernel': load a kernel with tftp into ram
 * - 'tftpramfs': load rootfs with tftp into ram
 *
 * processor specific settings
 * - 'tftpfdt': load fdt with tftp into ram
 */
#define CONFIG_KM_DEF_ENV_NET_BOOT					\
	"tftpkernel="							\
		"tftpboot ${kernel_addr_r} ${kernel_file} && "		\
		"setenv actual_kernel_addr ${kernel_addr_r}\0"		\
	"tftpramfs="							\
		"tftpboot ${rootfsaddr} \"\\\"${rootfsfile}\\\"\" && "	\
		"setenv loadaddr\0"					\
	""

/*
 * constants
 * - KM specific constants and commands
 *
 * - 'default': setup default environment
 */
#define CONFIG_KM_DEF_ENV_CONSTANTS					\
	"actual=setenv boot_bank ${actual_bank}\0"			\
	"backup=setenv boot_bank ${backup_bank}\0"			\
	"actual_bank=${initial_boot_bank}\0"				\
	"backup_bank=0\0"						\
	"default="							\
		"setenv default 'run newenv; reset' &&  "		\
		"run release && saveenv; reset\0"			\
	"checkboardidlist="						\
		"if test \"x${boardIdListHex}\" != \"x\"; then "	\
		"IVMbidhwk=${IVM_BoardId}_${IVM_HWKey}; "		\
		"found=0; "						\
		"for bidhwk in \"${boardIdListHex}\"; do "		\
		"echo trying $bidhwk ...; "				\
		"if test \"x$bidhwk\" = \"x$IVMbidhwk\"; then "		\
		"found=1; "						\
		"echo match found for $bidhwk; "			\
		"if test \"x$bidhwk\" != \"x${boardId}_${hwKey}\";then "\
			"setenv boardid ${IVM_BoardId}; "		\
			"setenv boardId ${IVM_BoardId}; "		\
			"setenv hwkey ${IVM_HWKey}; "			\
			"setenv hwKey ${IVM_HWKey}; "			\
			"echo \"boardId set to ${boardId}\"; "		\
			"echo \"hwKey   set to ${hwKey}\"; "		\
			"saveenv; "					\
		"fi; "							\
		"fi; "							\
		"done; "						\
		"else "							\
			"echo \"boardIdListHex not set, not checked\"; "\
			"found=1; "					\
		"fi; "							\
		"test \"$found\" = 1 \0"				\
	"checkboardid="							\
		"test \"x${boardId}\" = \"x${IVM_BoardId}\" && "	\
		"test \"x${hwKey}\" = \"x${IVM_HWKey}\"\0"		\
	"printbootargs=print bootargs\0"				\
	"rootfsfile="xstr(CONFIG_HOSTNAME) "/rootfsImage\0"		\
	""

#ifndef CONFIG_KM_DEF_ENV
#define CONFIG_KM_DEF_ENV	\
	CONFIG_KM_DEF_ENV_BOOTPARAMS					\
	CONFIG_KM_DEF_NETDEV						\
	CONFIG_KM_DEF_ENV_CPU						\
	CONFIG_KM_DEF_ENV_BOOTRUNNER					\
	CONFIG_KM_DEF_ENV_BOOTTARGETS					\
	CONFIG_KM_DEF_ENV_BOOTARGS					\
	CONFIG_KM_DEF_ENV_COMPUTE_ADDR					\
	CONFIG_KM_DEF_ENV_FLASH_BOOT					\
	CONFIG_KM_DEF_ENV_NET_BOOT					\
	CONFIG_KM_DEF_ENV_CONSTANTS					\
	"altbootcmd=run bootcmd\0"					\
	"bootcmd=run default\0"						\
	"bootlimit=2\0"							\
	"init=/sbin/init-overlay.sh\0"					\
	"kernel_addr_r="xstr(CONFIG_KM_KERNEL_ADDR) "\0"		\
	"kernel_file="xstr(CONFIG_HOSTNAME) "/uImage\0"			\
	"kernel_name=uImage\0"						\
	"load=tftpboot ${u-boot_addr_r} ${u-boot}\0"			\
	"mtdids=" MTDIDS_DEFAULT "\0"					\
	"mtdparts=" MTDPARTS_DEFAULT "\0"				\
	"stderr=serial\0"						\
	"stdin=serial\0"						\
	"stdout=serial\0"						\
	"u-boot="xstr(CONFIG_HOSTNAME) "/u-boot.bin\0"			\
	"u-boot_addr_r="xstr(CONFIG_KM_KERNEL_ADDR) "\0"		\
	""
#endif /* CONFIG_KM_DEF_ENV */

#define CONFIG_VERSION_VARIABLE 	/* include version env variable */

#endif /* __CONFIG_KEYMILE_H */

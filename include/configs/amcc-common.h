/*
 * (C) Copyright 2008, 2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * Common configuration options for all AMCC boards
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

#ifndef __AMCC_COMMON_H
#define __AMCC_COMMON_H

#define CONFIG_SYS_SDRAM_BASE		0x00000000	/* _must_ be 0		*/
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* Start of U-Boot	*/
#define CONFIG_SYS_MONITOR_LEN		(0xFFFFFFFF - CONFIG_SYS_MONITOR_BASE + 1)
#define CONFIG_SYS_MALLOC_LEN		(1 << 20)	/* Reserved for malloc	*/

/*
 * UART
 */
#define CONFIG_SERIAL_MULTI
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

/*
 * I2C
 */
#define CONFIG_HARD_I2C			/* I2C with hardware support	*/
#define CONFIG_PPC4XX_I2C		/* use PPC4xx driver		*/
#define CONFIG_SYS_I2C_SLAVE		0x7F

/*
 * Ethernet/EMAC/PHY
 */
#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII			/* MII PHY management		*/
#define CONFIG_NETCONSOLE		/* include NetConsole support	*/
#if defined(CONFIG_440)
#define CONFIG_SYS_RX_ETH_BUFFER	32	/* number of eth rx buffers	*/
#else
#define CONFIG_SYS_RX_ETH_BUFFER	16	/* number of eth rx buffers	*/
#endif

/*
 * Commands
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#if defined(CONFIG_440)
#define CONFIG_CMD_CACHE
#endif
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO

/*
 * Miscellaneous configurable options
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0400000 /* memtest works on		*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000  /* default load address	*/
#define CONFIG_SYS_EXTBDINFO			/* To use extended board_into (bd_t) */

#define CONFIG_SYS_HZ			1000	/* decrementer freq: 1 ms ticks	*/

#define CONFIG_CMDLINE_EDITING		/* add command line history	*/
#define CONFIG_AUTO_COMPLETE		/* add autocompletion support	*/
#define CONFIG_LOOPW			/* enable loopw command         */
#define CONFIG_MX_CYCLIC		/* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 	/* include version env variable */
#define CONFIG_SYS_CONSOLE_INFO_QUIET		/* don't print console @ startup*/

#define CONFIG_SYS_HUSH_PARSER			/* Use the HUSH parser		*/
#ifdef	CONFIG_SYS_HUSH_PARSER
#define	CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#endif

#define CONFIG_LOADS_ECHO		/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE		/* allow baudrate change	*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_SUBNETMASK

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 16 MB of memory, since this is
 * the maximum mapped by the 40x Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(16 << 20) /* Initial Memory map for Linux */
#define CONFIG_SYS_BOOTM_LEN		(16 << 20) /* Increase max gunzip size */

/*
 * Internal Definitions
 */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port*/
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use	*/
#endif

/*
 * Pass open firmware flat tree
 */
#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP
/* Update size in "reg" property of NOR FLASH device tree nodes */
#define CONFIG_FDT_FIXUP_NOR_FLASH_SIZE

/*
 * Booting and default environment
 */
#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"
#define CONFIG_BOOTCOMMAND	"run flash_self"

/*
 * Only very few boards have default console not on ttyS0 (like Taishan)
 */
#if !defined(CONFIG_USE_TTY)
#define CONFIG_USE_TTY	ttyS0
#endif

/*
 * Only very few boards have default netdev not set to eth0 (like Arches)
 */
#if !defined(CONFIG_USE_NETDEV)
#define CONFIG_USE_NETDEV	eth0
#endif

/*
 * Only some 4xx PPC's are equipped with an FPU
 */
#if defined(CONFIG_440EP) || defined(CONFIG_440EPX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define CONFIG_AMCC_DEF_ENV_ROOTPATH	"rootpath=/opt/eldk/ppc_4xxFP\0"
#else
#define CONFIG_AMCC_DEF_ENV_ROOTPATH	"rootpath=/opt/eldk/ppc_4xx\0"
#endif

/*
 * Only some boards need to extend the bootargs by some additional
 * parameters (like Makalu)
 */
#if !defined(CONFIG_ADDMISC)
#define CONFIG_ADDMISC	"addmisc=setenv bootargs ${bootargs}\0"
#endif

#define xstr(s)	str(s)
#define str(s)	#s

/*
 * General common environment variables shared on all AMCC eval boards
 */
#define CONFIG_AMCC_DEF_ENV						\
	"netdev=" xstr(CONFIG_USE_NETDEV) "\0"				\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs}"				\
		" console=" xstr(CONFIG_USE_TTY) ",${baudrate}\0"	\
	CONFIG_ADDMISC							\
	"initrd_high=30000000\0"					\
	"kernel_addr_r=1000000\0"					\
	"fdt_addr_r=1800000\0"						\
	"ramdisk_addr_r=1900000\0"					\
	"hostname=" xstr(CONFIG_HOSTNAME) "\0"				\
	"bootfile=" xstr(CONFIG_HOSTNAME) "/uImage\0"			\
	"ramdisk_file=" xstr(CONFIG_HOSTNAME) "/uRamdisk\0"		\
	CONFIG_AMCC_DEF_ENV_ROOTPATH

/*
 * Default environment for arch/powerpc booting
 * for boards that are ported to arch/powerpc
 */
#define CONFIG_AMCC_DEF_ENV_POWERPC					\
	"flash_self=run ramargs addip addtty addmisc;"			\
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdt_addr}\0"	\
	"flash_nfs=run nfsargs addip addtty addmisc;"			\
		"bootm ${kernel_addr} - ${fdt_addr}\0"			\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile}; "			\
		"tftp ${fdt_addr_r} ${fdt_file}; "			\
		"run nfsargs addip addtty addmisc;"			\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"net_self_load=tftp ${kernel_addr_r} ${bootfile};"		\
		"tftp ${fdt_addr_r} ${fdt_file};"			\
		"tftp ${ramdisk_addr_r} ${ramdisk_file};\0"		\
	"net_self=run net_self_load;"					\
		"run ramargs addip addtty addmisc;"			\
		"bootm ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr_r}\0" \
	"fdt_file=" xstr(CONFIG_HOSTNAME) "/" xstr(CONFIG_HOSTNAME) ".dtb\0"

/*
 * Default environment for arch/ppc booting,
 * for boards that are not ported to arch/powerpc yet
 */
#define CONFIG_AMCC_DEF_ENV_PPC						\
	"flash_self=run ramargs addip addtty addmisc;"			\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"flash_nfs=run nfsargs addip addtty addmisc;"			\
		"bootm ${kernel_addr}\0"				\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile};"			\
		"run nfsargs addip addtty addmisc;"			\
		"bootm ${kernel_addr_r}\0"

/*
 * Default environment for arch/ppc booting (old version),
 * for boards that are ported to arch/ppc and arch/powerpc
 */
#define CONFIG_AMCC_DEF_ENV_PPC_OLD					\
	"flash_self_old=run ramargs addip addtty addmisc;"		\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"flash_nfs_old=run nfsargs addip addtty addmisc;"		\
		"bootm ${kernel_addr}\0"				\
	"net_nfs_old=tftp ${kernel_addr_r} ${bootfile};"		\
		"run nfsargs addip addtty addmisc;"			\
		"bootm ${kernel_addr_r}\0"

#define CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"u-boot=" xstr(CONFIG_HOSTNAME) "/u-boot.bin\0"			\
	"load=tftp 200000 ${u-boot}\0"					\
	"update=protect off " xstr(CONFIG_SYS_MONITOR_BASE) " FFFFFFFF;"	\
		"era " xstr(CONFIG_SYS_MONITOR_BASE) " FFFFFFFF;"		\
		"cp.b ${fileaddr} " xstr(CONFIG_SYS_MONITOR_BASE) " ${filesize}\0" \
	"upd=run load update\0"						\

#define CONFIG_AMCC_DEF_ENV_NAND_UPD					\
	"u-boot-nand=" xstr(CONFIG_HOSTNAME) "/u-boot-nand.bin\0"	\
	"nload=tftp 200000 ${u-boot-nand}\0"				\
	"nupdate=nand erase 0 100000;nand write 200000 0 100000\0"	\
	"nupd=run nload nupdate\0"

#endif /* __AMCC_COMMON_H */

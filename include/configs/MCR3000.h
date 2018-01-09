/*
 * Copyright (C) 2010-2017 CS Systemes d'Information
 * Christophe Leroy <christophe.leroy@c-s.fr>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_early_init_f */
#define CONFIG_MISC_INIT_R		1	/* Call misc_init_r	*/

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"sdram_type=SDRAM\0"						\
	"flash_type=AM29LV160DB\0"					\
	"loadaddr=0x400000\0"						\
	"filename=uImage.lzma\0"					\
	"nfsroot=/opt/ofs\0"						\
	"dhcp_ip=ip=:::::eth0:dhcp\0"					\
	"console_args=console=ttyCPM0,115200N8\0"			\
	"flashboot=setenv bootargs "					\
		"${console_args} "					\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:"	\
		"mcr3k:eth0:off;"					\
		"${ofl_args}; "						\
		"bootm 0x04060000 - 0x04050000\0"			\
	"tftpboot=setenv bootargs "					\
		"${console_args} "					\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:"	\
		"mcr3k:eth0:off "					\
		"${ofl_args}; "						\
		"tftp ${loadaddr} ${filename};"				\
		"tftp 0xf00000 mcr3000.dtb;"				\
		"bootm ${loadaddr} - 0xf00000\0"			\
	"netboot=dhcp ${loadaddr} ${filename};"				\
		"tftp 0xf00000 mcr3000.dtb;"				\
		"setenv bootargs "					\
		"root=/dev/nfs rw "					\
		"${console_args} "					\
		"${dhcp_ip};"						\
		"bootm ${loadaddr} - 0xf00000\0"			\
	"nfsboot=setenv bootargs "					\
		"root=/dev/nfs rw nfsroot=${serverip}:${nfsroot} "	\
		"${console_args} "					\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:"	\
		"mcr3k:eth0:off;"					\
		"bootm 0x04060000 - 0x04050000\0"			\
	"dhcpboot=dhcp ${loadaddr} ${filename};"			\
		"tftp 0xf00000 mcr3000.dtb;"				\
		"setenv bootargs "					\
		"${console_args} "					\
		"${dhcp_ip} "						\
		"${ofl_args}; "						\
		"bootm ${loadaddr} - 0xf00000\0"

#define CONFIG_BOOTDELAY		5

#define CONFIG_IPADDR			192.168.0.3
#define CONFIG_SERVERIP			192.168.0.1
#define CONFIG_NETMASK			255.0.0.0

#define CONFIG_BOOTCOMMAND		"run flashboot"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CONFIG_LOADS_BAUD_CHANGE	/* don't allow baudrate change	*/

#define CONFIG_WATCHDOG		1	/* watchdog enabled */

/* Miscellaneous configurable options */
#define	CONFIG_SYS_LONGHELP

#define CONFIG_CMDLINE_EDITING		1
#ifdef	CONFIG_HUSH_PARSER
#define	CONFIG_SYS_PROMPT_HUSH_PS2	"S3K> "
#endif

#define CONFIG_SYS_MEMTEST_START	0x00002000
#define CONFIG_SYS_MEMTEST_END		0x00800000

#define	CONFIG_SYS_LOAD_ADDR		0x200000

#define	CONFIG_SYS_HZ			1000

/* Definitions for initial stack pointer and data area (in DPRAM) */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define	CONFIG_SYS_INIT_RAM_SIZE	0x2f00
#define	CONFIG_SYS_GBL_DATA_SIZE	64
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					 CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/* RAM configuration (note that CONFIG_SYS_SDRAM_BASE must be zero) */
#define	CONFIG_SYS_SDRAM_BASE		0x00000000
#define SDRAM_MAX_SIZE			(32 * 1024 * 1024)

/* FLASH organization */
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_FLASH_CFI		1
#define CONFIG_FLASH_CFI_DRIVER		1
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	35
#define CONFIG_SYS_FLASH_ERASE_TOUT	120000
#define CONFIG_SYS_FLASH_WRITE_TOUT	500

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CONFIG_SYS_BOOTMAPSZ		(8 << 20)
#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MALLOC_LEN		(4096 << 10)

/* Environment Configuration */

/* environment is in FLASH */
#define CONFIG_ENV_SECT_SIZE	(64 * 1024)
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_OFFSET	(CONFIG_ENV_ADDR - CONFIG_SYS_FLASH_BASE)
#define CONFIG_ENV_OVERWRITE	1

/* Cache Configuration */
#define CONFIG_SYS_CACHELINE_SIZE	16

/* Ethernet configuration part */
#define CONFIG_SYS_DISCOVER_PHY		1
#ifdef CONFIG_MPC8XX_FEC
#define CONFIG_MII_INIT			1
#endif

/* NAND configuration part */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_MAX_CHIPS	1
#define CONFIG_SYS_NAND_BASE		0x0C000000

/* Internal Definitions */

/* Boot Flags*/
#define	BOOTFLAG_COLD			0x01
#define BOOTFLAG_WARM			0x02

#endif /* __CONFIG_H */

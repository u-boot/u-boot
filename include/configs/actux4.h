/*
 * (C) Copyright 2007
 * Michael Schwingen, michael@schwingen.org
 *
 * Configuration settings for the AcTux-4 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_IXP425			1
#define CONFIG_ACTUX4			1

#define	CONFIG_MACH_TYPE		1532

#define CONFIG_DISPLAY_CPUINFO		1
#define CONFIG_DISPLAY_BOARDINFO	1

#define CONFIG_IXP_SERIAL
#define CONFIG_SYS_IXP425_CONSOLE		IXP425_UART1
#define CONFIG_BAUDRATE			115200
#define CONFIG_BOOTDELAY		3
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_BOARD_EARLY_INIT_F	1

/***************************************************************
 * U-boot generic defines start here.
 ***************************************************************/
/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN			(CONFIG_ENV_SIZE + 128*1024)

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Command line configuration */
#include <config_cmd_default.h>

#define CONFIG_CMD_ELF

#define CONFIG_PCI
#ifdef CONFIG_PCI
#define CONFIG_CMD_PCI
#define CONFIG_PCI_PNP
#define CONFIG_IXP_PCI
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_CMD_PCI_ENUM
#endif

#define CONFIG_BOOTCOMMAND		"run boot_flash"
/* enable passing of ATAGs */
#define CONFIG_CMDLINE_TAG		1
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

#if defined(CONFIG_CMD_KGDB)
# define CONFIG_KGDB_BAUDRATE		230400
#endif

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP
/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE			256
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE			(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
/* max number of command args */
#define CONFIG_SYS_MAXARGS			16
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE			CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START		0x00400000
#define CONFIG_SYS_MEMTEST_END			0x00800000

/* timer clock - 2* OSC_IN system clock */
#define CONFIG_IXP425_TIMER_CLK                 66000000

/* default load address */
#define CONFIG_SYS_LOAD_ADDR			0x00010000

/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE		{ 9600, 19200, 38400, 57600, \
					  115200, 230400 }
#define CONFIG_SERIAL_RTS_ACTIVE	1

/* Expansion bus settings */
#define CONFIG_SYS_EXP_CS0			0xbd113003

/* SDRAM settings */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			0x00000000
#define CONFIG_SYS_SDRAM_BASE			0x00000000

/* 32MB SDRAM */
#define CONFIG_SYS_SDR_CONFIG			0x18
#define PHYS_SDRAM_1_SIZE		0x02000000
#define CONFIG_SYS_SDRAM_REFRESH_CNT		0x81a
#define CONFIG_SYS_SDR_MODE_CONFIG		0x1
#define CONFIG_SYS_DRAM_SIZE			0x02000000

/* FLASH organization */
#define CONFIG_SYS_TEXT_BASE		0x50000000
#define CONFIG_SYS_MAX_FLASH_BANKS		2
/* max # of sectors per chip */
#define CONFIG_SYS_MAX_FLASH_SECT		70
#define PHYS_FLASH_1			0x50000000
#define PHYS_FLASH_2			0x51000000
#define CONFIG_SYS_FLASH_BANKS_LIST		{ PHYS_FLASH_1, PHYS_FLASH_2 }

#define CONFIG_SYS_FLASH_BASE			PHYS_FLASH_1
#define CONFIG_SYS_MONITOR_BASE		PHYS_FLASH_1
#define CONFIG_SYS_MONITOR_LEN			(252 << 10)
#define CONFIG_BOARD_SIZE_LIMIT			258048

/* Use common CFI driver */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
/* board provides its own flash_init code */
#define CONFIG_FLASH_CFI_LEGACY		1
/* no byte writes on IXP4xx */
#define CONFIG_SYS_FLASH_CFI_WIDTH		FLASH_CFI_16BIT
/* SST 39VF020 etc. support */
#define CONFIG_SYS_FLASH_LEGACY_256Kx8	1

/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_EMPTY_INFO

/* Ethernet */

/* include IXP4xx NPE support */
#define CONFIG_IXP4XX_NPE		1

/* NPE0 PHY address */
#define	CONFIG_PHY_ADDR			0x1C
/* MII PHY management */
#define CONFIG_MII			1

/* Number of ethernet rx buffers & descriptors */
#define CONFIG_SYS_RX_ETH_BUFFER		16

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#undef  CONFIG_CMD_NFS

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/* Cache Configuration */
#define CONFIG_SYS_CACHELINE_SIZE		32

/* environment organization: one complete 4k flash sector */
#define	CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_SIZE			0x1000
#define CONFIG_ENV_ADDR			(PHYS_FLASH_1 + 0x3f000)

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"npe_ucode=51000000\0"						\
	"mtd=IXP4XX-Flash.0:252k(uboot),4k(uboot_env);"			\
	"IXP4XX-Flash.1:128k(ucode),1280k(linux),-(root)\0"		\
	"kerneladdr=51020000\0"						\
	"kernelfile=actux4/uImage\0"					\
	"rootfile=actux4/rootfs\0"					\
	"rootaddr=51160000\0"						\
	"loadaddr=10000\0"						\
	"updateboot_ser=mw.b 10000 ff 40000;"				\
	" loady ${loadaddr};"						\
	" run eraseboot writeboot\0"					\
	"updateboot_net=mw.b 10000 ff 40000;"				\
	" tftp ${loadaddr} actux4/u-boot.bin;"					\
	" run eraseboot writeboot\0"					\
	"eraseboot=protect off 50000000 5003efff;"			\
	" erase 50000000 +${filesize}\0"				\
	"writeboot=cp.b 10000 50000000 ${filesize}\0"			\
	"updateucode=loady;"						\
	" era ${npe_ucode} +${filesize};"				\
	" cp.b ${loadaddr} ${npe_ucode} ${filesize}\0"			\
	"updateroot=tftp ${loadaddr} ${rootfile};"			\
	" era ${rootaddr} +${filesize};"				\
	" cp.b ${loadaddr} ${rootaddr} ${filesize}\0"			\
	"updatekern=tftp ${loadaddr} ${kernelfile};"			\
	" era ${kerneladdr} +${filesize};"				\
	" cp.b ${loadaddr} ${kerneladdr} ${filesize}\0"			\
	"flashargs=setenv bootargs mtdparts=${mtd} root=/dev/mtdblock4"	\
	" rootfstype=squashfs,jffs2 init=/etc/preinit\0"		\
	"netargs=setenv bootargs mtdparts=${mtd} root=/dev/mtdblock4"	\
	" rootfstype=squashfs,jffs2 init=/etc/preinit\0"		\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0" \
	"addeth=setenv bootargs ${bootargs} ethaddr=${ethaddr}\0"	\
	"boot_flash=run flashargs addtty addeth;"			\
	" bootm ${kerneladdr}\0"					\
	"boot_net=run netargs addtty addeth;"				\
	" tftpboot ${loadaddr} ${kernelfile};"				\
	" bootm\0"

/* additions for new relocation code, must be added to all boards */
#define CONFIG_SYS_INIT_SP_ADDR						\
	(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)

#endif /* __CONFIG_H */

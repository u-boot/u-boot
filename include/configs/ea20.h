/*
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Based on davinci_dvevm.h. Original Copyrights follow:
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * Board
 */
#define CONFIG_DRIVER_TI_EMAC
#define CONFIG_USE_SPIFLASH
#define	CONFIG_SYS_USE_NAND
#define CONFIG_DRIVER_TI_EMAC_USE_RMII
#define CONFIG_DRIVER_TI_EMAC_RMII_NO_NEGOTIATE
#define CONFIG_PREBOOT

/*
 * SoC Configuration
 */
#define CONFIG_MACH_DAVINCI_DA850_EVM
#define CONFIG_SOC_DA8XX		/* TI DA8xx SoC */
#define CONFIG_SOC_DA850		/* TI DA850 SoC */
#define CONFIG_SYS_CLK_FREQ		clk_get(DAVINCI_ARM_CLKID)
#define CONFIG_SYS_OSCIN_FREQ		24000000
#define CONFIG_SYS_TIMERBASE		DAVINCI_TIMER0_BASE
#define CONFIG_SYS_HZ_CLOCK		clk_get(DAVINCI_AUXCLK_CLKID)
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_TEXT_BASE		0xc1080000
#define CONFIG_DA8XX_GPIO

/*
 * Memory Info
 */
#define CONFIG_SYS_MALLOC_LEN	(0x10000 + 4*1024*1024) /* malloc() len */
#define PHYS_SDRAM_1		DAVINCI_DDR_EMIF_DATA_BASE /* DDR Start */
#define PHYS_SDRAM_1_SIZE	(64 << 20) /* SDRAM size 64MB */
#define CONFIG_MAX_RAM_BANK_SIZE (512 << 20) /* max size from SPRS586*/

/* memtest start addr */
#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM_1 + 0x2000000)

/* memtest will be run on 16MB */
#define CONFIG_SYS_MEMTEST_END	(PHYS_SDRAM_1 + 0x2000000 + 16*1024*1024)

#define CONFIG_NR_DRAM_BANKS	1 /* we have 1 bank of DRAM */

/*
 * Serial Driver info
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4	/* NS16550 register size */
#define CONFIG_SYS_NS16550_COM1	DAVINCI_UART0_BASE /* Base address of UART0 */
#define CONFIG_SYS_NS16550_CLK	clk_get(DAVINCI_UART2_CLKID)
#define CONFIG_CONS_INDEX	1		/* use UART0 for console */

#define CONFIG_SPI
#define CONFIG_DAVINCI_SPI
#define CONFIG_SYS_SPI_BASE		DAVINCI_SPI1_BASE
#define CONFIG_SYS_SPI_CLK		clk_get(DAVINCI_SPI1_CLKID)
#define CONFIG_SF_DEFAULT_SPEED		30000000
#define CONFIG_ENV_SPI_MAX_HZ	CONFIG_SF_DEFAULT_SPEED

/*
 * I2C Configuration
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_DAVINCI
#define CONFIG_SYS_DAVINCI_I2C_SPEED		100000
#define CONFIG_SYS_DAVINCI_I2C_SLAVE   10 /* Bogus, master-only in U-Boot */

/*
 * Network & Ethernet Configuration
 */
#ifdef CONFIG_DRIVER_TI_EMAC
#define CONFIG_MII
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT	10
#endif

#ifdef CONFIG_USE_SPIFLASH
#undef CONFIG_ENV_IS_IN_FLASH
#undef CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SIZE			(8 << 10)
#define CONFIG_ENV_OFFSET		0x80000
#define CONFIG_ENV_SECT_SIZE		(64 << 10)
#endif

#if defined(CONFIG_VIDEO)
#define CONFIG_VIDEO_DA8XX
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_VIDEO_BMP_LOGO
#endif

/*
 * U-Boot general configuration
 */
#define CONFIG_BOOTFILE		"uImage" /* Boot file name */
#define CONFIG_SYS_CBSIZE	1024 /* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16 /* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Args Buffer Size */
#define CONFIG_SYS_LOAD_ADDR	(PHYS_SDRAM_1 + 0x700000)
#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP
#define CONFIG_MX_CYCLIC

/*
 * Linux Information
 */
#define LINUX_BOOT_PARAM_ADDR	(PHYS_SDRAM_1 + 0x100)
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS

/*
 * U-Boot commands
 */
#define CONFIG_CMD_SAVES

#ifdef CONFIG_CMD_BDI
#define CONFIG_CLOCKS
#endif

#ifndef CONFIG_DRIVER_TI_EMAC
#endif

/* NAND Setup */
#ifdef CONFIG_SYS_USE_NAND
#define CONFIG_CMD_NAND

#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_LZO
#define CONFIG_RBTREE
#define CONFIG_CMD_UBIFS

#define CONFIG_NAND_DAVINCI
#define	CONFIG_SYS_NAND_PAGE_2K
#define CONFIG_SYS_NAND_NO_SUBPAGE
#define CONFIG_SYS_NAND_CS		2
#define CONFIG_SYS_NAND_BASE		DAVINCI_ASYNC_EMIF_DATA_CE2_BASE
#undef CONFIG_SYS_NAND_HW_ECC
#define CONFIG_SYS_NAND_4BIT_HW_ECC_OOBFIRST
#define	CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_MAX_NAND_DEVICE	1 /* Max number of NAND devices */
#endif

/* SPI Flash */
#ifdef CONFIG_USE_SPIFLASH
#endif

#if !defined(CONFIG_SYS_USE_NAND) && \
	!defined(CONFIG_USE_NOR) && \
	!defined(CONFIG_USE_SPIFLASH)
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE		(16 << 10)
#endif

/* additions for new relocation code, must added to all boards */
#define CONFIG_SYS_SDRAM_BASE		0xc0000000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x1000 - \
					GENERATED_GBL_DATA_SIZE)
/*
 * Default environment and default scripts
 * to update uboot and load kernel
 */

#define CONFIG_HOSTNAME ea20
#define	CONFIG_EXTRA_ENV_SETTINGS				\
	"as=3\0"							\
	"netdev=eth0\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"rfsbargs=setenv bootargs root=/dev/nfs rw "			\
	"nfsroot=${serverip}:${rfsbpath}\0"				\
	"testrfsargs=setenv bootargs root=/dev/nfs rw "		\
	"nfsroot=${serverip}:${testrfspath}\0"				\
	"ramargs=setenv bootargs root=/dev/ram rw initrd="		\
	"0x${ramdisk_addr_r},4M\0"					\
	"mtdids=nand0=davinci_nand.0\0"				\
	"serverip=192.168.5.249\0"					\
	"ipaddr=192.168.5.248\0"					\
	"rootpath=/opt/eldk/arm\0"					\
	"splashpos=230,180\0"						\
	"testrfspath=/opt/eldk/test_arm\0"				\
	"nandargs=setenv bootargs rootfstype=ubifs ro chk_data_crc "	\
	"ubi.mtd=${as} root=ubi0:rootfs\0"				\
	"nandrwargs=setenv bootargs rootfstype=ubifs rw chk_data_crc "	\
	"ubi.mtd=${as} root=ubi0:rootfs\0"				\
	"addip_sta=setenv bootargs ${bootargs} "			\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addip_dyn=setenv bootargs ${bootargs} ip=dhcp\0"		\
	"addip=if test -n ${ipdyn};then run addip_dyn;"		\
		"else run addip_sta;fi\0"				\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addtty=setenv bootargs ${bootargs}"				\
		" console=${consoledev},${baudrate}n8\0"		\
	"addmisc=setenv bootargs ${bootargs} ${misc}\0"		\
	"addmem=setenv bootargs ${bootargs} mem=${memory}\0"		\
	"consoledev=ttyS0\0"						\
	"loadaddr=c0000014\0"						\
	"memory=32M\0"							\
	"kernel_addr_r=c0700000\0"					\
	"hostname=" __stringify(CONFIG_HOSTNAME) "\0"			\
	"bootfile=" __stringify(CONFIG_HOSTNAME) "/uImage\0"		\
	"ramdisk_file=" __stringify(CONFIG_HOSTNAME) "/image.ext2\0"	\
	"flash_self=run ramargs addip addtty addmtd addmisc addmem;"	\
			"bootm ${kernel_addr_r}\0"			\
	"flash_nfs=run nfsargs addip addtty addmtd addmisc addmem;"	\
		"bootm ${kernel_addr}\0"				\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile}; "			\
		"run nfsargs addip addtty addmtd addmisc addmem;"	\
		"bootm ${kernel_addr_r}\0"				\
	"net_rfsb=tftp ${kernel_addr_r} ${bootfile}; "			\
		"run rfsbargs addip addtty addmtd addmisc addmem; "	\
		"bootm ${kernel_addr_r}\0"				\
	"net_testrfs=tftp ${kernel_addr_r} ${bootfile}; "		\
		"run testrfsargs addip addtty addmtd addmisc addmem; "	\
		"bootm ${kernel_addr_r}\0"				\
	"net_self_load=tftp ${kernel_addr_r} ${bootfile};"		\
		"tftp ${ramdisk_addr_r} ${ramdisk_file};\0"		\
	"nand_nand=ubi part nand0,${as};ubifsmount ubi:rootfs;"		\
		"ubifsload ${kernel_addr_r} /boot/uImage;"		\
		"ubifsumount; run nandargs addip addtty "		\
		"addmtd addmisc addmem;clrlogo;"			\
		"bootm ${kernel_addr_r}\0"				\
	"nand_nandrw=ubi part nand0,${as};ubifsmount ubi:rootfs;"	\
		"ubifsload ${kernel_addr_r} /boot/uImage;"		\
		"ubifsumount; run nandrwargs addip addtty "		\
		"addmtd addmisc addmem;clrlogo;"			\
		"bootm ${kernel_addr_r}\0"				\
	"net_nandrw=tftp ${kernel_addr_r} ${bootfile}; run nandrwargs"	\
		" addip addtty addmtd addmisc addmem;"			\
		"clrlogo;bootm ${kernel_addr_r}\0"			\
	"u-boot=" __stringify(CONFIG_HOSTNAME) "/u-boot.bin\0"		\
	"load_magic=if sf probe 0;then sf "				\
		"read c0000000 0x10000 0x60000;fi\0"			\
	"load_nand=ubi part nand0,${as};ubifsmount ubi:rootfs;"		\
		"if ubifsload c0000014 /boot/u-boot.bin;"		\
		"then mw c0000008 ${filesize};else echo Error reading"	\
		" u-boot from nand!;fi\0"				\
	"load_net=if sf probe 0;then sf read c0000000 0x10000 "	\
		"0x60000;tftp c0000014 ${u-boot};"			\
		"mw c0000008 ${filesize};fi\0"				\
	"upd=if sf probe 0;then sf erase 10000 60000;"			\
		"sf write c0000000 10000 60000;fi\0"			\
	"ublupdate=if tftp C0700000 ${ublname};then sf probe 0; "	\
		"sf erase 0 10000;"					\
		"sf write 0xc0700000 0 ${filesize};fi\0"		\
	"ubootupd_net=if run load_net;then echo Updating u-boot;"	\
		"if run upd; then echo U-Boot updated;"		\
			"else echo Error updating u-boot !;"		\
			"echo Board without bootloader !!;"		\
		"fi;"							\
		"else echo U-Boot not downloaded..exiting;fi\0"	\
	"ubootupd_nand=echo run load_magic,run load_nand,run upd;\0"	\
	"bootcmd=run net_testrfs\0"

#include <asm/arch/hardware.h>

#endif /* __CONFIG_H */

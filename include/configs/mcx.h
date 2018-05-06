/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Ilya Yanok, Emcraft Systems
 *
 * Based on omap3_evm_config.h
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */

#define CONFIG_MACH_TYPE	MACH_TYPE_MCX

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap.h>

/*
 * Leave it at 0x80008000 to allow booting new u-boot.bin with X-loader
 * and older u-boot.bin with the new U-Boot SPL.
 */

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/*
 * Size of malloc() pool
 */
#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB sector */
#define CONFIG_SYS_MALLOC_LEN		(1024 << 10)
/*
 * DDR related
 */
#define CONFIG_SYS_CS0_SIZE		(256 * 1024 * 1024)

/*
 * Hardware drivers
 */

/*
 * NS16550 Configuration
 */
#define V_NS16550_CLK			48000000	/* 48MHz (APLL96/2) */

#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * select serial console configuration
 */
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3
#define CONFIG_SERIAL3			3	/* UART3 */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}

/* EHCI */
#define CONFIG_OMAP_EHCI_PHY1_RESET_GPIO	57

/* commands to include */

#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE

#define CONFIG_SYS_I2C

/* RTC */
#define CONFIG_RTC_DS1337
#define CONFIG_SYS_I2C_RTC_ADDR		0x68

/*
 * Board NAND Info.
 */
#define CONFIG_SYS_NAND_ADDR		NAND_BASE	/* physical address */
							/* to access nand */
#define CONFIG_SYS_NAND_BASE		NAND_BASE	/* physical address */
							/* to access */
							/* nand at CS0 */

#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of */
							/* NAND devices */
#define CONFIG_JFFS2_NAND
/* nand device jffs2 lives on */
#define CONFIG_JFFS2_DEV		"nand0"
/* start of jffs2 partition */
#define CONFIG_JFFS2_PART_OFFSET	0x680000
#define CONFIG_JFFS2_PART_SIZE		0xf980000	/* sz of jffs2 part */

/* Environment information */

#define CONFIG_BOOTFILE		"uImage"

/* Setup MTD for NAND on the SOM */

#define CONFIG_HOSTNAME "mcx"
#define CONFIG_EXTRA_ENV_SETTINGS \
	"adddbg=setenv bootargs ${bootargs} trace_buf_size=64M\0"	\
	"adddebug=setenv bootargs ${bootargs} earlyprintk=serial\0"	\
	"addeth=setenv bootargs ${bootargs} ethaddr=${ethaddr}\0"	\
	"addfb=setenv bootargs ${bootargs} vram=6M "			\
		"omapfb.vram=1:2M,2:2M,3:2M omapdss.def_disp=lcd\0"	\
	"addip_sta=setenv bootargs ${bootargs} "			\
		"ip=${ipaddr}:${serverip}:${gatewayip}:"		\
		"${netmask}:${hostname}:eth0:off\0"			\
	"addip_dyn=setenv bootargs ${bootargs} ip=dhcp\0"		\
	"addip=if test -n ${ipdyn};then run addip_dyn;"			\
		"else run addip_sta;fi\0"				\
	"addmisc=setenv bootargs ${bootargs} ${misc}\0"			\
	"addtty=setenv bootargs ${bootargs} "				\
		"console=${consoledev},${baudrate}\0"			\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"baudrate=115200\0"						\
	"consoledev=ttyO2\0"						\
	"hostname=" CONFIG_HOSTNAME "\0"			\
	"loadaddr=0x82000000\0"						\
	"load=tftp ${loadaddr} ${u-boot}\0"				\
	"load_k=tftp ${loadaddr} ${bootfile}\0"				\
	"loaduimage=fatload mmc 0 ${loadaddr} uImage\0"			\
	"loadmlo=tftp ${loadaddr} ${mlo}\0"				\
	"mlo=" CONFIG_HOSTNAME "/MLO\0"			\
	"mmcargs=root=/dev/mmcblk0p2 rw "				\
		"rootfstype=ext3 rootwait\0"				\
	"mmcboot=echo Booting from mmc ...; "				\
		"run mmcargs; "						\
		"run addip addtty addmtd addfb addeth addmisc;"		\
		"run loaduimage; "					\
		"bootm ${loadaddr}\0"					\
	"net_nfs=run load_k; "						\
		"run nfsargs; "						\
		"run addip addtty addmtd addfb addeth addmisc;"		\
		"bootm ${loadaddr}\0"					\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"u-boot=" CONFIG_HOSTNAME "/u-boot.img\0"		\
	"uboot_addr=0x80000\0"						\
	"update=nandecc sw;nand erase ${uboot_addr} 100000;"		\
		"nand write ${loadaddr} ${uboot_addr} 80000\0"		\
	"updatemlo=nandecc hw;nand erase 0 20000;"			\
		"nand write ${loadaddr} 0 20000\0"			\
	"upd=if run load;then echo Updating u-boot;if run update;"	\
		"then echo U-Boot updated;"				\
			"else echo Error updating u-boot !;"		\
			"echo Board without bootloader !!;"		\
		"fi;"							\
		"else echo U-Boot not downloaded..exiting;fi\0"		\
	"loadbootscript=fatload mmc 0 ${loadaddr} boot.scr\0"		\
	"bootscript=echo Running bootscript from mmc ...; "		\
		"source ${loadaddr}\0"					\
	"nandargs=setenv bootargs ubi.mtd=7 "				\
		"root=ubi0:rootfs rootfstype=ubifs\0"			\
	"nandboot=echo Booting from nand ...; "				\
		"run nandargs; "					\
		"ubi part nand0,4;"					\
		"ubi readvol ${loadaddr} kernel;"			\
		"run addtty addmtd addfb addeth addmisc;"		\
		"bootm ${loadaddr}\0"					\
	"preboot=ubi part nand0,7;"					\
		"ubi readvol ${loadaddr} splash;"			\
		"bmp display ${loadaddr};"				\
		"gpio set 55\0"						\
	"swupdate_args=setenv bootargs root=/dev/ram "			\
		"quiet loglevel=1 "					\
		"consoleblank=0 ${swupdate_misc}\0"			\
	"swupdate=echo Running Sw-Update...;"				\
		"if printenv mtdparts;then echo Starting SwUpdate...; "	\
		"else mtdparts default;fi; "				\
		"ubi part nand0,5;"					\
		"ubi readvol 0x82000000 kernel_recovery;"		\
		"ubi part nand0,6;"					\
		"ubi readvol 0x84000000 fs_recovery;"			\
		"run swupdate_args; "					\
		"setenv bootargs ${bootargs} "				\
			"${mtdparts} "					\
			"vram=6M omapfb.vram=1:2M,2:2M,3:2M "		\
			"omapdss.def_disp=lcd;"				\
		"bootm 0x82000000 0x84000000\0"				\
	"bootcmd=mmc rescan;if fatload mmc 0 82000000 loadbootscr.scr;"	\
		"then source 82000000;else run nandboot;fi\0"

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE		1024/* Console I/O Buffer Size */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0)
#define CONFIG_SYS_MEMTEST_END		(OMAP34XX_SDRC_CS0 + \
					0x01F00000) /* 31MB */

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0) /* default load */
								/* address */
#define CONFIG_PREBOOT

/*
 * AM3517 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		OMAP34XX_GPT2
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	2	/* CS1 may or may not be populated */
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_2		OMAP34XX_SDRC_CS1

/*
 * FLASH and environment organization
 */

/* **** PISMO SUPPORT *** */

/* Redundant Environment */
#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */
#define CONFIG_ENV_OFFSET		0x180000
#define CONFIG_ENV_ADDR			0x180000
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + \
						2 * CONFIG_SYS_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE

/* Flash banks JFFS2 should use */
#define CONFIG_SYS_MAX_MTD_BANKS	(CONFIG_SYS_MAX_FLASH_BANKS + \
					CONFIG_SYS_MAX_NAND_DEVICE)
#define CONFIG_SYS_JFFS2_MEM_NAND
/* use flash_info[2] */
#define CONFIG_SYS_JFFS2_FIRST_BANK	CONFIG_SYS_MAX_FLASH_BANKS
#define CONFIG_SYS_JFFS2_NUM_BANKS	1

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

/* Defines for SPL */

#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC

#define CONFIG_SPL_TEXT_BASE		0x40200000 /*CONFIG_SYS_SRAM_START*/
#define CONFIG_SPL_MAX_SIZE		(54 * 1024)	/* 8 KB for stack */
#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK

/* move malloc and bss high to prevent clashing with the main image */
#define CONFIG_SYS_SPL_MALLOC_START	0x8f000000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000
#define CONFIG_SPL_BSS_START_ADDR	0x8f080000 /* end of RAM */
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000

#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"u-boot.img"

/* NAND boot config */
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
#define CONFIG_SYS_NAND_ECCPOS		{40, 41, 42, 43, 44, 45, 46, 47,\
					 48, 49, 50, 51, 52, 53, 54, 55,\
					 56, 57, 58, 59, 60, 61, 62, 63}
#define CONFIG_SYS_NAND_ECCSIZE		256
#define CONFIG_SYS_NAND_ECCBYTES	3
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_HAM1_CODE_SW
#define CONFIG_SPL_NAND_SOFTECC

#define CONFIG_SYS_NAND_U_BOOT_START   CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000

/*
 * ethernet support
 *
 */
#if defined(CONFIG_CMD_NET)
#define CONFIG_DRIVER_TI_EMAC
#define CONFIG_DRIVER_TI_EMAC_USE_RMII
#define CONFIG_MII
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT 10
#endif

#define CONFIG_SPLASH_SCREEN
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_VIDEO_OMAP3

#endif /* __CONFIG_H */

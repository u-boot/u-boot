/*
 * (C) Copyright 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
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
#define CONFIG_SYS_DAVINCI_EMAC_PHY_COUNT 7
#define CONFIG_USE_NAND

/*
 * SoC Configuration
 */
#define CONFIG_SOC_DA8XX		/* TI DA8xx SoC */
#define CONFIG_SOC_DA850		/* TI DA850 SoC */
#define CONFIG_SYS_EXCEPTION_VECTORS_HIGH
#define CONFIG_SYS_CLK_FREQ		clk_get(DAVINCI_ARM_CLKID)
#define CONFIG_SYS_OSCIN_FREQ		24000000
#define CONFIG_SYS_TIMERBASE		DAVINCI_TIMER0_BASE
#define CONFIG_SYS_HZ_CLOCK		clk_get(DAVINCI_AUXCLK_CLKID)
#define CONFIG_DA850_LOWLEVEL
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_SYS_DA850_PLL_INIT
#define CONFIG_SYS_DA850_DDR_INIT
#define CONFIG_DA8XX_GPIO
#define CONFIG_HOSTNAME		enbw_cmc

#define MACH_TYPE_ENBW_CMC	3585
#define CONFIG_MACH_TYPE	MACH_TYPE_ENBW_CMC

/*
 * Memory Info
 */
#define CONFIG_SYS_MALLOC_LEN	(0x10000 + 1*1024*1024) /* malloc() len */
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
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4	/* NS16550 register size */
#define CONFIG_SYS_NS16550_COM1	DAVINCI_UART2_BASE /* Base address of UART2 */
#define CONFIG_SYS_NS16550_CLK	clk_get(DAVINCI_UART2_CLKID)
#define CONFIG_CONS_INDEX	1		/* use UART0 for console */
#define CONFIG_BAUDRATE		115200		/* Default baud rate */

/*
 * I2C Configuration
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_DAVINCI
#define CONFIG_SYS_DAVINCI_I2C_SPEED		80000
#define CONFIG_SYS_DAVINCI_I2C_SLAVE   10 /* Bogus, master-only in U-Boot */
#define CONFIG_SYS_I2C_EXPANDER_ADDR   0x20
#define CONFIG_CMD_I2C

#define CONFIG_CMD_DTT
#define CONFIG_DTT_LM75
#define CONFIG_DTT_SENSORS	{0}	/* Sensor addresses		*/
#define CONFIG_SYS_DTT_MAX_TEMP	70
#define CONFIG_SYS_DTT_LOW_TEMP	-30
#define CONFIG_SYS_DTT_HYSTERESIS	3

/*
 * SPI Configuration
 */
#define CONFIG_DAVINCI_SPI
#define CONFIG_SYS_SPI_BASE		DAVINCI_SPI1_BASE
#define CONFIG_SYS_SPI_CLK		clk_get(DAVINCI_SPI1_CLKID)
#define CONFIG_CMD_SPI

/*
 * Flash & Environment
 */
#ifdef CONFIG_USE_NAND
#define CONFIG_NAND_DAVINCI
#define CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_NAND_4BIT_HW_ECC_OOBFIRST
#define CONFIG_SYS_NAND_PAGE_2K
#define CONFIG_SYS_NAND_CS		3
#define CONFIG_SYS_NAND_BASE		DAVINCI_ASYNC_EMIF_DATA_CE3_BASE
#define CONFIG_SYS_NAND_MASK_CLE		0x10
#define CONFIG_SYS_NAND_MASK_ALE		0x8
#undef CONFIG_SYS_NAND_HW_ECC
#define CONFIG_SYS_MAX_NAND_DEVICE	1 /* Max number of NAND devices */

#define MTDIDS_DEFAULT		"nor0=physmap-flash.0,nand0=davinci_nand.1"
#define MTDPARTS_DEFAULT			\
	"mtdparts="				\
		"physmap-flash.0:"		\
			"512k(U-Boot),"		\
			"64k(env1),"		\
			"64k(env2),"		\
			"-(rest);"		\
		"davinci_nand.1:"		\
			"128k(dtb),"		\
			"3m(kernel),"		\
			"4m(rootfs),"		\
			"-(userfs)"


#define CONFIG_CMD_MTDPARTS

#endif

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

/*
 * Flash configuration
 */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_FLASH_CFI_MTD
#define CONFIG_SYS_FLASH_BASE           0x60000000
#define CONFIG_SYS_FLASH_SIZE           0x01000000
#define CONFIG_SYS_MAX_FLASH_BANKS      1       /* max num of memory banks */
#define CONFIG_SYS_FLASH_BANKS_LIST     { CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_MAX_FLASH_SECT       128
#define CONFIG_FLASH_16BIT              /* Flash is 16-bit */

#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_SYS_MONITOR_LEN	0x80000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + \
					CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SECT_SIZE	(64 << 10)
#define CONFIG_ENV_SIZE		(16 << 10)	/* 16 KiB */
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + \
					CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND		(CONFIG_ENV_SIZE)
#undef CONFIG_ENV_IS_IN_NAND
#define CONFIG_DEFAULT_SETTINGS_ADDR	(CONFIG_ENV_ADDR_REDUND + \
						CONFIG_ENV_SECT_SIZE)

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"u-boot_addr_r=c0000000\0"					\
	"u-boot=" __stringify(CONFIG_HOSTNAME) "/u-boot.bin\0"		\
	"load=tftp ${u-boot_addr_r} ${u-boot}\0"			\
	"update=protect off " __stringify(CONFIG_SYS_FLASH_BASE) " +${filesize};"\
		"erase " __stringify(CONFIG_SYS_FLASH_BASE) " +${filesize};"	\
		"cp.b ${u-boot_addr_r} " __stringify(CONFIG_SYS_FLASH_BASE)	\
		" ${filesize};"						\
		"protect on " __stringify(CONFIG_SYS_FLASH_BASE) " +${filesize}\0"\
	"netdev=eth0\0"							\
	"rootpath=/opt/eldk-arm/arm\0"					\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"kernel_addr_r=c0700000\0"					\
	"fdt_addr_r=c0600000\0"						\
	"ramdisk_addr_r=c0b00000\0"					\
	"fdt_file=" __stringify(CONFIG_HOSTNAME) "/"			\
		__stringify(CONFIG_HOSTNAME) ".dtb\0"			\
	"kernel_file=" __stringify(CONFIG_HOSTNAME) "/uImage \0"	\
	"nand_ld_ramdsk=nand read ${ramdisk_addr_r} 320000 400000\0"	\
	"nand_ld_kernel=nand read ${kernel_addr_r} 20000 300000\0"	\
	"nand_ld_fdt=nand read ${fdt_addr_r} 0 2000\0"			\
	"load_kernel=tftp ${kernel_addr_r} ${kernel_file}\0"		\
	"load_fdt=tftp ${fdt_addr_r} ${fdt_file}\0"			\
	"load_nand=run nand_ld_ramdsk nand_ld_kernel nand_ld_fdt\0"	\
	"addcon=setenv bootargs ${bootargs} console=ttyS2,"		\
		"${baudrate}n8\0"					\
	"net_nfs=run load_fdt load_kernel; "				\
		"run nfsargs addip addcon addmtd addmisc;"		\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"nand_selfnand=run load_nand ramargs addip addcon addmisc;bootm "\
		"${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr_r}\0"	\
	"bootcmd=run net_nfs\0"						\
	"machid=e01\0"							\
	"key_cmd_0=echo key:   0\0"					\
	"key_cmd_1=echo key:   1\0"					\
	"key_cmd_2=echo key:   2\0"					\
	"key_cmd_3=echo key:   3\0"					\
	"key_magic_0=0\0"						\
	"key_magic_1=1\0"						\
	"key_magic_2=2\0"						\
	"key_magic_3=3\0"						\
	"magic_keys=0123\0"						\
	"hwconfig=switch:lan=on,pwl=off,config=0x60100000\0"		\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addmisc=setenv bootargs ${bootargs}\0"				\
	"mtdids=" MTDIDS_DEFAULT "\0"					\
	"mtdparts=" MTDPARTS_DEFAULT "\0"				\
	"logversion=2\0"						\
	"\0"

/*
 * U-Boot general configuration
 */
#define CONFIG_BOOTFILE		"uImage" /* Boot file name */
#define CONFIG_SYS_CBSIZE	1024 /* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16 /* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Args Buffer Size */
#define CONFIG_SYS_LOAD_ADDR	(PHYS_SDRAM_1 + 0x700000)
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP
#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC
#define CONFIG_BOOTDELAY	3
#define CONFIG_HWCONFIG
#define CONFIG_SHOW_BOOT_PROGRESS
#define CONFIG_BOARD_LATE_INIT

/*
 * U-Boot commands
 */
#define CONFIG_CMD_ENV
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_CACHE

#ifdef CONFIG_CMD_BDI
#define CONFIG_CLOCKS
#endif

#ifndef CONFIG_DRIVER_TI_EMAC
#undef CONFIG_CMD_DHCP
#undef CONFIG_CMD_MII
#undef CONFIG_CMD_PING
#endif

#ifdef CONFIG_USE_NAND
#define CONFIG_CMD_NAND

#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_LZO
#define CONFIG_RBTREE
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#endif

#if !defined(CONFIG_USE_NAND) && \
	!defined(CONFIG_USE_NOR) && \
	!defined(CONFIG_USE_SPIFLASH)
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_SYS_NO_FLASH
#define CONFIG_ENV_SIZE		(16 << 10)
#undef CONFIG_CMD_ENV
#endif

#define CONFIG_SYS_TEXT_BASE		0x60000000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_SDRAM_BASE		0xc0000000
#define CONFIG_SYS_INIT_SP_ADDR		(0x8001ff00)

#define CONFIG_VERSION_VARIABLE
#define CONFIG_ENV_OVERWRITE

#define CONFIG_PREBOOT	"echo;" \
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"
#define CONFIG_MISC_INIT_R

#define CONFIG_CMC_RESET_PIN	0x04000000
#define CONFIG_CMC_RESET_TIMEOUT	3

#define CONFIG_HW_WATCHDOG
#define CONFIG_SYS_WDTTIMERBASE		DAVINCI_TIMER1_BASE
#define CONFIG_SYS_WDT_PERIOD_LOW	0x0c000000
#define CONFIG_SYS_WDT_PERIOD_HIGH	0x0

#define CONFIG_CMD_DATE
#define CONFIG_RTC_DAVINCI

/* SD/MMC */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_DAVINCI_MMC
#define CONFIG_MMC_MBLOCK
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_FAT
#define CONFIG_CMD_MMC

/* GPIO */
#define CONFIG_ENBW_CMC_BOARD_TYPE	57
#define CONFIG_ENBW_CMC_HW_ID_BIT0	39
#define CONFIG_ENBW_CMC_HW_ID_BIT1	38
#define CONFIG_ENBW_CMC_HW_ID_BIT2	35

/* FDT support */
#define CONFIG_OF_LIBFDT

/* LowLevel Init */
/* PLL */
#define CONFIG_SYS_DV_CLKMODE		0
#define CONFIG_SYS_DA850_PLL0_POSTDIV	0
#define CONFIG_SYS_DA850_PLL0_PLLDIV1	0x8000
#define CONFIG_SYS_DA850_PLL0_PLLDIV2	0x8001
#define CONFIG_SYS_DA850_PLL0_PLLDIV3	0x8002 /* 150MHz */
#define CONFIG_SYS_DA850_PLL0_PLLDIV4	0x8003
#define CONFIG_SYS_DA850_PLL0_PLLDIV5	0x8002
#define CONFIG_SYS_DA850_PLL0_PLLDIV6	CONFIG_SYS_DA850_PLL0_PLLDIV1
#define CONFIG_SYS_DA850_PLL0_PLLDIV7	0x8005

#define CONFIG_SYS_DA850_PLL1_POSTDIV	1
#define CONFIG_SYS_DA850_PLL1_PLLDIV1	0x8000
#define CONFIG_SYS_DA850_PLL1_PLLDIV2	0x8001
#define CONFIG_SYS_DA850_PLL1_PLLDIV3	0x8002

#define CONFIG_SYS_DA850_PLL0_PLLM	18	/* PLL0 -> 456 MHz */
#define CONFIG_SYS_DA850_PLL1_PLLM	24	/* PLL1 -> 300 MHz */

/* DDR RAM */
#define CONFIG_SYS_DA850_DDR2_DDRPHYCR (DV_DDR_PHY_PWRDNEN | \
			DV_DDR_PHY_EXT_STRBEN   | \
			(0x4 << DV_DDR_PHY_RD_LATENCY_SHIFT))

#define CONFIG_SYS_DA850_DDR2_SDBCR (0 | \
		  (0 << DV_DDR_SDCR_DDR2TERM1_SHIFT) | \
		  (0 << DV_DDR_SDCR_MSDRAMEN_SHIFT) | \
		  (1 << DV_DDR_SDCR_DDR2EN_SHIFT) | \
		  (0x1 << DV_DDR_SDCR_DDREN_SHIFT)	| \
		  (0x1 << DV_DDR_SDCR_SDRAMEN_SHIFT)	| \
		  (0x1 << DV_DDR_SDCR_TIMUNLOCK_SHIFT)	| \
		  (0x1 << DV_DDR_SDCR_BUS_WIDTH_SHIFT)	| \
		  (0x3 << DV_DDR_SDCR_CL_SHIFT)		| \
		  (0x2 << DV_DDR_SDCR_IBANK_SHIFT)		| \
		  (0x2 << DV_DDR_SDCR_PAGESIZE_SHIFT))

#define CONFIG_SYS_DA850_DDR2_SDBCR2 4 /* 13 row address bits */

/*
 * freq = 150MHz -> t = 7ns
 */
#define CONFIG_SYS_DA850_DDR2_SDTIMR (0 | \
		(0x0d << DV_DDR_SDTMR1_RFC_SHIFT)	| \
		(1 << DV_DDR_SDTMR1_RP_SHIFT)		| \
		(1 << DV_DDR_SDTMR1_RCD_SHIFT)		| \
		(1 << DV_DDR_SDTMR1_WR_SHIFT)		| \
		(5 << DV_DDR_SDTMR1_RAS_SHIFT)		| \
		(7 << DV_DDR_SDTMR1_RC_SHIFT)		| \
		(1 << DV_DDR_SDTMR1_RRD_SHIFT)		| \
		(readl(&dv_ddr2_regs_ctrl->sdtimr) & 0x4) |  /* Reserved */ \
		((2 - 1) << DV_DDR_SDTMR1_WTR_SHIFT))

/*
 * freq = 150MHz -> t=7ns
 */
#define CONFIG_SYS_DA850_DDR2_SDTIMR2 (0 | \
	(readl(&dv_ddr2_regs_ctrl->sdtimr2) & 0x80000000) | /* Reserved */ \
	(8 << DV_DDR_SDTMR2_RASMAX_SHIFT)		| \
	(2 << DV_DDR_SDTMR2_XP_SHIFT)			| \
	(0 << DV_DDR_SDTMR2_ODT_SHIFT)			| \
	(15 << DV_DDR_SDTMR2_XSNR_SHIFT)		| \
	(27 << DV_DDR_SDTMR2_XSRD_SHIFT)		| \
	(0 << DV_DDR_SDTMR2_RTP_SHIFT)			| \
	(2 << DV_DDR_SDTMR2_CKE_SHIFT))

#define CONFIG_SYS_DA850_DDR2_SDRCR	0x00000407
#define CONFIG_SYS_DA850_DDR2_PBBPR	0x30
#define CONFIG_SYS_DA850_SYSCFG_SUSPSRC (DAVINCI_SYSCFG_SUSPSRC_TIMER0 | \
					DAVINCI_SYSCFG_SUSPSRC_SPI1 | \
					DAVINCI_SYSCFG_SUSPSRC_UART2 | \
					DAVINCI_SYSCFG_SUSPSRC_EMAC |\
					DAVINCI_SYSCFG_SUSPSRC_I2C)

#define CONFIG_SYS_DA850_CS2CFG	(DAVINCI_ABCR_WSETUP(2)	| \
				DAVINCI_ABCR_WSTROBE(6)	| \
				DAVINCI_ABCR_WHOLD(1)	| \
				DAVINCI_ABCR_RSETUP(2)	| \
				DAVINCI_ABCR_RSTROBE(6)	| \
				DAVINCI_ABCR_RHOLD(1)	| \
				DAVINCI_ABCR_ASIZE_16BIT)

#define CONFIG_SYS_DA850_CS3CFG	(DAVINCI_ABCR_WSETUP(1)	| \
				DAVINCI_ABCR_WSTROBE(2)	| \
				DAVINCI_ABCR_WHOLD(1)	| \
				DAVINCI_ABCR_RSETUP(1)	| \
				DAVINCI_ABCR_RSTROBE(6)	| \
				DAVINCI_ABCR_RHOLD(1)	| \
				DAVINCI_ABCR_ASIZE_8BIT)

/*
 * NOR Bootconfiguration word:
 * Method: Direc boot
 * EMIFA access mode: 16 Bit
 */
#define CONFIG_SYS_DV_NOR_BOOT_CFG	(0x11)

#define CONFIG_POST	(CONFIG_SYS_POST_MEMORY)
#define CONFIG_POST_EXTERNAL_WORD_FUNCS
#define CONFIG_SYS_POST_WORD_ADDR DAVINCI_RTC_BASE
#define CONFIG_LOGBUFFER
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_SYS_BOOTCOUNT_ADDR	DAVINCI_RTC_BASE
#define CONFIG_SYS_BOOTCOUNT_BE

#define CONFIG_SYS_NAND_U_BOOT_DST	0xc0080000
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x60004000
#define CONFIG_SYS_NAND_U_BOOT_SIZE	0x70000
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_NAND_U_BOOT_DST
#endif /* __CONFIG_H */

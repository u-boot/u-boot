/*
 * Copyright (C) 2009 Texas Instruments Incorporated
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_NO_FLASH		/* that is, no *NOR* flash */
#define CONFIG_SYS_CONSOLE_INFO_QUIET

/* SoC Configuration */
#define CONFIG_ARM926EJS				/* arm926ejs CPU */
#define CONFIG_SYS_TIMERBASE		0x01c21400	/* use timer 0 */
#define CONFIG_SYS_HZ_CLOCK		24000000	/* timer0 freq */
#define CONFIG_SYS_HZ			1000
#define CONFIG_SOC_DM365

#define CONFIG_MACH_TYPE	MACH_TYPE_DAVINCI_DM365_EVM

#define CONFIG_HOSTNAME			cam_enc_4xx

#define	CONFIG_BOARD_LATE_INIT
#define CONFIG_CAM_ENC_LED_MASK		0x0fc00000

/* Memory Info */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_1_SIZE		(256 << 20)	/* 256 MiB */
#define DDR_4BANKS				/* 4-bank DDR2 (256MB) */
#define CONFIG_MAX_RAM_BANK_SIZE	(256 << 20)	/* 256 MB */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* Serial Driver info: UART0 for console  */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#define CONFIG_SYS_NS16550_COM1		0x01c20000
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_HZ_CLOCK
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/* Network Configuration */
#define CONFIG_DRIVER_TI_EMAC
#define CONFIG_EMAC_MDIO_PHY_NUM	0
#define	CONFIG_SYS_EMAC_TI_CLKDIV	0xa9	/* 1MHz */
#define CONFIG_MII
#define CONFIG_BOOTP_DEFAULT
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT	10
#define CONFIG_CMD_MII
#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_RESET_PHY_R

/* I2C */
#define CONFIG_HARD_I2C
#define CONFIG_DRIVER_DAVINCI_I2C
#define CONFIG_SYS_I2C_SPEED		400000
#define CONFIG_SYS_I2C_SLAVE		0x10	/* SMBus host address */

/* NAND: socketed, two chipselects, normally 2 GBytes */
#define CONFIG_NAND_DAVINCI
#define CONFIG_SYS_NAND_CS		2
#define CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_NAND_4BIT_HW_ECC_OOBFIRST
#define CONFIG_SYS_NAND_PAGE_2K

#define CONFIG_SYS_NAND_LARGEPAGE
#define CONFIG_SYS_NAND_BASE_LIST	{ 0x02000000, }
/* socket has two chipselects, nCE0 gated by address BIT(14) */
#define CONFIG_SYS_MAX_NAND_DEVICE	1

/* SPI support */
#define CONFIG_SPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_DAVINCI_SPI
#define CONFIG_SYS_SPI_BASE		DAVINCI_SPI1_BASE
#define CONFIG_SYS_SPI_CLK		davinci_clk_get(SPI_PLLDIV)
#define CONFIG_SF_DEFAULT_SPEED		3000000
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED
#define CONFIG_CMD_SF

/* SD/MMC */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_DAVINCI_MMC
#define CONFIG_MMC_MBLOCK

/* U-Boot command configuration */
#include <config_cmd_default.h>

#define CONFIG_CMD_BDI
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_SETGETDCR
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVES

#ifdef CONFIG_CMD_BDI
#define CONFIG_CLOCKS
#endif

#ifdef CONFIG_MMC
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_MMC
#endif

#ifdef CONFIG_NAND_DAVINCI
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_NAND
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_RBTREE
#define CONFIG_LZO
#endif

#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC

/* U-Boot general configuration */
#define CONFIG_BOOTFILE		"uImage"	/* Boot file name */
#define CONFIG_SYS_PROMPT	"cam_enc_4xx> "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size  */
#define CONFIG_SYS_PBSIZE			/* Print buffer size */ \
		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_LONGHELP

#define CONFIG_MENU
#define CONFIG_MENU_SHOW
#define CONFIG_FIT
#define CONFIG_BOARD_IMG_ADDR_R 0x80000000

#ifdef CONFIG_NAND_DAVINCI
#define CONFIG_ENV_SIZE			(16 << 10)
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0x180000
#define CONFIG_ENV_RANGE		0x040000
#define CONFIG_ENV_OFFSET_REDUND	0x1c0000
#undef CONFIG_ENV_IS_IN_FLASH
#endif

#if defined(CONFIG_MMC) && !defined(CONFIG_ENV_IS_IN_NAND)
#define CONFIG_CMD_ENV
#define CONFIG_SYS_MMC_ENV_DEV	0
#define CONFIG_ENV_SIZE		(16 << 10)	/* 16 KiB */
#define CONFIG_ENV_OFFSET	(51 << 9)	/* Sector 51 */
#define CONFIG_ENV_IS_IN_MMC
#undef CONFIG_ENV_IS_IN_FLASH
#endif

#define CONFIG_BOOTDELAY	3
/*
 * 24MHz InputClock / 15 prediv -> 1.6 MHz timer running
 * Timeout 1 second.
 */
#define CONFIG_AIT_TIMER_TIMEOUT	0x186a00

#define CONFIG_CMDLINE_EDITING
#define CONFIG_VERSION_VARIABLE
#define CONFIG_TIMESTAMP

/* U-Boot memory configuration */
#define CONFIG_SYS_MALLOC_LEN		(1 << 20)	/* 1 MiB */
#define CONFIG_SYS_MEMTEST_START	0x80000000	/* physical address */
#define CONFIG_SYS_MEMTEST_END		0x81000000	/* test 16MB RAM */

/* Linux interfacing */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_SYS_BARGSIZE	1024			/* bootarg Size */
#define CONFIG_SYS_LOAD_ADDR	0x80700000		/* kernel address */

#define MTDIDS_DEFAULT		"nand0=davinci_nand.0"
#define MTDPARTS_DEFAULT			\
	"mtdparts="				\
		"davinci_nand.0:"		\
			"128k(spl),"		\
			"384k(UBLheader),"	\
			"1m(u-boot),"		\
			"512k(env),"		\
			"-(ubi)"

#define CONFIG_SYS_NAND_PAGE_SIZE	0x800
#define CONFIG_SYS_NAND_BLOCK_SIZE	0x20000

/* Defines for SPL */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_NAND_SIMPLE
#define CONFIG_SYS_NAND_HW_ECC_OOBFIRST
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_POST_MEM_SUPPORT
#define CONFIG_SPL_LDSCRIPT		"$(BOARDDIR)/u-boot-spl.lds"
#define CONFIG_SPL_STACK		(0x00010000 + 0x7f00)

#define CONFIG_SPL_TEXT_BASE		0x00000020 /*CONFIG_SYS_SRAM_START*/
#define CONFIG_SPL_MAX_SIZE		12320

#ifndef CONFIG_SPL_BUILD
#define CONFIG_SYS_TEXT_BASE		0x81080000
#endif

#define CONFIG_SYS_NAND_BASE		0x02000000
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					CONFIG_SYS_NAND_PAGE_SIZE)

#define CONFIG_SYS_NAND_ECCPOS		{				\
				24, 25, 26, 27, 28,			\
				29, 30, 31, 32, 33, 34, 35, 36, 37, 38,	\
				39, 40, 41, 42, 43, 44, 45, 46, 47, 48,	\
				49, 50, 51, 52, 53, 54, 55, 56, 57, 58,	\
				59, 60, 61, 62, 63 }
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
#define CONFIG_SYS_NAND_ECCSIZE		0x200
#define CONFIG_SYS_NAND_ECCBYTES	10
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_5_ADDR_CYCLE

/*
 * RBL searches from Block n (n = 1..24)
 * so we can define, how many UBL Headers
 * we can write before the real spl code
 */
#define CONFIG_SYS_NROF_PAGES_NAND_SPL	6

#define CONFIG_SYS_NAND_U_BOOT_DST	0x81080000 /* u-boot TEXT_BASE */
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_NAND_U_BOOT_DST

/*
 * Post tests for memory testing
 */
#define CONFIG_POST	CONFIG_SYS_POST_MEMORY
#define _POST_WORD_ADDR	0x0

#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SPL_STACK

#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000
#define CONFIG_SYS_NAND_U_BOOT_SIZE	0xa0000
#define CONFIG_SYS_NAND_U_BOOT_ERA_SIZE	0x100000

/* for UBL header */
#define CONFIG_SYS_UBL_BLOCK		(CONFIG_SYS_NAND_PAGE_SIZE)

#define CONFIG_SYS_DM36x_PLL1_PLLM	0x55
#define CONFIG_SYS_DM36x_PLL1_PREDIV	0x8005
#define CONFIG_SYS_DM36x_PLL2_PLLM	0x09
#define CONFIG_SYS_DM36x_PLL2_PREDIV	0x8000
#define CONFIG_SYS_DM36x_PERI_CLK_CTRL	0x243F04FC
#define CONFIG_SYS_DM36x_PLL1_PLLDIV1	0x801b
#define CONFIG_SYS_DM36x_PLL1_PLLDIV2	0x8001
/* POST DIV 680/2 = 340Mhz  -> MJCP and HDVICP bus interface clock */
#define CONFIG_SYS_DM36x_PLL1_PLLDIV3	0x8001
/*
 * POST DIV 680/4 = 170Mhz  -> EDMA/Peripheral CFG0(1/2 MJCP/HDVICP bus
 * interface clk)
 */
#define CONFIG_SYS_DM36x_PLL1_PLLDIV4	0x8003
/* POST DIV 680/2 = 340Mhz  -> VPSS */
#define CONFIG_SYS_DM36x_PLL1_PLLDIV5	0x8001
/* POST DIV 680/9 = 75.6 Mhz -> VENC */
#define CONFIG_SYS_DM36x_PLL1_PLLDIV6	0x8008
/*
 * POST DIV 680/1 = 680Mhz -> DDRx2(with internal divider of 2, clock boils
 * down to 340 Mhz)
 */
#define CONFIG_SYS_DM36x_PLL1_PLLDIV7	0x8000
/* POST DIV 680/7= 97Mhz-> MMC0/SD0 */
#define CONFIG_SYS_DM36x_PLL1_PLLDIV8	0x8006
/* POST DIV 680/28 = 24.3Mhz-> CLKOUT */
#define CONFIG_SYS_DM36x_PLL1_PLLDIV9	0x801b

#define CONFIG_SYS_DM36x_PLL2_PLLDIV1	0x8011
/* POST DIV 432/1=432 Mhz  -> ARM926/(HDVICP block) clk */
#define CONFIG_SYS_DM36x_PLL2_PLLDIV2	0x8000
#define CONFIG_SYS_DM36x_PLL2_PLLDIV3	0x8001
/* POST DIV 432/21= 20.5714 Mhz->VOICE Codec clk */
#define CONFIG_SYS_DM36x_PLL2_PLLDIV4	0x8014
/* POST DIV 432/16=27 Mhz  -> VENC(For SD modes, requires) */
#define CONFIG_SYS_DM36x_PLL2_PLLDIV5	0x800f

/*
 * READ LATENCY 7 (CL + 2)
 * CONFIG_PWRDNEN = 1
 * CONFIG_EXT_STRBEN = 1
 */
#define CONFIG_SYS_DM36x_DDR2_DDRPHYCR	(0 \
	| DV_DDR_PHY_EXT_STRBEN \
	| DV_DDR_PHY_PWRDNEN \
	| (7 << DV_DDR_PHY_RD_LATENCY_SHIFT))

/*
 * T_RFC = (trfc/DDR_CLK) - 1 = (195 / 2.941) - 1
 * T_RP  = (trp/DDR_CLK) - 1  = (12.5 / 2.941) - 1
 * T_RCD = (trcd/DDR_CLK) - 1 = (12.5 / 2.941) - 1
 * T_WR  = (twr/DDR_CLK) - 1  = (15 / 2.941) - 1
 * T_RAS = (tras/DDR_CLK) - 1 = (45 / 2.941) - 1
 * T_RC  = (trc/DDR_CLK) - 1  = (57.5 / 2.941) - 1
 * T_RRD = (trrd/DDR_CLK) - 1 = (7.5 / 2.941) - 1
 * T_WTR = (twtr/DDR_CLK) - 1 = (7.5 / 2.941) - 1
 */
#define CONFIG_SYS_DM36x_DDR2_SDTIMR	(0 \
	| (66 << DV_DDR_SDTMR1_RFC_SHIFT) \
	| (4  << DV_DDR_SDTMR1_RP_SHIFT) \
	| (4  << DV_DDR_SDTMR1_RCD_SHIFT) \
	| (5  << DV_DDR_SDTMR1_WR_SHIFT) \
	| (14 << DV_DDR_SDTMR1_RAS_SHIFT) \
	| (19 << DV_DDR_SDTMR1_RC_SHIFT) \
	| (2  << DV_DDR_SDTMR1_RRD_SHIFT) \
	| (2  << DV_DDR_SDTMR1_WTR_SHIFT))

/*
 * T_RASMAX = (trasmax/refresh_rate) - 1 = (70K / 7812.6) - 1
 * T_XP  = tCKE - 1 = 3 - 2
 * T_XSNR= ((trfc + 10)/DDR_CLK) - 1 = (205 / 2.941) - 1
 * T_XSRD = txsrd - 1 = 200 - 1
 * T_RTP = (trtp/DDR_CLK) - 1 = (7.5 / 2.941) - 1
 * T_CKE = tcke - 1     = 3 - 1
 */
#define CONFIG_SYS_DM36x_DDR2_SDTIMR2	(0 \
	| (8  << DV_DDR_SDTMR2_RASMAX_SHIFT) \
	| (2  << DV_DDR_SDTMR2_XP_SHIFT) \
	| (69 << DV_DDR_SDTMR2_XSNR_SHIFT) \
	| (199 <<  DV_DDR_SDTMR2_XSRD_SHIFT) \
	| (2 <<  DV_DDR_SDTMR2_RTP_SHIFT) \
	| (2 <<  DV_DDR_SDTMR2_CKE_SHIFT))

/* PR_OLD_COUNT = 0xfe */
#define CONFIG_SYS_DM36x_DDR2_PBBPR	0x000000FE
/* refresh rate = 0x768 */
#define CONFIG_SYS_DM36x_DDR2_SDRCR	0x00000768

#define CONFIG_SYS_DM36x_DDR2_SDBCR	(0 \
	| (2 << DV_DDR_SDCR_PAGESIZE_SHIFT) \
	| (3 << DV_DDR_SDCR_IBANK_SHIFT) \
	| (5 << DV_DDR_SDCR_CL_SHIFT) \
	| (1 << DV_DDR_SDCR_BUS_WIDTH_SHIFT)	\
	| (1 << DV_DDR_SDCR_TIMUNLOCK_SHIFT) \
	| (1 << DV_DDR_SDCR_DDREN_SHIFT) \
	| (0 << DV_DDR_SDCR_DDRDRIVE0_SHIFT)	\
	| (1 << DV_DDR_SDCR_DDR2EN_SHIFT) \
	| (1 << DV_DDR_SDCR_DDR_DDQS_SHIFT) \
	| (1 << DV_DDR_SDCR_BOOTUNLOCK_SHIFT))

#define CONFIG_SYS_DM36x_AWCCR	0xff
#define CONFIG_SYS_DM36x_AB1CR	0x40400204
#define CONFIG_SYS_DM36x_AB2CR	0x04ca2650

/* All Video Inputs */
#define CONFIG_SYS_DM36x_PINMUX0	0x00000000
/*
 * All Video Outputs,
 * GPIO 86, 87 + 90 0x0000f030
 */
#define CONFIG_SYS_DM36x_PINMUX1	0x00530002
#define CONFIG_SYS_DM36x_PINMUX2	0x00001815
/*
 * SPI1, UART1, I2C, SD0, SD1, McBSP0, CLKOUTs
 * GPIO 25 0x60000000
 */
#define CONFIG_SYS_DM36x_PINMUX3	0x9b5affff
/*
 * MMC/SD0 instead of MS, SPI0
 * GPIO 34 0x0000c000
 */
#define CONFIG_SYS_DM36x_PINMUX4	0x00002655

/*
 * Default environment settings
 */
#define xstr(s)	str(s)
#define str(s)	#s

#define DVN4XX_UBOOT_ADDR_R_RAM		0x80000000
/* (DVN4XX_UBOOT_ADDR_R_RAM + CONFIG_SYS_NAND_PAGE_SIZE) */
#define DVN4XX_UBOOT_ADDR_R_NAND_SPL	0x80000800
/*
 * (DVN4XX_UBOOT_ADDR_R_NAND_SPL + (CONFIG_SYS_NROF_PAGES_NAND_SPL * \
 * CONFIG_SYS_NAND_PAGE_SIZE))
 */
#define DVN4XX_UBOOT_ADDR_R_UBOOT	0x80003800

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"u_boot_addr_r=" xstr(DVN4XX_UBOOT_ADDR_R_RAM) "\0"		\
	"u-boot=" xstr(CONFIG_HOSTNAME) "/u-boot.ubl\0"			\
	"load=tftp ${u_boot_addr_r} ${u-boot}\0"			\
	"pagesz=" xstr(CONFIG_SYS_NAND_PAGE_SIZE) "\0"			\
	"writeheader=nandrbl rbl;nand erase 20000 ${pagesz};"		\
		"nand write ${u_boot_addr_r} 20000 ${pagesz};"		\
		"nandrbl uboot\0"					\
	"writenand_spl=nandrbl rbl;nand erase 0 3000;"			\
		"nand write " xstr(DVN4XX_UBOOT_ADDR_R_NAND_SPL)	\
		" 0 3000;nandrbl uboot\0"				\
	"writeuboot=nandrbl uboot;"					\
		"nand erase " xstr(CONFIG_SYS_NAND_U_BOOT_OFFS) " "	\
		 xstr(CONFIG_SYS_NAND_U_BOOT_ERA_SIZE)			\
		";nand write " xstr(DVN4XX_UBOOT_ADDR_R_UBOOT)		\
		" " xstr(CONFIG_SYS_NAND_U_BOOT_OFFS) " "		\
		xstr(CONFIG_SYS_NAND_U_BOOT_SIZE) "\0"			\
	"update=run load writenand_spl writeuboot\0"			\
	"bootcmd=run net_nfs\0"						\
	"rootpath=/opt/eldk-arm/arm\0"					\
	"mtdids=" MTDIDS_DEFAULT "\0"					\
	"mtdparts=" MTDPARTS_DEFAULT "\0"				\
	"netdev=eth0\0"							\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addmisc=setenv bootargs ${bootargs} app_reset=${app_reset}\0"	\
	"addcon=setenv bootargs ${bootargs} console=ttyS0,"		\
		"${baudrate}n8\0"					\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off eth=${ethaddr} panic=1\0"	\
	"rootpath=/opt/eldk-arm/arm\0"					\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"bootfile=" xstr(CONFIG_HOSTNAME) "/uImage \0"			\
	"kernel_addr_r=80600000\0"					\
	"load_kernel=tftp ${kernel_addr_r} ${bootfile}\0"		\
	"ubi_load_kernel=ubi part ubi 2048;ubifsmount ${img_volume};"	\
		"ubifsload ${kernel_addr_r} boot/uImage\0"		\
	"fit_addr_r=" xstr(CONFIG_BOARD_IMG_ADDR_R) "\0"		\
	"img_addr_r=" xstr(CONFIG_BOARD_IMG_ADDR_R) "\0"		\
	"img_file=" xstr(CONFIG_HOSTNAME) "/ait.itb\0"			\
	"header_addr=20000\0"						\
	"img_writeheader=nandrbl rbl;"					\
		"nand erase ${header_addr} ${pagesz};"			\
		"nand write ${img_addr_r} ${header_addr} ${pagesz};"	\
		"nandrbl uboot\0"					\
	"img_writespl=nandrbl rbl;nand erase 0 3000;"			\
		"nand write ${img_addr_r} 0 3000;nandrbl uboot\0"	\
	"img_writeuboot=nandrbl uboot;"					\
		"nand erase " xstr(CONFIG_SYS_NAND_U_BOOT_OFFS) " "	\
		 xstr(CONFIG_SYS_NAND_U_BOOT_ERA_SIZE)			\
		";nand write ${img_addr_r} "				\
		xstr(CONFIG_SYS_NAND_U_BOOT_OFFS) " "			\
		xstr(CONFIG_SYS_NAND_U_BOOT_SIZE) "\0"			\
	"img_writedfenv=ubi part ubi 2048;"				\
		"ubi write ${img_addr_r} default ${filesize}\0"		\
	"img_volume=rootfs1\0"						\
	"img_writeramdisk=ubi part ubi 2048;"				\
		"ubi write ${img_addr_r} ${img_volume} ${filesize}\0"	\
	"load_img=tftp ${fit_addr_r} ${img_file}\0"			\
	"net_nfs=run load_kernel; "					\
		"run nfsargs addip addcon addmtd addmisc;"		\
		"bootm ${kernel_addr_r}\0"				\
	"ubi_ubi=run ubi_load_kernel; "					\
		"run ubiargs addip addcon addmtd addmisc;"		\
		"bootm ${kernel_addr_r}\0"				\
	"ubiargs=setenv bootargs ubi.mtd=4,2048"			\
		" root=ubi0:${img_volume} rw rootfstype=ubifs\0"	\
	"app_reset=no\0"						\
	"dvn_app_vers=void\0"						\
	"dvn_boot_vers=void\0"						\
	"savenewvers=run savetmpparms restoreparms; saveenv;"		\
		"run restoretmpparms\0"					\
	"savetmpparms=setenv y_ipaddr ${ipaddr};"			\
		"setenv y_netmask ${netmask};"				\
		"setenv y_serverip ${serverip};"			\
		"setenv y_gatewayip ${gatewayip}\0"			\
	"saveparms=setenv x_ipaddr ${ipaddr};"				\
		"setenv x_netmask ${netmask};"				\
		"setenv x_serverip ${serverip};"			\
		"setenv x_gatewayip ${gatewayip}\0"			\
	"restoreparms=setenv ipaddr ${x_ipaddr};"			\
		"setenv netmask ${x_netmask};"				\
		"setenv serverip ${x_serverip};"			\
		"setenv gatewayip ${x_gatewayip}\0"			\
	"restoretmpparms=setenv ipaddr ${y_ipaddr};"			\
		"setenv netmask ${y_netmask};"				\
		"setenv serverip ${y_serverip};"			\
		"setenv gatewayip ${y_gatewayip}\0"			\
	"\0"

/* USB Configuration */
#define CONFIG_USB_DAVINCI
#define CONFIG_MUSB_HCD
#define CONFIG_DV_USBPHY_CTL (USBPHY_SESNDEN | USBPHY_VBDTCTEN | \
				USBPHY_PHY24MHZ)

#define CONFIG_CMD_USB         /* include support for usb cmd */
#define CONFIG_USB_STORAGE     /* MSC class support */
#define CONFIG_CMD_STORAGE     /* inclue support for usb-storage cmd */
#define CONFIG_CMD_FAT         /* inclue support for FAT/storage */
#define CONFIG_DOS_PARTITION   /* inclue support for FAT/storage */

#undef DAVINCI_DM365EVM
#define PINMUX4_USBDRVBUS_BITCLEAR       0x3000
#define PINMUX4_USBDRVBUS_BITSET         0x2000

#endif /* __CONFIG_H */

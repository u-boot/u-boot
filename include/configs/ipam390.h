/*
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 * Based on:
 * U-Boot:include/configs/da850evm.h
 *
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

/*
 * SoC Configuration
 */
#define CONFIG_SYS_EXCEPTION_VECTORS_HIGH
#define CONFIG_SYS_CLK_FREQ		clk_get(DAVINCI_ARM_CLKID)
#define CONFIG_SYS_OSCIN_FREQ		24000000
#define CONFIG_SYS_TIMERBASE		DAVINCI_TIMER0_BASE
#define CONFIG_SYS_HZ_CLOCK		clk_get(DAVINCI_AUXCLK_CLKID)

/*
 * Memory Info
 */
#define CONFIG_SYS_MALLOC_LEN	(0x10000 + 1*1024*1024) /* malloc() len */
#define PHYS_SDRAM_1		DAVINCI_DDR_EMIF_DATA_BASE /* DDR Start */
#define PHYS_SDRAM_1_SIZE	(128 << 20) /* SDRAM size 128MB */
#define CONFIG_MAX_RAM_BANK_SIZE (512 << 20) /* max size from SPRS586*/

/* memtest start addr */
#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM_1 + 0x2000000)

/* memtest will be run on 16MB */
#define CONFIG_SYS_MEMTEST_END	(CONFIG_SYS_MEMTEST_START + 16 * 1024 * 1024)

#define CONFIG_NR_DRAM_BANKS	1 /* we have 1 bank of DRAM */

#define CONFIG_SYS_DA850_SYSCFG_SUSPSRC (	\
	DAVINCI_SYSCFG_SUSPSRC_TIMER0 |		\
	DAVINCI_SYSCFG_SUSPSRC_UART2 |		\
	DAVINCI_SYSCFG_SUSPSRC_UART0 |		\
	DAVINCI_SYSCFG_SUSPSRC_EMAC)

/*
 * PLL configuration
 */

#define CONFIG_SYS_DA850_PLL0_PLLM     24
#define CONFIG_SYS_DA850_PLL1_PLLM     24

/*
 * DDR2 memory configuration
 */
#define CONFIG_SYS_DA850_DDR2_DDRPHYCR (DV_DDR_PHY_PWRDNEN | \
					DV_DDR_PHY_EXT_STRBEN | \
					(0x2 << DV_DDR_PHY_RD_LATENCY_SHIFT))
#define CONFIG_SYS_DA850_DDR2_SDRCR	0x00000498

#define CONFIG_SYS_DA850_DDR2_SDBCR2	0x00000004
#define CONFIG_SYS_DA850_DDR2_PBBPR	0x00000020

#define CONFIG_SYS_DA850_DDR2_SDTIMR (		\
	(13 << DV_DDR_SDTMR1_RFC_SHIFT) |	\
	(2 << DV_DDR_SDTMR1_RP_SHIFT) |		\
	(2 << DV_DDR_SDTMR1_RCD_SHIFT) |	\
	(2 << DV_DDR_SDTMR1_WR_SHIFT) |		\
	(5 << DV_DDR_SDTMR1_RAS_SHIFT) |	\
	(8 << DV_DDR_SDTMR1_RC_SHIFT) |		\
	(1 << DV_DDR_SDTMR1_RRD_SHIFT) |	\
	(1 << DV_DDR_SDTMR1_WTR_SHIFT))

#define CONFIG_SYS_DA850_DDR2_SDTIMR2 (		\
	(8 << DV_DDR_SDTMR2_RASMAX_SHIFT) |	\
	(2 << DV_DDR_SDTMR2_XP_SHIFT) |		\
	(0 << DV_DDR_SDTMR2_ODT_SHIFT) |	\
	(14 << DV_DDR_SDTMR2_XSNR_SHIFT) |	\
	(0xc7 << DV_DDR_SDTMR2_XSRD_SHIFT) |	\
	(1 << DV_DDR_SDTMR2_RTP_SHIFT) |	\
	(2 << DV_DDR_SDTMR2_CKE_SHIFT))

#define CONFIG_SYS_DA850_DDR2_SDBCR (		\
	(1 << DV_DDR_SDCR_DDR2EN_SHIFT) |	\
	(1 << DV_DDR_SDCR_DDRDRIVE0_SHIFT) |	\
	(1 << DV_DDR_SDCR_DDREN_SHIFT) |	\
	(1 << DV_DDR_SDCR_SDRAMEN_SHIFT) |	\
	(1 << DV_DDR_SDCR_BUS_WIDTH_SHIFT) |	\
	(2 << DV_DDR_SDCR_CL_SHIFT) |	\
	(3 << DV_DDR_SDCR_IBANK_SHIFT) |	\
	(2 << DV_DDR_SDCR_PAGESIZE_SHIFT))

#define CONFIG_SYS_DA850_CS3CFG	(DAVINCI_ABCR_WSETUP(1)	| \
				DAVINCI_ABCR_WSTROBE(2)	| \
				DAVINCI_ABCR_WHOLD(0)	| \
				DAVINCI_ABCR_RSETUP(1)	| \
				DAVINCI_ABCR_RSTROBE(2)	| \
				DAVINCI_ABCR_RHOLD(1)	| \
				DAVINCI_ABCR_TA(0)	| \
				DAVINCI_ABCR_ASIZE_8BIT)

/*
 * Serial Driver info
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4	/* NS16550 register size */
#define CONFIG_SYS_NS16550_COM1	DAVINCI_UART0_BASE /* Base address of UART0 */
#define CONFIG_SYS_NS16550_CLK	clk_get(DAVINCI_UART2_CLKID)
#define CONFIG_CONS_INDEX	1		/* use UART0 for console */

/*
 * Flash & Environment
 */
#define CONFIG_NAND_DAVINCI
#define CONFIG_ENV_OFFSET		0x0 /* Block 0--not used by bootcode */
#define CONFIG_ENV_SIZE			(128 << 10)
#define	CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_NAND_4BIT_HW_ECC_OOBFIRST
#define	CONFIG_SYS_NAND_PAGE_2K
#define CONFIG_SYS_NAND_CS		3
#define CONFIG_SYS_NAND_BASE		DAVINCI_ASYNC_EMIF_DATA_CE3_BASE
#define CONFIG_SYS_NAND_MASK_CLE		0x10
#define CONFIG_SYS_NAND_MASK_ALE		0x8
#undef CONFIG_SYS_NAND_HW_ECC
#define CONFIG_SYS_MAX_NAND_DEVICE	1 /* Max number of NAND devices */
#define CONFIG_SYS_NAND_HW_ECC_OOBFIRST
#define CONFIG_NAND_6BYTES_OOB_FREE_10BYTES_ECC
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_SIZE	(2 << 10)
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 << 10)
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x40000
#define CONFIG_SYS_NAND_U_BOOT_SIZE	0x120000
#define CONFIG_SYS_NAND_U_BOOT_DST	0xc1080000
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_NAND_U_BOOT_DST
#define CONFIG_SYS_NAND_U_BOOT_RELOC_SP	(CONFIG_SYS_NAND_U_BOOT_DST - \
					CONFIG_SYS_NAND_U_BOOT_SIZE - \
					CONFIG_SYS_MALLOC_LEN -       \
					GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_NAND_ECCPOS		{				\
			6,   7,  8,  9, 10,	11, 12, 13, 14, 15,	\
			22, 23, 24, 25, 26,	27, 28, 29, 30, 31,	\
			38, 39, 40, 41, 42,	43, 44, 45, 46, 47,	\
			54, 55, 56, 57, 58,	59, 60, 61, 62, 63}
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	10
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SPL_NAND_LOAD

/*
 * Network & Ethernet Configuration
 */
#ifdef CONFIG_DRIVER_TI_EMAC
#define CONFIG_DRIVER_TI_EMAC_USE_RMII
#define CONFIG_BOOTP_DEFAULT
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT	10
#endif

/*
 * U-Boot general configuration
 */
#define CONFIG_MISC_INIT_R
#define CONFIG_BOOTFILE		"uImage" /* Boot file name */
#define CONFIG_SYS_CBSIZE	1024 /* Console I/O Buffer Size	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Args Buffer Size */
#define CONFIG_SYS_LOAD_ADDR	(PHYS_SDRAM_1 + 0x700000)
#define CONFIG_MX_CYCLIC

/*
 * Linux Information
 */
#define LINUX_BOOT_PARAM_ADDR	(PHYS_SDRAM_1 + 0x100)
#define CONFIG_HWCONFIG		/* enable hwconfig */
#define CONFIG_CMDLINE_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_EXTRA_ENV_SETTINGS \
	"defbootargs=setenv bootargs mem=128M console=ttyS0,115200n8 " \
		"root=/dev/mtdblock5 rw noinitrd " \
		"rootfstype=jffs2 noinitrd\0" \
	"hwconfig=dsp:wake=yes\0" \
	"bootcmd=nboot kernel;run defbootargs addmtd;bootm 0xc0700000\0" \
	"bootfile=uImage\0" \
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"	\
	"mtddevname=uboot-env\0" \
	"mtddevnum=0\0" \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0"				\
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0"			\
	"u-boot=/tftpboot/ipam390/u-boot.ais\0"			\
	"upd_uboot=tftp c0000000 ${u-boot};nand erase.part u-boot;" \
		"nand write c0000000 20000 ${filesize}\0"	\
	"setbootparms=nand read c0100000 200000 400000;"	\
		"run defbootargs addmtd;"			\
		"spl export atags c0100000;"			\
		"nand erase.part bootparms;"			\
		"nand write c0000100 180000 20000\0"		\
	"\0"

#ifdef CONFIG_CMD_BDI
#define CONFIG_CLOCKS
#endif

#ifndef CONFIG_DRIVER_TI_EMAC
#endif

#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS

/* defines for SPL */
#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SYS_TEXT_BASE - \
						CONFIG_SYS_MALLOC_LEN)
#define CONFIG_SYS_SPL_MALLOC_SIZE	CONFIG_SYS_MALLOC_LEN
#define CONFIG_SPL_STACK	0x8001ff00
#define CONFIG_SPL_TEXT_BASE	0x80000000
#define CONFIG_SPL_MAX_SIZE	0x20000
#define CONFIG_SPL_MAX_FOOTPRINT	32768

/* additions for new relocation code, must added to all boards */
#define CONFIG_SYS_SDRAM_BASE		0xc0000000

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x1000 - \
					GENERATED_GBL_DATA_SIZE)

/* add FALCON boot mode */
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS	0x00200000
#define CONFIG_SYS_SPL_ARGS_ADDR	LINUX_BOOT_PARAM_ADDR

/* GPIO support */
#define CONFIG_DA8XX_GPIO
#define CONFIG_IPAM390_GPIO_BOOTMODE	((16 * 7) + 14)

#define CONFIG_SHOW_BOOT_PROGRESS
#define CONFIG_IPAM390_GPIO_LED_RED	((16 * 7) + 11)
#define CONFIG_IPAM390_GPIO_LED_GREEN	((16 * 7) + 12)

#include <asm/arch/hardware.h>

#endif /* __CONFIG_H */

/*
 * Copyright (C) 2011-2014 OMICRON electronics GmbH
 *
 * Based on da850evm.h. Original Copyrights follow:
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
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
#define MACH_TYPE_CALIMAIN	3528
#define CONFIG_MACH_TYPE	MACH_TYPE_CALIMAIN

/*
 * SoC Configuration
 */
#define CONFIG_MACH_DAVINCI_CALIMAIN
#define CONFIG_SOC_DA8XX		/* TI DA8xx SoC */
#define CONFIG_SOC_DA850		/* TI DA850 SoC */
#define CONFIG_SYS_EXCEPTION_VECTORS_HIGH
#define CONFIG_SYS_CLK_FREQ		clk_get(DAVINCI_ARM_CLKID)
#define CONFIG_SYS_OSCIN_FREQ		calimain_get_osc_freq()
#define CONFIG_SYS_TIMERBASE		DAVINCI_TIMER0_BASE
#define CONFIG_SYS_HZ_CLOCK		clk_get(DAVINCI_AUXCLK_CLKID)
#define CONFIG_SYS_TEXT_BASE		0x60000000
#define CONFIG_DA850_LOWLEVEL
#define CONFIG_SYS_DA850_PLL_INIT
#define CONFIG_SYS_DA850_DDR_INIT
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_DA8XX_GPIO
#define CONFIG_HW_WATCHDOG
#define CONFIG_SYS_WDTTIMERBASE	DAVINCI_TIMER1_BASE
#define CONFIG_SYS_WDT_PERIOD_LOW \
	(60 * CONFIG_SYS_OSCIN_FREQ) /* 60 s heartbeat */
#define CONFIG_SYS_WDT_PERIOD_HIGH	0x0
#define CONFIG_SYS_DV_NOR_BOOT_CFG	(0x11)

/*
 * PLL configuration
 */
#define CONFIG_SYS_DV_CLKMODE          0
#define CONFIG_SYS_DA850_PLL0_POSTDIV  1
#define CONFIG_SYS_DA850_PLL0_PLLDIV1  0x8000
#define CONFIG_SYS_DA850_PLL0_PLLDIV2  0x8001
#define CONFIG_SYS_DA850_PLL0_PLLDIV3  0x8002
#define CONFIG_SYS_DA850_PLL0_PLLDIV4  0x8003
#define CONFIG_SYS_DA850_PLL0_PLLDIV5  0x8002
#define CONFIG_SYS_DA850_PLL0_PLLDIV6  CONFIG_SYS_DA850_PLL0_PLLDIV1
#define CONFIG_SYS_DA850_PLL0_PLLDIV7  0x8005

#define CONFIG_SYS_DA850_PLL1_POSTDIV  1
#define CONFIG_SYS_DA850_PLL1_PLLDIV1  0x8000
#define CONFIG_SYS_DA850_PLL1_PLLDIV2  0x8001
#define CONFIG_SYS_DA850_PLL1_PLLDIV3  0x8002

#define CONFIG_SYS_DA850_PLL0_PLLM \
	((calimain_get_osc_freq() == 25000000) ? 23 : 24)
#define CONFIG_SYS_DA850_PLL1_PLLM \
	((calimain_get_osc_freq() == 25000000) ? 20 : 21)

/*
 * DDR2 memory configuration
 */
#define CONFIG_SYS_DA850_DDR2_DDRPHYCR (DV_DDR_PHY_PWRDNEN | \
					DV_DDR_PHY_EXT_STRBEN | \
					(0x4 << DV_DDR_PHY_RD_LATENCY_SHIFT))

#define CONFIG_SYS_DA850_DDR2_SDBCR (		\
	(1 << DV_DDR_SDCR_DDR2EN_SHIFT) |	\
	(1 << DV_DDR_SDCR_DDRDRIVE0_SHIFT) |	\
	(1 << DV_DDR_SDCR_DDREN_SHIFT) |	\
	(1 << DV_DDR_SDCR_SDRAMEN_SHIFT) |	\
	(1 << DV_DDR_SDCR_BUS_WIDTH_SHIFT) |	\
	(0x3 << DV_DDR_SDCR_CL_SHIFT) |		\
	(0x3 << DV_DDR_SDCR_IBANK_SHIFT) |	\
	(0x2 << DV_DDR_SDCR_PAGESIZE_SHIFT))

/* SDBCR2 is only used if IBANK_POS bit in SDBCR is set */
#define CONFIG_SYS_DA850_DDR2_SDBCR2	0

#define CONFIG_SYS_DA850_DDR2_SDTIMR (		\
	(16 << DV_DDR_SDTMR1_RFC_SHIFT) |	\
	(1 << DV_DDR_SDTMR1_RP_SHIFT) |		\
	(1 << DV_DDR_SDTMR1_RCD_SHIFT) |	\
	(1 << DV_DDR_SDTMR1_WR_SHIFT) |		\
	(5 << DV_DDR_SDTMR1_RAS_SHIFT) |	\
	(7 << DV_DDR_SDTMR1_RC_SHIFT) |		\
	(1 << DV_DDR_SDTMR1_RRD_SHIFT) |	\
	(1 << DV_DDR_SDTMR1_WTR_SHIFT))

#define CONFIG_SYS_DA850_DDR2_SDTIMR2 (		\
	(7 << DV_DDR_SDTMR2_RASMAX_SHIFT) |	\
	(2 << DV_DDR_SDTMR2_XP_SHIFT) |		\
	(0 << DV_DDR_SDTMR2_ODT_SHIFT) |	\
	(18 << DV_DDR_SDTMR2_XSNR_SHIFT) |	\
	(199 << DV_DDR_SDTMR2_XSRD_SHIFT) |	\
	(0 << DV_DDR_SDTMR2_RTP_SHIFT) |	\
	(2 << DV_DDR_SDTMR2_CKE_SHIFT))

#define CONFIG_SYS_DA850_DDR2_SDRCR	0x000003FF
#define CONFIG_SYS_DA850_DDR2_PBBPR	0x30

/*
 * Flash memory timing
 */

#define CONFIG_SYS_DA850_CS2CFG	(	\
	DAVINCI_ABCR_WSETUP(2) |	\
	DAVINCI_ABCR_WSTROBE(5)	|	\
	DAVINCI_ABCR_WHOLD(3) |		\
	DAVINCI_ABCR_RSETUP(1) |	\
	DAVINCI_ABCR_RSTROBE(14) |	\
	DAVINCI_ABCR_RHOLD(0) |		\
	DAVINCI_ABCR_TA(3) |		\
	DAVINCI_ABCR_ASIZE_16BIT)

/* single 64 MB NOR flash device connected to CS2 and CS3 */
#define CONFIG_SYS_DA850_CS3CFG CONFIG_SYS_DA850_CS2CFG

/*
 * Memory Info
 */
#define CONFIG_SYS_MALLOC_LEN	(0x10000 + 1*1024*1024) /* malloc() len */
#define PHYS_SDRAM_1		DAVINCI_DDR_EMIF_DATA_BASE /* DDR Start */
#define PHYS_SDRAM_1_SIZE	(128 << 20) /* SDRAM size 128MB */
#define CONFIG_MAX_RAM_BANK_SIZE (512 << 20) /* max size from SPRS586*/

#define CONFIG_SYS_DA850_SYSCFG_SUSPSRC (	\
	DAVINCI_SYSCFG_SUSPSRC_TIMER0 |		\
	DAVINCI_SYSCFG_SUSPSRC_SPI1 |		\
	DAVINCI_SYSCFG_SUSPSRC_UART2 |		\
	DAVINCI_SYSCFG_SUSPSRC_EMAC |		\
	DAVINCI_SYSCFG_SUSPSRC_I2C)

/* memtest start addr */
#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM_1 + 0x2000000)

/* memtest will be run on 16MB */
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + (16 << 20))

#define CONFIG_NR_DRAM_BANKS	1 /* we have 1 bank of DRAM */

/*
 * Serial Driver info
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4	/* NS16550 register size */
#define CONFIG_SYS_NS16550_COM1	DAVINCI_UART2_BASE /* Base address of UART2 */
#define CONFIG_SYS_NS16550_CLK	clk_get(DAVINCI_UART2_CLKID)
#define CONFIG_CONS_INDEX	1		/* use UART0 for console */
#define CONFIG_BAUDRATE		115200		/* Default baud rate */

#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_SYS_MAX_FLASH_BANKS  1 /* max number of flash banks */
#define CONFIG_SYS_FLASH_SECT_SZ    (128 << 10) /* 128KB */
#define CONFIG_SYS_FLASH_BASE       DAVINCI_ASYNC_EMIF_DATA_CE2_BASE
#define CONFIG_ENV_SECT_SIZE        CONFIG_SYS_FLASH_SECT_SZ
#define CONFIG_ENV_ADDR \
	(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_FLASH_SECT_SZ * 2)
#define CONFIG_ENV_SIZE             (128 << 10)
#define CONFIG_ENV_ADDR_REDUND      (CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND      CONFIG_ENV_SIZE
#define PHYS_FLASH_SIZE             (64 << 20) /* Flash size 64MB */
#define CONFIG_SYS_MAX_FLASH_SECT \
	((PHYS_FLASH_SIZE/CONFIG_SYS_FLASH_SECT_SZ) + 3)

/*
 * Network & Ethernet Configuration
 */
#ifdef CONFIG_DRIVER_TI_EMAC
#define CONFIG_EMAC_MDIO_PHY_NUM	1
#define CONFIG_MII
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT	10
#endif

/*
 * U-Boot general configuration
 */
#define CONFIG_BOOTFILE        "uImage" /* Boot file name */
#define CONFIG_SYS_CBSIZE      1024 /* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE      (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS     16 /* max number of command args */
#define CONFIG_SYS_BARGSIZE    CONFIG_SYS_CBSIZE /* Boot Args Buffer Size */
#define CONFIG_SYS_LOAD_ADDR   (PHYS_SDRAM_1 + 0x700000)
#define CONFIG_LOADADDR        0xc0700000
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP
#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC

/*
 * Linux Information
 */
#define LINUX_BOOT_PARAM_ADDR     (PHYS_SDRAM_1 + 0x100)
#define CONFIG_CMDLINE_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_BOOTARGS           ""
#define CONFIG_BOOTCOMMAND        "run checkupdate; run checkbutton;"
#define CONFIG_BOOTDELAY          0
#define CONFIG_ZERO_BOOTDELAY_CHECK   /* check for keypress on bootdelay==0 */
#define CONFIG_BOOT_RETRY_TIME    60  /* continue boot after 60 s inactivity */
#define CONFIG_RESET_TO_RETRY

/*
 * Default environment settings
 * gpio0 = button, gpio1 = led green, gpio2 = led red
 * verify = n ... disable kernel checksum verification for faster booting
 */
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"tftpdir=calimero\0"						\
	"flashkernel=tftpboot $loadaddr $tftpdir/uImage; "		\
		"erase 0x60800000 +0x400000; "				\
		"cp.b $loadaddr 0x60800000 $filesize\0"			\
	"flashrootfs="							\
		"tftpboot $loadaddr $tftpdir/rootfs.jffs2; "		\
		"erase 0x60c00000 +0x2e00000; "				\
		"cp.b $loadaddr 0x60c00000 $filesize\0"			\
	"flashuboot=tftpboot $loadaddr $tftpdir/u-boot.bin; "		\
		"protect off all; "					\
		"erase 0x60000000 +0x80000; "				\
		"cp.b $loadaddr 0x60000000 $filesize\0"			\
	"flashrlk=tftpboot $loadaddr $tftpdir/uImage-rlk; "		\
		"erase 0x60080000 +0x780000; "				\
		"cp.b $loadaddr 0x60080000 $filesize\0"			\
	"erase_persistent=erase 0x63a00000 +0x600000;\0"		\
	"bootnor=setenv bootargs console=ttyS2,115200n8 "		\
		"root=/dev/mtdblock3 rw rootfstype=jffs2 "		\
		"rootwait ethaddr=$ethaddr; "				\
		"gpio c 1; gpio s 2; bootm 0x60800000\0"		\
	"bootrlk=gpio s 1; gpio s 2;"					\
		"setenv bootargs console=ttyS2,115200n8 "		\
		"ethaddr=$ethaddr; bootm 0x60080000\0"			\
	"boottftp=setenv bootargs console=ttyS2,115200n8 "		\
		"root=/dev/mtdblock3 rw rootfstype=jffs2 "		\
		"rootwait ethaddr=$ethaddr; "				\
		"tftpboot $loadaddr $tftpdir/uImage;"			\
		"gpio c 1; gpio s 2; bootm $loadaddr\0"			\
	"checkupdate=if test -n $update_flag; then "			\
		"echo Previous update failed - starting RLK; "		\
		"run bootrlk; fi; "					\
		"if test -n $initial_setup; then "			\
		"echo Running initial setup procedure; "		\
		"sleep 1; run flashall; fi\0"				\
	"product=accessory\0"						\
	"serial=XX12345\0"						\
	"checknor="							\
		"if gpio i 0; then run bootnor; fi;\0"			\
	"checkrlk="							\
		"if gpio i 0; then run bootrlk; fi;\0"			\
	"checkbutton="							\
		"run checknor; sleep 1;"				\
		"run checknor; sleep 1;"				\
		"run checknor; sleep 1;"				\
		"run checknor; sleep 1;"				\
		"run checknor;"						\
		"gpio s 1; gpio s 2;"					\
		"echo ---- Release button to boot RLK ----;"		\
		"run checkrlk; sleep 1;"				\
		"run checkrlk; sleep 1;"				\
		"run checkrlk; sleep 1;"				\
		"run checkrlk; sleep 1;"				\
		"run checkrlk; sleep 1;"				\
		"run checkrlk;"						\
		"echo ---- Factory reset requested ----;"		\
		"gpio c 1;"						\
		"setenv factory_reset true;"				\
		"saveenv;"						\
		"run bootnor;\0"					\
	"flashall=run flashrlk;"					\
		"run flashkernel;"					\
		"run flashrootfs;"					\
		"setenv erase_datafs true;"				\
		"setenv initial_setup;"					\
		"saveenv;"						\
		"run bootnor;\0"					\
	"verify=n\0"							\
	"clearenv=protect off all;"					\
		"erase 0x60040000 +0x40000;\0"				\
	"bootlimit=3\0"							\
	"altbootcmd=run bootrlk\0"

#define CONFIG_PREBOOT			\
	"echo Version: $ver; "		\
	"echo Serial: $serial; "	\
	"echo MAC: $ethaddr; "		\
	"echo Product: $product; "	\
	"gpio c 1; gpio c 2;"

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

#ifndef CONFIG_DRIVER_TI_EMAC
#undef CONFIG_CMD_DHCP
#undef CONFIG_CMD_MII
#undef CONFIG_CMD_PING
#endif

/* additions for new relocation code, must added to all boards */
#define CONFIG_SYS_SDRAM_BASE		0xc0000000
/* initial stack pointer in internal SRAM */
#define CONFIG_SYS_INIT_SP_ADDR		(0x8001ff00)

#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_SYS_BOOTCOUNT_LE		/* Use little-endian accessors */
#define CONFIG_SYS_BOOTCOUNT_ADDR	DAVINCI_RTC_BASE

#ifndef __ASSEMBLY__
int calimain_get_osc_freq(void);
#endif

#endif /* __CONFIG_H */

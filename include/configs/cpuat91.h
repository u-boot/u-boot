/*
 * CPUAT91 by (C) Copyright 2006-2010 Eric Benard
 * eric@eukrea.com
 *
 * Configuration settings for the CPUAT91 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#ifndef _CONFIG_CPUAT91_H
#define _CONFIG_CPUAT91_H

#include <asm/sizes.h>

#ifdef CONFIG_RAMBOOT
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_TEXT_BASE		0x21F00000
#else
#define CONFIG_BOOTDELAY		1
#define CONFIG_SYS_TEXT_BASE		0
#endif

#define AT91C_XTAL_CLOCK		18432000
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768
#define AT91C_MAIN_CLOCK		((AT91C_XTAL_CLOCK / 4) * 39)
#define AT91C_MASTER_CLOCK		(AT91C_MAIN_CLOCK / 3)
#define CONFIG_SYS_HZ_CLOCK		(AT91C_MASTER_CLOCK / 2)
#define CONFIG_SYS_HZ			1000

#define CONFIG_ARM920T
#define CONFIG_AT91RM9200
#define CONFIG_CPUAT91
#define USE_920T_MMU

#include <asm/hardware.h>	/* needed for port definitions */

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_BOARD_EARLY_INIT_F

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_USE_MAIN_OSCILLATOR
/* flash */
#define CONFIG_SYS_MC_PUIA_VAL	0x00000000
#define CONFIG_SYS_MC_PUP_VAL	0x00000000
#define CONFIG_SYS_MC_PUER_VAL	0x00000000
#define CONFIG_SYS_MC_ASR_VAL	0x00000000
#define CONFIG_SYS_MC_AASR_VAL	0x00000000
#define CONFIG_SYS_EBI_CFGR_VAL	0x00000000
#define CONFIG_SYS_SMC_CSR0_VAL	0x00003284 /* 16bit, 2 TDF, 4 WS */

/* clocks */
#define CONFIG_SYS_PLLAR_VAL	0x20263E04 /* 179.712000 MHz for PCK */
#define CONFIG_SYS_PLLBR_VAL	0x10483E0E /* 48.054857 MHz for USB */
#define CONFIG_SYS_MCKR_VAL	0x00000202 /* PCK/3 = MCK Master Clock */

/* sdram */
#define CONFIG_SYS_PIOC_ASR_VAL	0xFFFF0000 /* Configure PIOC as D16/D31 */
#define CONFIG_SYS_PIOC_BSR_VAL	0x00000000
#define CONFIG_SYS_PIOC_PDR_VAL	0xFFFF0000
#define CONFIG_SYS_EBI_CSA_VAL	0x00000002 /* CS1=SDRAM */
#define CONFIG_SYS_SDRC_CR_VAL	0x2188C155 /* set up the SDRAM */
#define CONFIG_SYS_SDRAM	0x20000000 /* address of the SDRAM */
#define CONFIG_SYS_SDRAM1	0x20000080 /* address of the SDRAM */
#define CONFIG_SYS_SDRAM_VAL	0x00000000 /* value written to SDRAM */
#define CONFIG_SYS_SDRC_MR_VAL	0x00000002 /* Precharge All */
#define CONFIG_SYS_SDRC_MR_VAL1	0x00000004 /* refresh */
#define CONFIG_SYS_SDRC_MR_VAL2	0x00000003 /* Load Mode Register */
#define CONFIG_SYS_SDRC_MR_VAL3	0x00000000 /* Normal Mode */
#define CONFIG_SYS_SDRC_TR_VAL	0x000002E0 /* Write refresh rate */
#endif	/* CONFIG_SKIP_LOWLEVEL_INIT */

#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE	ATMEL_BASE_DBGU
#define CONFIG_USART_ID		0/* ignored in arm */

#undef CONFIG_HARD_I2C
#define AT91_PIN_SDA			(1<<25)
#define AT91_PIN_SCL			(1<<26)

#define CONFIG_SYS_I2C_INIT_BOARD
#define	CONFIG_SYS_I2C_SPEED		50000
#define CONFIG_SYS_I2C_SLAVE		0

#define I2C_INIT	i2c_init_board();
#define I2C_ACTIVE	writel(AT91_PMX_AA_TWD, &pio->pioa.mddr);
#define I2C_TRISTATE	writel(AT91_PMX_AA_TWD, &pio->pioa.mder);
#define I2C_READ	((readl(&pio->pioa.pdsr) & AT91_PMX_AA_TWD) != 0)
#define I2C_SDA(bit)						\
	if (bit)						\
		writel(AT91_PMX_AA_TWD, &pio->pioa.sodr);	\
	else							\
		writel(AT91_PMX_AA_TWD, &pio->pioa.codr);
#define I2C_SCL(bit)						\
	if (bit)						\
		writel(AT91_PMX_AA_TWCK, &pio->pioa.sodr);	\
	else							\
		writel(AT91_PMX_AA_TWCK, &pio->pioa.codr);

#define I2C_DELAY	udelay(2500000/CONFIG_SYS_I2C_SPEED)

#define CONFIG_SYS_I2C_EEPROM_ADDR		0x54
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	1
#define	CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10

#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_MII
#define CONFIG_CMD_CACHE
#undef CONFIG_CMD_USB
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_DHCP

#ifdef CONFIG_SYS_I2C_SOFT
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#endif

#define CONFIG_NR_DRAM_BANKS			1
#define CONFIG_SYS_SDRAM_BASE			0x20000000
#define CONFIG_SYS_SDRAM_SIZE			(32 * 1024 * 1024)

#define CONFIG_SYS_MEMTEST_START		CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END			\
	(CONFIG_SYS_MEMTEST_START + CONFIG_SYS_SDRAM_SIZE - 512 * 1024)

#define CONFIG_DRIVER_AT91EMAC
#define CONFIG_SYS_RX_ETH_BUFFER	16
#define CONFIG_RMII
#define CONFIG_MII
#define CONFIG_DRIVER_AT91EMAC_PHYADDR	1
#define CONFIG_NET_RETRY_COUNT			20
#define CONFIG_KS8721_PHY

#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_SYS_MAX_FLASH_BANKS		1
#define CONFIG_SYS_FLASH_PROTECTION
#define PHYS_FLASH_1				0x10000000
#define CONFIG_SYS_FLASH_BASE			PHYS_FLASH_1
#define CONFIG_SYS_MAX_FLASH_SECT		128
#define CONFIG_SYS_FLASH_CFI_WIDTH		FLASH_CFI_16BIT
#define CONFIG_SYS_MONITOR_BASE			PHYS_FLASH_1
#define PHYS_FLASH_SIZE				(16 * 1024 * 1024)
#define CONFIG_SYS_FLASH_BANKS_LIST		\
		{ PHYS_FLASH_1 }

#if defined(CONFIG_CMD_USB)
#define CONFIG_USB_ATMEL
#define CONFIG_USB_OHCI_NEW
#define CONFIG_USB_STORAGE
#define CONFIG_DOS_PARTITION
#define CONFIG_AT91C_PQFP_UHPBU
#undef CONFIG_SYS_USB_OHCI_BOARD_INIT
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE		AT91_USB_HOST_BASE
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91rm9200"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15
#endif

#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_ADDR				(PHYS_FLASH_1 + 128 * 1024)
#define CONFIG_ENV_SIZE				(128 * 1024)
#define CONFIG_ENV_SECT_SIZE		(128 * 1024)

#define CONFIG_SYS_LOAD_ADDR		0x21000000

#define CONFIG_BAUDRATE			115200

#define CONFIG_SYS_PROMPT		"CPUAT91=> "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		32
#define CONFIG_SYS_PBSIZE		\
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_CMDLINE_EDITING

#define CONFIG_SYS_MALLOC_LEN		\
			ROUND(3 * CONFIG_ENV_SIZE + 128 * 1024, 4 * 1024)

#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_SDRAM_BASE + 4 * 1024 - \
				GENERATED_GBL_DATA_SIZE)

#define CONFIG_DEVICE_NULLDEV
#define CONFIG_SILENT_CONSOLE

#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT		\
	"Press SPACE to abort autoboot\n"
#define CONFIG_AUTOBOOT_STOP_STR	" "
#define CONFIG_AUTOBOOT_DELAY_STR	"d"

#define CONFIG_VERSION_VARIABLE

#define MTDIDS_DEFAULT			"nor0=physmap-flash.0"
#define MTDPARTS_DEFAULT		\
	"mtdparts=physmap-flash.0:"	\
		"128k(u-boot)ro,"	\
		"128k(u-boot-env),"	\
		"1792k(kernel),"	\
		"-(rootfs)"

#define CONFIG_BOOTARGS 		\
	"root=/dev/mtdblock3 rootfstype=jffs2 console=ttyS0,115200"

#define CONFIG_BOOTCOMMAND		"run flashboot"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"mtdid=" MTDIDS_DEFAULT "\0"					\
	"mtdparts=" MTDPARTS_DEFAULT "\0"				\
	"flub=tftp 21000000 cpuat91/u-boot.bin; protect off 10000000 "	\
		"1001FFFF; erase 10000000 1001FFFF; cp.b 21000000 "	\
		"10000000 ${filesize}\0"				\
	"flui=tftp 21000000 cpuat91/uImage; protect off 10040000 "	\
		"1019ffff; erase 10040000 101fffff; cp.b 21000000 "	\
		"10040000 ${filesize}\0"				\
	"flrfs=tftp 21000000 cpuat91/rootfs.jffs2; protect off "	\
		"10200000 10ffffff; erase 10200000 10ffffff; cp.b "	\
		"21000000 10200000 ${filesize}\0"			\
	"ramargs=setenv bootargs $(bootargs) $(mtdparts)\0"		\
	"flashboot=run ramargs;bootm 10040000\0"			\
	"netboot=run ramargs;tftpboot 21000000 cpuat91/uImage;"		\
		"bootm 21000000\0"
#endif	/* _CONFIG_CPUAT91_H */

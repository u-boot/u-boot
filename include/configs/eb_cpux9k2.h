/*
 * (C) Copyright 2008-2009
 * BuS Elektronik GmbH & Co. KG <www.bus-elektronik.de>
 * Jens Scharsig <esw@bus-elektronik.de>
 *
 * Configuation settings for the EB+CPUx9K2 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_EB_CPUx9K2_H_
#define _CONFIG_EB_CPUx9K2_H_

/*--------------------------------------------------------------------------*/

#define CONFIG_AT91RM9200		/* It's an Atmel AT91RM9200 SoC	*/
#define CONFIG_EB_CPUX9K2		/* on an EP+CPUX9K2 Board	*/
#define USE_920T_MMU

#define CONFIG_VERSION_VARIABLE
#define CONFIG_IDENT_STRING	" on EB+CPUx9K2"

#include <asm/hardware.h>	/* needed for port definitions */

#define CONFIG_MISC_INIT_R
#define CONFIG_BOARD_EARLY_INIT_F

#define MACH_TYPE_EB_CPUX9K2		1977
#define CONFIG_MACH_TYPE		MACH_TYPE_EB_CPUX9K2

#define CONFIG_SYS_CACHELINE_SIZE	32
#define CONFIG_SYS_DCACHE_OFF

/*--------------------------------------------------------------------------*/
#ifndef CONFIG_RAMBOOT
#define CONFIG_SYS_TEXT_BASE		0x00000000
#else
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_TEXT_BASE		0x21800000
#endif
#define CONFIG_SYS_LOAD_ADDR		0x21000000  /* default load address */
#define CONFIG_STANDALONE_LOAD_ADDR	0x21000000

#define CONFIG_BOOT_RETRY_TIME		30
#define CONFIG_CMDLINE_EDITING

#define CONFIG_SYS_PROMPT	"U-Boot> "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	512		/* Console I/O Buffer Size */
#define CONFIG_SYS_MAXARGS	32		/* max number of command args */
#define CONFIG_SYS_PBSIZE	\
	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */

/*
 * ARM asynchronous clock
 */

#define AT91C_MAIN_CLOCK	179404800	/* from 12.288 MHz * 73 / 5 */
#define AT91C_MASTER_CLOCK	(AT91C_MAIN_CLOCK / 3)
#define CONFIG_SYS_HZ_CLOCK 	(AT91C_MASTER_CLOCK / 2)

#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock */

#define CONFIG_CMDLINE_TAG		1
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

#define CONFIG_SYS_USE_MAIN_OSCILLATOR	1
/* flash */
#define CONFIG_SYS_EBI_CFGR_VAL		0x00000000
#define CONFIG_SYS_SMC_CSR0_VAL		0x00003284 /* 16bit, 2 TDF, 4 WS */

/* clocks */
#define CONFIG_SYS_PLLAR_VAL		0x20483E05 /* 179.4048 MHz for PCK */
#define CONFIG_SYS_PLLBR_VAL		0x104C3E0A /* 47.3088 MHz (for USB) */
#define CONFIG_SYS_MCKR_VAL		0x00000202 /* PCK/3 = MCK Clock */

/*
 * Size of malloc() pool
 */

#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)

/*
 * sdram
 */

#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x04000000  /* 64 megs */
#define CONFIG_SYS_INIT_SP_ADDR		0x00204000  /* use internal SRAM */

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + \
					CONFIG_SYS_SDRAM_SIZE - 0x00400000 - \
					CONFIG_SYS_MALLOC_LEN)

#define CONFIG_SYS_PIOC_ASR_VAL		0xFFFF0000 /* PIOC as D16/D31 */
#define CONFIG_SYS_PIOC_BSR_VAL		0x00000000
#define CONFIG_SYS_PIOC_PDR_VAL		0xFFFF0000
#define CONFIG_SYS_EBI_CSA_VAL		0x00000002 /* CS1=SDRAM */
#define CONFIG_SYS_SDRC_CR_VAL		0x2188c159 /* set up the SDRAM */
#define CONFIG_SYS_SDRAM		0x20000000 /* address of the SDRAM */
#define CONFIG_SYS_SDRAM1		0x20000080 /* address of the SDRAM */
#define CONFIG_SYS_SDRAM_VAL		0x00000000 /* value written to SDRAM */
#define CONFIG_SYS_SDRC_MR_VAL		0x00000002 /* Precharge All */
#define CONFIG_SYS_SDRC_MR_VAL1		0x00000004 /* refresh */
#define CONFIG_SYS_SDRC_MR_VAL2		0x00000003 /* Load Mode Register */
#define CONFIG_SYS_SDRC_MR_VAL3		0x00000000 /* Normal Mode */
#define CONFIG_SYS_SDRC_TR_VAL		0x000002E0 /* Write refresh rate */

/*
 * Command line configuration
 */
#define CONFIG_CMD_BMP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_NAND
#define CONFIG_CMD_PING
#define CONFIG_I2C_CMD_TREE
#define CONFIG_CMD_USB
#define CONFIG_CMD_FAT
#define CONFIG_CMD_UBI
#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_UBIFS

#define CONFIG_SYS_LONGHELP

/*
 * MTD defines
 */

#define CONFIG_FLASH_CFI_MTD
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_RBTREE
#define CONFIG_LZO

#define MTDIDS_DEFAULT		"nor0=physmap-flash.0,nand0=atmel_nand"
#define MTDPARTS_DEFAULT	"mtdparts="				\
					"physmap-flash.0:"		\
						"512k(U-Boot),"		\
						"128k(Env),"		\
						"128k(Splash),"		\
						"4M(Kernel),"		\
						"384k(MiniFS),"		\
						"-(FS)"			\
					";"				\
					"atmel_nand:"			\
						"1M(emergency),"	\
						"-(data)"
/*
 * Hardware drivers
 */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_PLLB
#define CONFIG_USB_OHCI_NEW
#define CONFIG_AT91C_PQFP_UHPBUG
#define CONFIG_USB_STORAGE
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION
#define CONFIG_EFI_PARTITION

#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	1
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00300000
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91rm9200"

/*
 * UART/CONSOLE
 */

#define CONFIG_BAUDRATE 115200
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE	ATMEL_BASE_DBGU
#define CONFIG_USART_ID		0/* ignored in arm */

/*
 * network
 */

#define CONFIG_NET_RETRY_COUNT		10
#define CONFIG_RESET_PHY_R		1

#define CONFIG_DRIVER_AT91EMAC		1
#define CONFIG_DRIVER_AT91EMAC_QUIET	1
#define CONFIG_SYS_RX_ETH_BUFFER	8
#define CONFIG_MII			1

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/*
 * I2C-Bus
 */

#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SOFT		/* I2C bit-banged */
#define CONFIG_SYS_I2C_SOFT_SPEED	50000
#define CONFIG_SYS_I2C_SOFT_SLAVE	0

/* Software  I2C driver configuration */

#define AT91_PIN_SDA			(1<<25)		/* AT91C_PIO_PA25 */
#define AT91_PIN_SCL			(1<<26)		/* AT91C_PIO_PA26 */

#define CONFIG_SYS_I2C_INIT_BOARD

#define I2C_INIT	i2c_init_board();
#define I2C_ACTIVE	writel(ATMEL_PMX_AA_TWD, &pio->pioa.mddr);
#define I2C_TRISTATE	writel(ATMEL_PMX_AA_TWD, &pio->pioa.mder);
#define I2C_READ	((readl(&pio->pioa.pdsr) & ATMEL_PMX_AA_TWD) != 0)
#define I2C_SDA(bit)						\
	if (bit)						\
		writel(ATMEL_PMX_AA_TWD, &pio->pioa.sodr);	\
	else							\
		writel(ATMEL_PMX_AA_TWD, &pio->pioa.codr);
#define I2C_SCL(bit)						\
	if (bit)						\
		writel(ATMEL_PMX_AA_TWCK, &pio->pioa.sodr);	\
	else							\
		writel(ATMEL_PMX_AA_TWCK, &pio->pioa.codr);

#define I2C_DELAY	udelay(2500000/CONFIG_SYS_I2C_SOFT_SPEED)

/* I2C-RTC */

#ifdef CONFIG_CMD_DATE
#define CONFIG_RTC_DS1338
#define CONFIG_SYS_I2C_RTC_ADDR	0x68
#endif

/* EEPROM */

#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50

/* FLASH organization */

/*  NOR-FLASH */
#define CONFIG_FLASH_SHOW_PROGRESS	45

#define CONFIG_FLASH_CFI_DRIVER	1

#define PHYS_FLASH_1			0x10000000
#define PHYS_FLASH_SIZE			0x01000000  /* 16 megs main flash */
#define CONFIG_SYS_FLASH_CFI		1
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT

#define CONFIG_SYS_FLASH_PROTECTION	1
#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	512
#define CONFIG_SYS_FLASH_ERASE_TOUT	6000
#define CONFIG_SYS_FLASH_WRITE_TOUT	2000

/* NAND */

#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_DBW_8		1

/* Status LED's */

#define CONFIG_STATUS_LED		1
#define CONFIG_BOARD_SPECIFIC_LED	1

#define STATUS_LED_BOOT			1
#define STATUS_LED_ACTIVE		0

#define STATUS_LED_BIT			1	/* AT91C_PIO_PD0 green LED */
#define STATUS_LED_PERIOD		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_STATE 		STATUS_LED_OFF		/* BLINKING */
#define STATUS_LED_BIT1			2	/* AT91C_PIO_PD1  red LED */
#define STATUS_LED_STATE1		STATUS_LED_ON		/* BLINKING */
#define STATUS_LED_PERIOD1		(CONFIG_SYS_HZ / 4)

#define	CONFIG_VIDEO			1

/* Options */

#ifdef CONFIG_VIDEO

#define CONFIG_VIDEO_VCXK			1

#define CONFIG_SPLASH_SCREEN			1

#define CONFIG_SYS_VCXK_DEFAULT_LINEALIGN	4
#define CONFIG_SYS_VCXK_BASE	0x30000000

#define CONFIG_SYS_VCXK_ACKNOWLEDGE_PIN		(1<<3)
#define CONFIG_SYS_VCXK_ACKNOWLEDGE_PORT	piob
#define CONFIG_SYS_VCXK_ACKNOWLEDGE_DDR		odr

#define CONFIG_SYS_VCXK_ENABLE_PIN		(1<<5)
#define CONFIG_SYS_VCXK_ENABLE_PORT		piob
#define CONFIG_SYS_VCXK_ENABLE_DDR		oer

#define CONFIG_SYS_VCXK_REQUEST_PIN		(1<<2)
#define CONFIG_SYS_VCXK_REQUEST_PORT		piob
#define CONFIG_SYS_VCXK_REQUEST_DDR		oer

#define CONFIG_SYS_VCXK_INVERT_PIN		(1<<4)
#define CONFIG_SYS_VCXK_INVERT_PORT		piob
#define CONFIG_SYS_VCXK_INVERT_DDR		oer

#define CONFIG_SYS_VCXK_RESET_PIN		(1<<6)
#define CONFIG_SYS_VCXK_RESET_PORT		piob
#define CONFIG_SYS_VCXK_RESET_DDR		oer

#endif	/* CONFIG_VIDEO */

/* Environment */

#define CONFIG_BOOTDELAY		5

#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_ADDR			(PHYS_FLASH_1 + 0x80000)
#define CONFIG_ENV_SIZE			0x20000 /* sectors are 128K here */

#define CONFIG_BAUDRATE 		115200

#define CONFIG_BOOTCOMMAND		"run nfsboot"

#define CONFIG_NFSBOOTCOMMAND 						\
		"dhcp $(copy_addr) uImage_cpux9k2;"			\
		"run bootargsdefaults;"					\
		"set bootargs $(bootargs) boot=nfs "			\
		";echo $(bootargs)"					\
	";bootm"

#define CONFIG_EXTRA_ENV_SETTINGS 					\
	"displaywidth=256\0"						\
	"displayheight=512\0"						\
	"displaybsteps=1023\0"						\
	"ubootaddr=10000000\0"						\
	"splashimage=100A0000\0"					\
	"kerneladdr=100C0000\0"						\
	"kernelsize=00400000\0"						\
	"rootfsaddr=10520000\0"						\
	"copy_addr=21200000\0"						\
	"rootfssize=00AE0000\0"						\
	"mtdids=" MTDIDS_DEFAULT "\0"					\
	"mtdparts=" MTDPARTS_DEFAULT "\0"				\
	"bootargsdefaults=set bootargs "				\
		"console=ttyS0,115200 "					\
		"video=vcxk_fb:xres:${displaywidth},"			\
			"yres:${displayheight},"			\
			"bres:${displaybsteps} "			\
		"mem=62M "						\
		"panic=10 "						\
		"uboot=\\\"${ver}\\\" "					\
		"\0"							\
	"update_kernel=protect off $(kerneladdr) +$(kernelsize);"	\
		"dhcp $(copy_addr) uImage_cpux9k2;"			\
		"erase $(kerneladdr) +$(kernelsize);"			\
		"cp.b $(fileaddr) $(kerneladdr) $(filesize);"		\
		"protect on $(kerneladdr) +$(kernelsize)"		\
		"\0"							\
	"update_root=protect off $(rootfsaddr) +$(rootfssize);"		\
		"dhcp $(copy_addr) rfs;"				\
		"erase $(rootfsaddr) +$(rootfssize);"			\
		"cp.b $(fileaddr) $(rootfsaddr) $(filesize);"		\
		"\0"							\
	"update_uboot=protect off 10000000 1007FFFF;"			\
		"dhcp $(copy_addr) u-boot_eb_cpux9k2;"			\
		"erase 10000000 1007FFFF;"				\
		"cp.b $(fileaddr) $(ubootaddr) $(filesize);"		\
		"protect on 10000000 1007FFFF;reset\0"			\
	"update_splash=protect off $(splashimage) +20000;"		\
		"dhcp $(copy_addr) splash_eb_cpux9k2.bmp;"		\
		"erase $(splashimage) +20000;"				\
		"cp.b $(fileaddr) $(splashimage) $(filesize);"		\
		"protect on $(splashimage) +20000;reset\0"		\
	"emergency=run bootargsdefaults;"				\
		"set bootargs $(bootargs) root=initramfs boot=emergency " \
		";bootm $(kerneladdr)\0"				\
	"netemergency=run bootargsdefaults;"				\
		"dhcp $(copy_addr) uImage_cpux9k2;"			\
		"set bootargs $(bootargs) root=initramfs boot=emergency " \
		";bootm $(copy_addr)\0"					\
	"norboot=run bootargsdefaults;"					\
		"set bootargs $(bootargs) root=initramfs boot=local "	\
		";bootm $(kerneladdr)\0"				\
	"nandboot=run bootargsdefaults;"				\
		"set bootargs $(bootargs) root=initramfs boot=nand "	\
		";bootm $(kerneladdr)\0"				\
	" "

/*--------------------------------------------------------------------------*/

#endif

/* EOF */

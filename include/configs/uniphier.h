/*
 * Copyright (C) 2012-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* U-boot - Common settings for UniPhier Family */

#ifndef __CONFIG_UNIPHIER_COMMON_H__
#define __CONFIG_UNIPHIER_COMMON_H__

#if defined(CONFIG_MACH_PH1_PRO4)
#define CONFIG_DDR_NUM_CH0 2
#define CONFIG_DDR_NUM_CH1 2

/* Physical start address of SDRAM */
#define CONFIG_SDRAM0_BASE	0x80000000
#define CONFIG_SDRAM0_SIZE	0x20000000
#define CONFIG_SDRAM1_BASE	0xa0000000
#define CONFIG_SDRAM1_SIZE	0x20000000
#endif

#if defined(CONFIG_MACH_PH1_LD4)
#define CONFIG_DDR_NUM_CH0 1
#define CONFIG_DDR_NUM_CH1 1

/* Physical start address of SDRAM */
#define CONFIG_SDRAM0_BASE	0x80000000
#define CONFIG_SDRAM0_SIZE	0x10000000
#define CONFIG_SDRAM1_BASE	0x90000000
#define CONFIG_SDRAM1_SIZE	0x10000000
#endif

#if defined(CONFIG_MACH_PH1_SLD8)
#define CONFIG_DDR_NUM_CH0 1
#define CONFIG_DDR_NUM_CH1 1

/* Physical start address of SDRAM */
#define CONFIG_SDRAM0_BASE	0x80000000
#define CONFIG_SDRAM0_SIZE	0x10000000
#define CONFIG_SDRAM1_BASE	0x90000000
#define CONFIG_SDRAM1_SIZE	0x10000000
#endif

#define CONFIG_I2C_EEPROM
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS  10

/*
 * Support card address map
 */
#if defined(CONFIG_PFC_MICRO_SUPPORT_CARD)
# define CONFIG_SUPPORT_CARD_BASE	0x03f00000
# define CONFIG_SUPPORT_CARD_ETHER_BASE	(CONFIG_SUPPORT_CARD_BASE + 0x00000000)
# define CONFIG_SUPPORT_CARD_LED_BASE	(CONFIG_SUPPORT_CARD_BASE + 0x00090000)
# define CONFIG_SUPPORT_CARD_UART_BASE	(CONFIG_SUPPORT_CARD_BASE + 0x000b0000)
#endif

#if defined(CONFIG_DCC_MICRO_SUPPORT_CARD)
# define CONFIG_SUPPORT_CARD_BASE	0x08000000
# define CONFIG_SUPPORT_CARD_ETHER_BASE	(CONFIG_SUPPORT_CARD_BASE + 0x00000000)
# define CONFIG_SUPPORT_CARD_LED_BASE	(CONFIG_SUPPORT_CARD_BASE + 0x00401630)
# define CONFIG_SUPPORT_CARD_UART_BASE	(CONFIG_SUPPORT_CARD_BASE + 0x00200000)
#endif

#ifdef CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_COM1		CONFIG_SUPPORT_CARD_UART_BASE
#define CONFIG_SYS_NS16550_CLK		12288000
#define CONFIG_SYS_NS16550_REG_SIZE	-2
#endif

/* TODO: move to Kconfig and device tree */
#if 0
#define CONFIG_SYS_NS16550_SERIAL
#endif

#define CONFIG_SMC911X

#define CONFIG_SMC911X_BASE		CONFIG_SUPPORT_CARD_ETHER_BASE
#define CONFIG_SMC911X_32_BIT

/*-----------------------------------------------------------------------
 * MMU and Cache Setting
 *----------------------------------------------------------------------*/

/* Comment out the following to enable L1 cache */
/* #define CONFIG_SYS_ICACHE_OFF */
/* #define CONFIG_SYS_DCACHE_OFF */

#define CONFIG_SYS_CACHELINE_SIZE	32

/* Comment out the following to enable L2 cache */
#define CONFIG_UNIPHIER_L2CACHE_ON

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_MISC_INIT_F
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_BOARD_LATE_INIT

#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)

#define CONFIG_TIMESTAMP

/* FLASH related */
#define CONFIG_MTD_DEVICE

/*
 * uncomment the following to disable FLASH related code.
 */
/* #define CONFIG_SYS_NO_FLASH */

#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI

#define CONFIG_SYS_MAX_FLASH_SECT	256
#define CONFIG_SYS_MONITOR_BASE		0
#define CONFIG_SYS_FLASH_BASE		0

/*
 * flash_toggle does not work for out supoort card.
 * We need to use flash_status_poll.
 */
#define CONFIG_SYS_CFI_FLASH_STATUS_POLL

#define CONFIG_FLASH_SHOW_PROGRESS	45 /* count down from 45/5: 9..1 */

#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT 2

/* serial console configuration */
#define CONFIG_BAUDRATE			115200

#define CONFIG_SYS_GENERIC_BOARD

#if !defined(CONFIG_SPL_BUILD)
#define CONFIG_USE_ARCH_MEMSET
#define CONFIG_USE_ARCH_MEMCPY
#endif

#define CONFIG_SYS_LONGHELP		/* undef to save memory */

#define CONFIG_CMDLINE_EDITING		/* add command line history	*/
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

#define CONFIG_CONS_INDEX		1

/*
 * For NAND booting the environment is embedded in the U-Boot image. Please take
 * look at the file board/amcc/canyonlands/u-boot-nand.lds for details.
 */
/* #define CONFIG_ENV_IS_IN_NAND */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE				0x2000
#define CONFIG_ENV_OFFSET			0x0
/* #define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE) */

/* Time clock 1MHz */
#define CONFIG_SYS_TIMER_RATE			1000000

/*
 * By default, ARP timeout is 5 sec.
 * The first ARP request does not seem to work.
 * So we need to retry ARP request anyway.
 * We want to shrink the interval until the second ARP request.
 */
#define CONFIG_ARP_TIMEOUT	500UL  /* 0.5 msec */

#define CONFIG_SYS_MAX_NAND_DEVICE			1
#define CONFIG_SYS_NAND_MAX_CHIPS			2
#define CONFIG_SYS_NAND_ONFI_DETECTION

#define CONFIG_NAND_DENALI_ECC_SIZE			1024

#define CONFIG_SYS_NAND_REGS_BASE			0x68100000
#define CONFIG_SYS_NAND_DATA_BASE			0x68000000

#define CONFIG_SYS_NAND_BASE		(CONFIG_SYS_NAND_DATA_BASE + 0x10)

#define CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_NAND_BAD_BLOCK_POS			0

/* USB */
#define CONFIG_USB_MAX_CONTROLLER_COUNT		2
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS	4
#define CONFIG_CMD_FAT
#define CONFIG_FAT_WRITE
#define CONFIG_DOS_PARTITION

/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x01000000)

#define CONFIG_BOOTDELAY			3
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

/*
 * Network Configuration
 */
#define CONFIG_SERVERIP			192.168.11.1
#define CONFIG_IPADDR			192.168.11.10
#define CONFIG_GATEWAYIP		192.168.11.1
#define CONFIG_NETMASK			255.255.255.0

#define CONFIG_LOADADDR			0x84000000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_BOOTFILE			"fit.itb"

#define CONFIG_CMDLINE_EDITING		/* add command line history	*/

#define CONFIG_BOOTCOMMAND		"run $bootmode"

#define CONFIG_ROOTPATH			"/nfs/root/path"
#define CONFIG_NFSBOOTCOMMAND						\
	"setenv bootargs $bootargs root=/dev/nfs rw "			\
	"nfsroot=$serverip:$rootpath "					\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off;" \
	"tftpboot; bootm;"

#define CONFIG_BOOTARGS		" user_debug=0x1f init=/sbin/init"

#define	CONFIG_EXTRA_ENV_SETTINGS		\
	"netdev=eth0\0"				\
	"image_offset=0x00080000\0"		\
	"image_size=0x00f00000\0"		\
	"verify=n\0"				\
	"nandupdate=nand erase 0 0x100000 &&"				\
		   "tftpboot u-boot-spl.bin &&"				\
		   "nand write $loadaddr 0 0x10000 &&"			\
		   "tftpboot u-boot-dtb.img &&"				\
		   "nand write $loadaddr 0x10000 0xf0000\0"		\
	"norboot=run add_default_bootargs &&"				\
		"bootm $image_offset\0"					\
	"nandboot=run add_default_bootargs &&"				\
		 "nand read $loadaddr $image_offset $image_size &&"	\
		 "bootm\0"						\
	"add_default_bootargs=setenv bootargs $bootargs"		\
		" console=ttyS0,$baudrate\0"				\

/* Open Firmware flat tree */
#define CONFIG_OF_LIBFDT

#define CONFIG_HAVE_ARM_SECURE

/* Memory Size & Mapping */
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SDRAM0_BASE

#if CONFIG_SDRAM0_BASE + CONFIG_SDRAM0_SIZE >= CONFIG_SDRAM1_BASE
/* Thre is no memory hole */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_SIZE	(CONFIG_SDRAM0_SIZE + CONFIG_SDRAM1_SIZE)
#else
#define CONFIG_NR_DRAM_BANKS		2
#define CONFIG_SYS_SDRAM_SIZE	(CONFIG_SDRAM0_SIZE)
#endif

#if defined(CONFIG_MACH_PH1_LD4) || defined(CONFIG_MACH_PH1_SLD8)
#define CONFIG_SPL_TEXT_BASE		0x00040000
#endif
#if defined(CONFIG_MACH_PH1_PRO4)
#define CONFIG_SPL_TEXT_BASE		0x00100000
#endif

#define CONFIG_SPL_STACK		(0x0ff08000)
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE)

#define CONFIG_PANIC_HANG

#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_NAND_SUPPORT

#define CONFIG_SPL_LIBCOMMON_SUPPORT	/* for mem_malloc_init */
#define CONFIG_SPL_LIBGENERIC_SUPPORT

#define CONFIG_SPL_BOARD_INIT

#define CONFIG_SYS_NAND_U_BOOT_OFFS		0x10000

#define CONFIG_SPL_MAX_FOOTPRINT		0x10000

#endif /* __CONFIG_UNIPHIER_COMMON_H__ */

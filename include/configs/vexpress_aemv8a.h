/*
 * Configuration for Versatile Express. Parts were derived from other ARM
 *   configurations.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __VEXPRESS_AEMV8A_H
#define __VEXPRESS_AEMV8A_H

/* We use generic board for v8 Versatile Express */
#define CONFIG_SYS_GENERIC_BOARD

#ifdef CONFIG_TARGET_VEXPRESS64_BASE_FVP
#ifndef CONFIG_SEMIHOSTING
#error CONFIG_TARGET_VEXPRESS64_BASE_FVP requires CONFIG_SEMIHOSTING
#endif
#define CONFIG_BOARD_LATE_INIT
#define CONFIG_ARMV8_SWITCH_TO_EL1
#endif

#define CONFIG_REMAKE_ELF

#if !defined(CONFIG_TARGET_VEXPRESS64_BASE_FVP) && \
    !defined(CONFIG_TARGET_VEXPRESS64_JUNO)
/* Base FVP and Juno not using GICv3 yet */
#define CONFIG_GICV3
#endif

/*#define CONFIG_ARMV8_SWITCH_TO_EL1*/

#define CONFIG_SUPPORT_RAW_INITRD

/* Cache Definitions */
#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_SYS_ICACHE_OFF

#define CONFIG_IDENT_STRING		" vexpress_aemv8a"
#define CONFIG_BOOTP_VCI_STRING		"U-boot.armv8.vexpress_aemv8a"

/* Link Definitions */
#ifdef CONFIG_TARGET_VEXPRESS64_BASE_FVP
/* ATF loads u-boot here for BASE_FVP model */
#define CONFIG_SYS_TEXT_BASE		0x88000000
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_SDRAM_BASE + 0x03f00000)
#elif CONFIG_TARGET_VEXPRESS64_JUNO
#define CONFIG_SYS_TEXT_BASE		0xe0000000
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_SDRAM_BASE + 0x7fff0)
#else
#define CONFIG_SYS_TEXT_BASE		0x80000000
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_SDRAM_BASE + 0x7fff0)
#endif

/* Flat Device Tree Definitions */
#define CONFIG_OF_LIBFDT

/* CS register bases for the original memory map. */
#define V2M_PA_CS0			0x00000000
#define V2M_PA_CS1			0x14000000
#define V2M_PA_CS2			0x18000000
#define V2M_PA_CS3			0x1c000000
#define V2M_PA_CS4			0x0c000000
#define V2M_PA_CS5			0x10000000

#define V2M_PERIPH_OFFSET(x)		(x << 16)
#define V2M_SYSREGS			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(1))
#define V2M_SYSCTL			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(2))
#define V2M_SERIAL_BUS_PCI		(V2M_PA_CS3 + V2M_PERIPH_OFFSET(3))

#define V2M_BASE			0x80000000

/* Common peripherals relative to CS7. */
#define V2M_AACI			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(4))
#define V2M_MMCI			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(5))
#define V2M_KMI0			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(6))
#define V2M_KMI1			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(7))

#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
#define V2M_UART0			0x7ff80000
#define V2M_UART1			0x7ff70000
#else /* Not Juno */
#define V2M_UART0			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(9))
#define V2M_UART1			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(10))
#define V2M_UART2			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(11))
#define V2M_UART3			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(12))
#endif

#define V2M_WDT				(V2M_PA_CS3 + V2M_PERIPH_OFFSET(15))

#define V2M_TIMER01			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(17))
#define V2M_TIMER23			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(18))

#define V2M_SERIAL_BUS_DVI		(V2M_PA_CS3 + V2M_PERIPH_OFFSET(22))
#define V2M_RTC				(V2M_PA_CS3 + V2M_PERIPH_OFFSET(23))

#define V2M_CF				(V2M_PA_CS3 + V2M_PERIPH_OFFSET(26))

#define V2M_CLCD			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(31))

/* System register offsets. */
#define V2M_SYS_CFGDATA			(V2M_SYSREGS + 0x0a0)
#define V2M_SYS_CFGCTRL			(V2M_SYSREGS + 0x0a4)
#define V2M_SYS_CFGSTAT			(V2M_SYSREGS + 0x0a8)

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		(0x1800000)	/* 24MHz */

/* Generic Interrupt Controller Definitions */
#ifdef CONFIG_GICV3
#define GICD_BASE			(0x2f000000)
#define GICR_BASE			(0x2f100000)
#else

#ifdef CONFIG_TARGET_VEXPRESS64_BASE_FVP
#define GICD_BASE			(0x2f000000)
#define GICC_BASE			(0x2c000000)
#elif CONFIG_TARGET_VEXPRESS64_JUNO
#define GICD_BASE			(0x2C010000)
#define GICC_BASE			(0x2C02f000)
#else
#define GICD_BASE			(0x2C001000)
#define GICC_BASE			(0x2C002000)
#endif
#endif

#define CONFIG_SYS_MEMTEST_START	V2M_BASE
#define CONFIG_SYS_MEMTEST_END		(V2M_BASE + 0x80000000)

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (8 << 20))

/* Ethernet Configuration */
#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
/* The real hardware Versatile express uses SMSC9118 */
#define CONFIG_SMC911X			1
#define CONFIG_SMC911X_32_BIT		1
#define CONFIG_SMC911X_BASE		(0x018000000)
#else
/* The Vexpress64 simulators use SMSC91C111 */
#define CONFIG_SMC91111			1
#define CONFIG_SMC91111_BASE		(0x01A000000)
#endif

/* PL011 Serial Configuration */
#define CONFIG_PL011_SERIAL
#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
#define CONFIG_PL011_CLOCK		7273800
#else
#define CONFIG_PL011_CLOCK		24000000
#endif
#define CONFIG_PL01x_PORTS		{(void *)CONFIG_SYS_SERIAL0, \
					 (void *)CONFIG_SYS_SERIAL1}
#define CONFIG_CONS_INDEX		0

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_SERIAL0		V2M_UART0
#define CONFIG_SYS_SERIAL1		V2M_UART1

/* Command line configuration */
#define CONFIG_MENU
/*#define CONFIG_MENU_SHOW*/
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_BDI
#define CONFIG_CMD_BOOTI
#define CONFIG_CMD_UNZIP
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PXE
#define CONFIG_CMD_ENV
#define CONFIG_CMD_IMI
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_RUN
#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_SOURCE
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_PXE
#define CONFIG_BOOTP_PXE_CLIENTARCH	0x100

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		(V2M_BASE + 0x10000000)

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			(V2M_BASE)	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE		0x80000000	/* 2048 MB */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* Initial environment variables */
#ifdef CONFIG_TARGET_VEXPRESS64_BASE_FVP
#define CONFIG_EXTRA_ENV_SETTINGS	\
				"kernel_name=uImage\0"	\
				"kernel_addr_r=0x80000000\0"	\
				"initrd_name=ramdisk.img\0"	\
				"initrd_addr_r=0x88000000\0"	\
				"fdt_name=devtree.dtb\0"		\
				"fdt_addr_r=0x83000000\0"		\
				"fdt_high=0xffffffffffffffff\0"	\
				"initrd_high=0xffffffffffffffff\0"

#define CONFIG_BOOTARGS		"console=ttyAMA0 earlyprintk=pl011,"\
				"0x1c090000 debug user_debug=31 "\
				"loglevel=9"

#define CONFIG_BOOTCOMMAND	"fdt addr $fdt_addr_r; fdt resize; " \
				"fdt chosen $initrd_addr_r $initrd_end; " \
				"bootm $kernel_addr_r - $fdt_addr_r"

#define CONFIG_BOOTDELAY		1

#else

#define CONFIG_EXTRA_ENV_SETTINGS	\
					"kernel_addr_r=0x80000000\0"	\
					"initrd_addr_r=0x88000000\0"	\
					"fdt_addr_r=0x83000000\0"		\
					"fdt_high=0xa0000000\0"

#define CONFIG_BOOTARGS			"console=ttyAMA0,115200n8 root=/dev/ram0"
#define CONFIG_BOOTCOMMAND		"bootm $kernel_addr_r " \
					"$initrd_addr_r:$initrd_size $fdt_addr_r"
#define CONFIG_BOOTDELAY		-1
#endif

/* Do not preserve environment */
#define CONFIG_ENV_IS_NOWHERE		1
#define CONFIG_ENV_SIZE			0x1000

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_PROMPT		"VExpress64# "
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_MAXARGS		64	/* max command args */

/* Flash memory is available on the Juno board only */
#ifndef CONFIG_TARGET_VEXPRESS64_JUNO
#define CONFIG_SYS_NO_FLASH
#else
#define CONFIG_CMD_FLASH
#define CONFIG_SYS_FLASH_CFI		1
#define CONFIG_FLASH_CFI_DRIVER		1
#define CONFIG_SYS_FLASH_BASE		0x08000000
#define CONFIG_SYS_FLASH_SIZE		0x04000000 /* 64 MiB */
#define CONFIG_SYS_MAX_FLASH_BANKS	2

/* Timeout values in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(2 * CONFIG_SYS_HZ) /* Erase Timeout */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2 * CONFIG_SYS_HZ) /* Write Timeout */

/* 255 0x40000 sectors + first or last sector may have 4 erase regions = 259 */
#define CONFIG_SYS_MAX_FLASH_SECT	259		/* Max sectors */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE /* use buffered writes */
#define CONFIG_SYS_FLASH_PROTECTION	/* The devices have real protection */
#define CONFIG_SYS_FLASH_EMPTY_INFO	/* flinfo indicates empty blocks */

#endif

#endif /* __VEXPRESS_AEMV8A_H */

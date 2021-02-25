/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Versatile Express. Parts were derived from other ARM
 *   configurations.
 */

#ifndef __VEXPRESS_AEMV8A_H
#define __VEXPRESS_AEMV8A_H

#define CONFIG_REMAKE_ELF

/* Link Definitions */
#ifdef CONFIG_TARGET_VEXPRESS64_BASE_FVP
/* ATF loads u-boot here for BASE_FVP model */
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_SDRAM_BASE + 0x03f00000)
#elif CONFIG_TARGET_VEXPRESS64_JUNO
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_SDRAM_BASE + 0x7fff0)
#endif

#define CONFIG_SYS_BOOTM_LEN (64 << 20)      /* Increase max gunzip size */

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
#define COUNTER_FREQUENCY		24000000	/* 24MHz */

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
#endif
#endif /* !CONFIG_GICV3 */

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (8 << 20))

#ifndef CONFIG_TARGET_VEXPRESS64_JUNO
/* The Vexpress64 simulators use SMSC91C111 */
#define CONFIG_SMC91111			1
#define CONFIG_SMC91111_BASE		(0x01A000000)
#endif

/* PL011 Serial Configuration */
#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
#define CONFIG_PL011_CLOCK		7372800
#else
#define CONFIG_PL011_CLOCK		24000000
#endif

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		(V2M_BASE + 0x10000000)

/* Physical Memory Map */
#define PHYS_SDRAM_1			(V2M_BASE)	/* SDRAM Bank #1 */
/* Top 16MB reserved for secure world use */
#define DRAM_SEC_SIZE		0x01000000
#define PHYS_SDRAM_1_SIZE	0x80000000 - DRAM_SEC_SIZE
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1

#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
#define PHYS_SDRAM_2			(0x880000000)
#define PHYS_SDRAM_2_SIZE		0x180000000
#elif CONFIG_TARGET_VEXPRESS64_BASE_FVP && CONFIG_NR_DRAM_BANKS == 2
#define PHYS_SDRAM_2			(0x880000000)
#define PHYS_SDRAM_2_SIZE		0x80000000
#endif

/* Enable memtest */

/* Initial environment variables */
#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
/*
 * Defines where the kernel and FDT exist in NOR flash and where it will
 * be copied into DRAM
 */
#define CONFIG_EXTRA_ENV_SETTINGS	\
				"kernel_name=norkern\0"	\
				"kernel_alt_name=Image\0"	\
				"kernel_addr_r=0x80080000\0" \
				"ramdisk_name=ramdisk.img\0"	\
				"ramdisk_addr_r=0x88000000\0"	\
				"fdtfile=board.dtb\0" \
				"fdt_alt_name=juno\0" \
				"fdt_addr_r=0x80000000\0" \

#ifndef CONFIG_BOOTCOMMAND
/* Copy the kernel and FDT to DRAM memory and boot */
#define CONFIG_BOOTCOMMAND	"afs load ${kernel_name} ${kernel_addr_r} ;"\
				"if test $? -eq 1; then "\
				"  echo Loading ${kernel_alt_name} instead of "\
				"${kernel_name}; "\
				"  afs load ${kernel_alt_name} ${kernel_addr_r};"\
				"fi ; "\
				"afs load ${fdtfile} ${fdt_addr_r} ;"\
				"if test $? -eq 1; then "\
				"  echo Loading ${fdt_alt_name} instead of "\
				"${fdtfile}; "\
				"  afs load ${fdt_alt_name} ${fdt_addr_r}; "\
				"fi ; "\
				"fdt addr ${fdt_addr_r}; fdt resize; " \
				"if afs load  ${ramdisk_name} ${ramdisk_addr_r} ; "\
				"then "\
				"  setenv ramdisk_param ${ramdisk_addr_r}; "\
				"  else setenv ramdisk_param -; "\
				"fi ; " \
				"booti ${kernel_addr_r} ${ramdisk_param} ${fdt_addr_r}"
#endif


#elif CONFIG_TARGET_VEXPRESS64_BASE_FVP
#define CONFIG_EXTRA_ENV_SETTINGS	\
				"kernel_name=Image\0"		\
				"kernel_addr=0x80080000\0"	\
				"initrd_name=ramdisk.img\0"	\
				"initrd_addr=0x88000000\0"	\
				"fdtfile=devtree.dtb\0"		\
				"fdt_addr=0x83000000\0"		\
				"boot_name=boot.img\0"		\
				"boot_addr=0x8007f800\0"

#ifndef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND	"if smhload ${boot_name} ${boot_addr}; then " \
				"  set bootargs; " \
				"  abootimg addr ${boot_addr}; " \
				"  abootimg get dtb --index=0 fdt_addr; " \
				"  bootm ${boot_addr} ${boot_addr} " \
				"  ${fdt_addr}; " \
				"else; " \
				"  set fdt_high 0xffffffffffffffff; " \
				"  set initrd_high 0xffffffffffffffff; " \
				"  smhload ${kernel_name} ${kernel_addr}; " \
				"  smhload ${fdtfile} ${fdt_addr}; " \
				"  smhload ${initrd_name} ${initrd_addr} "\
				"  initrd_end; " \
				"  fdt addr ${fdt_addr}; fdt resize; " \
				"  fdt chosen ${initrd_addr} ${initrd_end}; " \
				"  booti $kernel_addr - $fdt_addr; " \
				"fi"
#endif
#endif

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_MAXARGS		64	/* max command args */

#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
#define CONFIG_SYS_FLASH_BASE		0x08000000
/* 255 x 256KiB sectors + 4 x 64KiB sectors at the end = 259 */
#define CONFIG_SYS_MAX_FLASH_SECT	259
/* Store environment at top of flash in the same location as blank.img */
/* in the Juno firmware. */
#else
#define CONFIG_SYS_FLASH_BASE		0x0C000000
/* 256 x 256KiB sectors */
#define CONFIG_SYS_MAX_FLASH_SECT	256
/* Store environment at top of flash */
#endif

#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_32BIT
#define CONFIG_SYS_MAX_FLASH_BANKS	1

#ifdef CONFIG_USB_EHCI_HCD
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 1
#endif

#define CONFIG_SYS_FLASH_EMPTY_INFO	/* flinfo indicates empty blocks */
#define FLASH_MAX_SECTOR_SIZE		0x00040000

#endif /* __VEXPRESS_AEMV8A_H */

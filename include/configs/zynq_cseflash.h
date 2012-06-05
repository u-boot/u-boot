/*
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_ARM1176		1 /* CPU */
#define CONFIG_ZYNQ		1 /* SoC? */
#define CONFIG_ZC770		1 /* Board */
#define CONFIG_ZC770_XM010	1 /* Board sub-type ("flavor"?) */

#define CONFIG_ARM_DCC  1 /* enable dcc channel */
#define CONFIG_CPU_V6   1

#include <asm/arch/xparameters.h>

/*
 * Open Firmware flat tree
 */
//#define CONFIG_OF_LIBFDT                1

/* Default environment */
//#define CONFIG_IPADDR   10.10.70.102
//#define CONFIG_ETHADDR  00:0a:35:00:01:22
//#define CONFIG_SERVERIP 10.10.70.101

/*#define CONFIG_EXTRA_ENV_SETTINGS 	\
	"kernel_size=0x140000\0" 	\
	"ramdisk_size=0x200000\0" 	\
	"nand_kernel_size=0x400000\0" 	\
	"nand_ramdisk_size=0x400000\0" 	\
	"bootcmd=run modeboot\0"	\
	"norboot=echo Copying Linux from NOR flash to RAM...; \
			    cp 0xE2100000 0x8000 ${kernel_size}; \
			    cp 0xE2600000 0x1000000 0x8000; \
			    echo Copying ramdisk...; \
			    cp 0xE3000000 0x800000 ${ramdisk_size}; \
			    go 0x8000\0" \
	"qspiboot=echo Copying Linux from QSPI flash to RAM...; \
			    cp 0xFC100000 0x8000 ${kernel_size}; \
			    cp 0xFC600000 0x1000000 0x8000; \
			    echo Copying ramdisk...; \
			    cp 0xFC800000 0x800000 ${ramdisk_size};\
			    go 0x8000\0" \
	"sdboot=echo Copying Linux from SD to RAM...; \
			mmcinfo; \
			fatload mmc 0 0x8000 zImage; \
			fatload mmc 0 0x1000000 devicetree.dtb; \
			fatload mmc 0 0x800000 ramdisk8M.image.gz; \
			    go 0x8000\0" \
	"nandboot=echo Copying Linux from NAND flash to RAM...;	\
			    nand read 0x8000 0x200000 ${nand_kernel_size}; \
			    nand read 0x1000000 0x700000 0x20000; \
			    echo Copying ramdisk...; \
			    nand read 0x800000 0x900000 ${nand_ramdisk_size}; \
			    go 0x8000\0"
*/

#undef CONFIG_ZYNQ_XIL_LQSPI

/* default boot is according to the bootmode switch settings */
#define CONFIG_BOOTCOMMAND "run modeboot"

#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE { 9600, 38400, 115200 }
#define CONFIG_BOOTDELAY	-1 /* -1 to Disable autoboot */

#define	CONFIG_PSS_SERIAL
#define	CONFIG_RTC_XPSSRTC

#include <config_cmd_default.h>
#undef CONFIG_CMD_DATE		/* RTC? */
#undef CONFIG_CMD_PING		/* Might be useful for debugging */
#undef CONFIG_REGINFO		/* Again, debugging */
#undef CONFIG_CMD_SETGETDCR	/* README says 4xx only */
#undef CONFIG_CMD_EDITENV
#undef CONFIG_CMD_SAVEENV

#undef CONFIG_CMD_NET		/* bootp, tftpboot, rarpboot	*/
#undef CONFIG_CMD_NFS		/* NFS support			*/
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_XIMG

#undef CONFIG_CMD_LOADB	/* loadb			*/
#undef CONFIG_CMD_LOADS	/* loads			*/

#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */

#undef CONFIG_CMD_MISC		/* Misc functions like sleep etc*/
#undef CONFIG_CMD_RUN		/* run command in env variable	*/
#undef CONFIG_CMD_SOURCE	/* "source" command support	*/

#undef CONFIG_CMD_BDI		/* bdinfo			*/
#undef CONFIG_CMD_BOOTD	/* bootd			*/
#undef CONFIG_CMD_CONSOLE	/* coninfo			*/
#undef CONFIG_CMD_ECHO		/* echo arguments		*/
#undef CONFIG_CMD_IMI		/* iminfo			*/
#undef CONFIG_CMD_ITEST	/* Integer (and string) test	*/
#undef CONFIG_CMD_IMLS		/* List all found images	*/

#undef CONFIG_BOOTM_LINUX
#undef CONFIG_BOOTM_NETBSD
#undef CONFIG_BOOTM_RTEMS
#undef CONFIG_GZIP
#undef CONFIG_ZLIB

//#define CONFIG_TIMESTAMP	/* print image timestamp on bootm, etc */

/* IPADDR, SERVERIP */
/* Need I2C for RTC? */

#define CONFIG_PANIC_HANG	1 /* For development/debugging */

//#define CONFIG_AUTO_COMPLETE	1
#define CONFIG_CMDLINE_EDITING	1

#define CONFIG_SYS_PROMPT	"zynq-uboot> "

#define CONFIG_SKIP_RELOCATE_UBOOT

/* Uncomment it if you don't want Flash */
//#define CONFIG_SYS_NO_FLASH

#define CONFIG_SYS_SDRAM_BASE	0

#define CONFIG_L2_OFF

/* this is to initialize GEM at uboot start */
/* #define CONFIG_ZYNQ_INIT_GEM	*/
/* this is to set ipaddr, ethaddr and serverip env variables. */
/* #define CONFIG_ZYNQ_IP_ENV	*/


#ifndef CONFIG_SYS_NO_FLASH

/* FLASH organization */
#define CONFIG_SYS_FLASH_BASE           0xE2000000
#define CONFIG_SYS_FLASH_SIZE           (16*1024*1024)  /* i.e. 16MB */
#define CONFIG_SYS_MAX_FLASH_BANKS      1       /* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT       512     /* max number of sectors/blocks on one chip */
#define CONFIG_SYS_FLASH_ERASE_TOUT     1000
#define CONFIG_SYS_FLASH_WRITE_TOUT     5000

#define CONFIG_FLASH_SHOW_PROGRESS	10

#define CONFIG_SYS_FLASH_CFI            1
// #define CONFIG_SYS_FLASH_EMPTY_INFO     0
#define CONFIG_FLASH_CFI_DRIVER 	1

#define CONFIG_SYS_FLASH_PROTECTION     0       /* use hardware protection           */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE       /* use buffered writes (20x faster)  */
//#define CONFIG_ENV_ADDR         	(CONFIG_SYS_FLASH_BASE + 0x00000000)
#define CONFIG_ENV_OFFSET		0xC0000		/*768 KB*/
#define CONFIG_ENV_SECT_SIZE    	0x20000		/*128 KB*/
#ifdef CONFIG_EP107
# define CONFIG_ENV_IS_IN_FLASH		1
#else
# define CONFIG_ENV_IS_NOWHERE		1
#endif
#else

#define CONFIG_ENV_IS_NOWHERE	1

#endif

/* CONFIG_SYS_MONITOR_BASE? */
/* CONFIG_SYS_MONITOR_LEN? */

#define CONFIG_SYS_CACHELINE_SIZE	32 /* Assuming bytes? */

/* CONFIG_SYS_INIT_RAM_ADDR? */
/* CONFIG_SYS_GLOBAL_DATA_OFFSET? */

/* Because (at least at first) we're going to be loaded via JTAG_Tcl */
#define CONFIG_SKIP_LOWLEVEL_INIT


/* HW to use */
#ifdef CONFIG_EP107
# define CONFIG_UART0	1
#else
# define CONFIG_UART1	1
#endif
#define CONFIG_TTC0	1
//#define CONFIG_GEM0	1

#define TIMER_INPUT_CLOCK               XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ / 2
#define CONFIG_TIMER_PRESCALE           255
#define TIMER_TICK_HZ                   (TIMER_INPUT_CLOCK / CONFIG_TIMER_PRESCALE)
#define CONFIG_SYS_HZ                   1000

/* And here... */
#define CONFIG_SYS_LOAD_ADDR	0 /* default? */
/* Semi-educated guess based on p.48 of DF Arch spec */
#define PHYS_SDRAM_1		(256 * 1024)
#define PHYS_SDRAM_1_SIZE	(256 * 1024 * 1024) /* Cameron guessed 256 or 512 MB */

#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x10000)

/*
 * These were lifted straight from imx31_phycore, and may well be very wrong.
 */
//#define CONFIG_ENV_SIZE			4096
#define CONFIG_ENV_SIZE			    896
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_MALLOC_LEN		0x4000 // 16KB
#define CONFIG_SYS_GBL_DATA_SIZE	128
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* phycore */

/*
 * SPI Settings
 */
#define CONFIG_CMD_SPI
#define CONFIG_ENV_SPI_MAX_HZ   30000000
#define CONFIG_SF_DEFAULT_SPEED 30000000
#define CONFIG_SPI_FLASH
#define CONFIG_CMD_SF
/* #define CONFIG_XILINX_PSS_QSPI_USE_DUAL_FLASH */
//#ifdef NOTOW_BHILL
//#define CONFIG_SPI_FLASH_ATMEL
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SPI_FLASH_WINBOND
//#endif
#define CONFIG_SPI_FLASH_STMICRO

/*
 * NAND Flash settings
 */
#if defined(CONFIG_ZC770_XM011) || defined(CONFIG_ZC770_XM010_XM011)
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NAND_LOCK_UNLOCK
#define CONFIG_SYS_MAX_NAND_DEVICE 1
#define CONFIG_SYS_NAND_BASE XPSS_NAND_BASEADDR
#define CONFIG_MTD_DEVICE
#endif

/* Place a Xilinx Boot ROM header in u-boot image? */
#define CONFIG_ZYNQ_XILINX_FLASH_HEADER

#ifdef CONFIG_ZYNQ_XILINX_FLASH_HEADER
/* Address Xilinx boot rom should use to launch u-boot */
#ifdef CONFIG_ZYNQ_XIL_LQSPI
#define CONFIG_ZYNQ_XIP_START XPSS_QSPI_LIN_BASEADDR
#else
/* NOR */
#define CONFIG_ZYNQ_XIP_START CONFIG_SYS_FLASH_BASE
#endif
#endif

/* Secure Digital */
//#define CONFIG_MMC     1

#ifdef CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_DOS_PARTITION
#endif

#define BOARD_LATE_INIT	1

#endif /* __CONFIG_H */

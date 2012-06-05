/*
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_CSEFLASH /* Board */

#include <configs/zynq_common.h>

/* Default environment */
#define CONFIG_IPADDR   10.10.70.102
#define CONFIG_SERVERIP 10.10.70.101

#define CONFIG_ARM_DCC
#define CONFIG_CPU_V6 /* Required by CONFIG_ARM_DCC */

/*
 * Open Firmware flat tree
 */
#undef CONFIG_OF_LIBFDT

#undef CONFIG_ZYNQ_XIL_LQSPI
#undef CONFIG_PSS_SERIAL
#undef CONFIG_RTC_XPSSRTC

#undef CONFIG_SYS_NO_FLASH
#define CONFIG_ENV_IS_NOWHERE

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

#define CONFIG_PANIC_HANG /* For development/debugging */

//#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING

#define CONFIG_SKIP_RELOCATE_UBOOT

/* this is to initialize GEM at uboot start */
/* #define CONFIG_ZYNQ_INIT_GEM	*/
/* this is to set ipaddr, ethaddr and serverip env variables. */
/* #define CONFIG_ZYNQ_IP_ENV	*/


#ifndef CONFIG_SYS_NO_FLASH

/* FLASH organization */
#define CONFIG_SYS_FLASH_BASE           0xE2000000
#define CONFIG_SYS_FLASH_SIZE           (16 * 1024 * 1024)
#define CONFIG_SYS_MAX_FLASH_BANKS      1
/* max number of sectors/blocks on one chip */
#define CONFIG_SYS_MAX_FLASH_SECT       512
#define CONFIG_SYS_FLASH_ERASE_TOUT     1000
#define CONFIG_SYS_FLASH_WRITE_TOUT     5000

#define CONFIG_FLASH_SHOW_PROGRESS	10

#define CONFIG_SYS_FLASH_CFI
#undef CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_FLASH_CFI_DRIVER

#undef CONFIG_SYS_FLASH_PROTECTION /* don't use hardware protection */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE /* use buffered writes (20x faster) */
#endif

/* Because (at least at first) we're going to be loaded via JTAG_Tcl */
#define CONFIG_SKIP_LOWLEVEL_INIT

/* HW to use */
#define CONFIG_UART1
#define CONFIG_TTC0
//#define CONFIG_GEM0

/*
 * Physical Memory map
 */
#define PHYS_SDRAM_1_SIZE (256 * 1024 * 1024)

/* Why? */
#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE 896

/*
 * SPI Settings
 */
#define CONFIG_CMD_SPI
#define CONFIG_SF_DEFAULT_SPEED 30000000
#define CONFIG_SPI_FLASH
#define CONFIG_CMD_SF
/* #define CONFIG_XILINX_PSS_QSPI_USE_DUAL_FLASH */
//#define CONFIG_SPI_FLASH_ATMEL
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_SPI_FLASH_STMICRO

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

#endif /* __CONFIG_H */

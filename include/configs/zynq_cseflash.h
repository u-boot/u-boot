/*
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define PHYS_SDRAM_1_SIZE (256 * 1024 * 1024)

#define CONFIG_ENV_IS_NOWHERE

#include <configs/zynq_common.h>

/* Disable uart console */
#undef CONFIG_SERIAL_MULTI
#undef CONFIG_ZYNQ_SERIAL


#define CONFIG_ARM_DCC
#define CONFIG_CPU_V6 /* Required by CONFIG_ARM_DCC */

/*
 * Open Firmware flat tree
 */
#undef CONFIG_OF_LIBFDT




#undef CONFIG_CMD_EDITENV
#undef CONFIG_CMD_SAVEENV

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


// FIXME this is silly - there is no any bootm image enabled - disable BOOTM
//#undef CONFIG_BOOTM_LINUX
#undef CONFIG_BOOTM_NETBSD
#undef CONFIG_BOOTM_RTEMS
#undef CONFIG_GZIP
#undef CONFIG_ZLIB

#undef CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING
#undef CONFIG_SYS_LONGHELP


/* Because (at least at first) we're going to be loaded via JTAG_Tcl */
#define CONFIG_SKIP_LOWLEVEL_INIT



/* Why? */
#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE 896

/* Place a Xilinx Boot ROM header in u-boot image? */
#define CONFIG_ZYNQ_XILINX_FLASH_HEADER

#ifdef CONFIG_ZYNQ_XILINX_FLASH_HEADER
/* NOR */
#define CONFIG_ZYNQ_XIP_START CONFIG_SYS_FLASH_BASE
#endif

#endif /* __CONFIG_H */

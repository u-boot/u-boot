/*
 * Hacked together,
 * hopefully functional.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_ARM1176		1 /* CPU */
#define CONFIG_XDF		1 /* Board */
#define CONFIG_DFE		1 /* Board sub-type ("flavor"?) */
#define CONFIG_PELE		1 /* SoC? */

#include "../board/xilinx/dfe/xparameters.h"

/* Default environment */
#define CONFIG_IPADDR   10.10.70.102
#define CONFIG_ETHADDR  00:0a:35:00:01:22
#define CONFIG_SERVERIP 10.10.70.101
#define CONFIG_BOOTCOMMAND "tftp 0x800000 ramdisk3M.image; tftp 0x8000 vmlinux.bin; go 0x8000"

#define CONFIG_BAUDRATE		9600
#define CONFIG_SYS_BAUDRATE_TABLE { 9600, 38400, 115200 }
#define CONFIG_BOOTDELAY	-1 /* Disable autoboot */

#define	CONFIG_PSS_SERIAL
#define	CONFIG_RTC_XPSSRTC

#include <config_cmd_default.h>	/* FIXME: check this! */
#define CONFIG_CMD_DATE		/* RTC? */
#define CONFIG_CMD_PING		/* Might be useful for debugging */
#define CONFIG_CMD_SAVEENV	/* Command to save ENV to Flash */
#define CONFIG_REGINFO		/* Again, debugging */
#undef CONFIG_CMD_SETGETDCR	/* README says 4xx only */

#define CONFIG_TIMESTAMP	/* print image timestamp on bootm, etc */

/* IPADDR, SERVERIP */
/* Need I2C for RTC? */

#define CONFIG_IDENT_STRING	"\nXilinx Pele Emulaton Platform"
#define CONFIG_PANIC_HANG	1 /* For development/debugging */

#define CONFIG_AUTO_COMPLETE	1
#define CONFIG_CMDLINE_EDITING	1

#define CONFIG_SYS_PROMPT	"pele-boot> "

/* CONFIG_SYS_TFTP_LOADADDR? */

#define CONFIG_SYS_RAM_BOOT_ADDR	0x04000000
#define CONFIG_SYS_FLASH_BOOT_ADDR	0xE2010000

#undef CONFIG_SKIP_RELOCATE_UBOOT	

/* Uncomment it if you don't want Flash */
//#define CONFIG_SYS_NO_FLASH	

#define CONFIG_SYS_SDRAM_BASE	0

#define CONFIG_L2_OFF
#define PELE_TTC
#define DRAGONFIRE_TTC0_BASE 0x90001000
#define DRAGONFIRE_TTC1_BASE 0x90002000

//#define CONFIG_PELE_INIT_GEM	//this is to initialize GEM at uboot start
#define CONFIG_PELE_IP_ENV	//this is to set ipaddr, ethaddr and serverip env variables.
#define CONFIG_PELE_FLASH_RELOCATE

#ifndef CONFIG_SYS_NO_FLASH

/* FLASH organization */
#define CONFIG_SYS_FLASH_BASE           0xE2000000 
#define CONFIG_SYS_FLASH_SIZE           (16*1024*1024)  /* i.e. 16MB */
#define CONFIG_SYS_MAX_FLASH_BANKS      1       /* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT       256      /* max number of sectors/blocks on one chip */
#define CONFIG_SYS_FLASH_ERASE_TOUT     1000

#define CONFIG_SYS_FLASH_PROTECTION     1       /* use hardware protection           */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1     /* use buffered writes (20x faster)  */
#define CONFIG_ENV_ADDR         	(CONFIG_SYS_FLASH_BASE + 0x00000000)
#define CONFIG_ENV_SECT_SIZE    	0x10000		/*64 KB*/
#define CONFIG_ENV_IS_NOWHERE	1
//#define CONFIG_ENV_IS_IN_FLASH		1

#else

#define CONFIG_ENV_IS_NOWHERE	1

#endif

/*
#define CONFIG_SYS_FLASH_BASE 		0xE2000000
#define CONFIG_SYS_FLASH_SIZE           (32*1024*1024)
#define CONFIG_SYS_MAX_FLASH_SECT	16
#define CONFIG_SYS_FLASH_CFI            1
#define CONFIG_SYS_FLASH_EMPTY_INFO     1
#define CONFIG_SYS_MAX_FLASH_BANKS      2
#define CONFIG_SYS_FLASH_PROTECTION	1
*/
//#define CONFIG_FLASH_CFI_DRIVER 1
/* CONFIG_SYS_MONITOR_BASE? */
/* CONFIG_SYS_MONITOR_LEN? */

#define CONFIG_SYS_CACHELINE_SIZE	32 /* Assuming bytes? */

/* CONFIG_SYS_INIT_RAM_ADDR? */
/* CONFIG_SYS_GLOBAL_DATA_OFFSET? */

/* Because (at least at first) we're going to be loaded via JTAG_Tcl */
#define CONFIG_SKIP_LOWLEVEL_INIT	


/* HW to use */
#define CONFIG_XDF_UART	1
#define CONFIG_XDF_ETH	1
#define CONFIG_XDF_RTC	1
#define CONFIG_UART0	1
#define CONFIG_TTC0	1
#define CONFIG_GEM0	1

/* XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ */
#define CONFIG_SYS_HZ	12500000 

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
#define CONFIG_ENV_SIZE			0x10000
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_MALLOC_LEN		0x21000	
#define CONFIG_SYS_GBL_DATA_SIZE	128
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* phycore */

#endif /* __CONFIG_H */

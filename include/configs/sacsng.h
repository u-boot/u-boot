/*
 * (C) Copyright 2000
 * Murray Jensen <Murray.Jensen@cmst.csiro.au>
 *
 * (C) Copyright 2000
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2001
 * Advent Networks, Inc. <http://www.adventnetworks.com>
 * Jay Monkman <jtm@smoothsmoothie.com>
 *
 * Configuration settings for the SACSng 8260 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define	CONFIG_SYS_TEXT_BASE	0x40000000

#undef DEBUG_BOOTP_EXT	      /* Debug received vendor fields */

#undef CONFIG_LOGBUFFER       /* External logbuffer support */

/*****************************************************************************
 *
 * These settings must match the way _your_ board is set up
 *
 *****************************************************************************/

/* What is the oscillator's (UX2) frequency in Hz? */
#define CONFIG_8260_CLKIN  66666600

/*-----------------------------------------------------------------------
 * MODCK_H & MODCLK[1-3] - Ref: Section 9.2 in MPC8206 User Manual
 *-----------------------------------------------------------------------
 * What should MODCK_H be? It is dependent on the oscillator
 * frequency, MODCK[1-3], and desired CPM and core frequencies.
 * Here are some example values (all frequencies are in MHz):
 *
 * MODCK_H   MODCK[1-3]	 Osc	CPM    Core  S2-6   S2-7   S2-8
 * -------   ----------	 ---	---    ----  -----  -----  -----
 * 0x1	     0x5	 33	100    133   Open   Close  Open
 * 0x1	     0x6	 33	100    166   Open   Open   Close
 * 0x1	     0x7	 33	100    200   Open   Open   Open
 *
 * 0x2	     0x2	 33	133    133   Close  Open   Close
 * 0x2	     0x3	 33	133    166   Close  Open   Open
 * 0x2	     0x4	 33	133    200   Open   Close  Close
 * 0x2	     0x5	 33	133    233   Open   Close  Open
 * 0x2	     0x6	 33	133    266   Open   Open   Close
 *
 * 0x5	     0x5	 66	133    133   Open   Close  Open
 * 0x5	     0x6	 66	133    166   Open   Open   Close
 * 0x5	     0x7	 66	133    200   Open   Open   Open
 * 0x6	     0x0	 66	133    233   Close  Close  Close
 * 0x6	     0x1	 66	133    266   Close  Close  Open
 * 0x6	     0x2	 66	133    300   Close  Open   Close
 */
#define CONFIG_SYS_SBC_MODCK_H 0x05

/* Define this if you want to boot from 0x00000100. If you don't define
 * this, you will need to program the bootloader to 0xfff00000, and
 * get the hardware reset config words at 0xfe000000. The simplest
 * way to do that is to program the bootloader at both addresses.
 * It is suggested that you just let U-Boot live at 0x00000000.
 */
#define CONFIG_SYS_SBC_BOOT_LOW 1

/* What should the base address of the main FLASH be and how big is
 * it (in MBytes)?  This must contain CONFIG_SYS_TEXT_BASE from board/sacsng/config.mk
 * The main FLASH is whichever is connected to *CS0.
 */
#define CONFIG_SYS_FLASH0_BASE 0x40000000
#define CONFIG_SYS_FLASH0_SIZE 2

/* What should the base address of the secondary FLASH be and how big
 * is it (in Mbytes)?  The secondary FLASH is whichever is connected
 * to *CS6.
 */
#define CONFIG_SYS_FLASH1_BASE 0x60000000
#define CONFIG_SYS_FLASH1_SIZE 2

/* Define CONFIG_VERY_BIG_RAM to allow use of SDRAMs larger than 256MBytes
 */
#define CONFIG_VERY_BIG_RAM	1

/* What should be the base address of SDRAM DIMM and how big is
 * it (in Mbytes)?  This will normally auto-configure via the SPD.
*/
#define CONFIG_SYS_SDRAM0_BASE 0x00000000
#define CONFIG_SYS_SDRAM0_SIZE 64

/*
 * Memory map example with 64 MB DIMM:
 *
 *     0x0000 0000     Exception Vector code, 8k
 *	     :
 *     0x0000 1FFF
 *     0x0000 2000     Free for Application Use
 *	     :
 *	     :
 *
 *	     :
 *	     :
 *     0x03F5 FF30     Monitor Stack (Growing downward)
 *		       Monitor Stack Buffer (0x80)
 *     0x03F5 FFB0     Board Info Data
 *     0x03F6 0000     Malloc Arena
 *	     :		    CONFIG_ENV_SECT_SIZE, 16k
 *	     :		    CONFIG_SYS_MALLOC_LEN,    128k
 *     0x03FC 0000     RAM Copy of Monitor Code
 *	     :		    CONFIG_SYS_MONITOR_LEN,   256k
 *     0x03FF FFFF     [End of RAM], CONFIG_SYS_SDRAM_SIZE - 1
 */

#define CONFIG_POST		(CONFIG_SYS_POST_MEMORY | \
				 CONFIG_SYS_POST_CPU)


/*
 * select serial console configuration
 *
 * if either CONFIG_CONS_ON_SMC or CONFIG_CONS_ON_SCC is selected, then
 * CONFIG_CONS_INDEX must be set to the channel number (1-2 for SMC, 1-4
 * for SCC).
 *
 * if CONFIG_CONS_NONE is defined, then the serial console routines must
 * defined elsewhere.
 */
#define CONFIG_CONS_ON_SMC	1	/* define if console on SMC */
#undef	CONFIG_CONS_ON_SCC		/* define if console on SCC */
#undef	CONFIG_CONS_NONE		/* define if console on neither */
#define CONFIG_CONS_INDEX	1	/* which SMC/SCC channel for console */

/*
 * select ethernet configuration
 *
 * if either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected, then
 * CONFIG_ETHER_INDEX must be set to the channel number (1-4 for SCC, 1-3
 * for FCC)
 *
 * if CONFIG_ETHER_NONE is defined, then either the ethernet routines must be
 * defined elsewhere (as for the console), or CONFIG_CMD_NET must be unset.
 */

#undef	CONFIG_ETHER_ON_SCC
#define CONFIG_ETHER_ON_FCC
#undef	CONFIG_ETHER_NONE		/* define if ethernet on neither */

#ifdef	CONFIG_ETHER_ON_SCC
#define CONFIG_ETHER_INDEX	1	/* which SCC/FCC channel for ethernet */
#endif	/* CONFIG_ETHER_ON_SCC */

#ifdef	CONFIG_ETHER_ON_FCC
#define CONFIG_ETHER_INDEX	2	/* which SCC/FCC channel for ethernet */
#undef  CONFIG_ETHER_LOOPBACK_TEST      /* Ethernet external loopback test */
#define CONFIG_MII			/* MII PHY management		*/
#define CONFIG_BITBANGMII		/* bit-bang MII PHY management	*/
/*
 * Port pins used for bit-banged MII communictions (if applicable).
 */

#define MDIO_PORT	2	        /* Port A=0, B=1, C=2, D=3 */
#define MDIO_DECLARE	volatile ioport_t *iop = ioport_addr ( \
				(immap_t *) CONFIG_SYS_IMMR, MDIO_PORT )
#define MDC_DECLARE	MDIO_DECLARE

#define MDIO_ACTIVE	(iop->pdir |=  0x40000000)
#define MDIO_TRISTATE	(iop->pdir &= ~0x40000000)
#define MDIO_READ	((iop->pdat &  0x40000000) != 0)

#define MDIO(bit)	if(bit) iop->pdat |=  0x40000000; \
			else	iop->pdat &= ~0x40000000

#define MDC(bit)	if(bit) iop->pdat |=  0x80000000; \
			else	iop->pdat &= ~0x80000000

#define MIIDELAY	udelay(50)
#endif	/* CONFIG_ETHER_ON_FCC */

#if defined(CONFIG_ETHER_ON_SCC) && (CONFIG_ETHER_INDEX == 1)

/*
 *  - RX clk is CLK11
 *  - TX clk is CLK12
 */
# define CONFIG_SYS_CMXSCR_VALUE1	(CMXSCR_RS1CS_CLK11  | CMXSCR_TS1CS_CLK12)

#elif defined(CONFIG_ETHER_ON_FCC) && (CONFIG_ETHER_INDEX == 2)

/*
 * - Rx-CLK is CLK13
 * - Tx-CLK is CLK14
 * - Select bus for bd/buffers (see 28-13)
 * - Enable Full Duplex in FSMR
 */
# define CONFIG_SYS_CMXFCR_MASK2	(CMXFCR_FC2|CMXFCR_RF2CS_MSK|CMXFCR_TF2CS_MSK)
# define CONFIG_SYS_CMXFCR_VALUE2	(CMXFCR_RF2CS_CLK13|CMXFCR_TF2CS_CLK14)
# define CONFIG_SYS_CPMFCR_RAMTYPE	0
# define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)

#endif /* CONFIG_ETHER_ON_FCC, CONFIG_ETHER_INDEX */

#define CONFIG_SHOW_BOOT_PROGRESS 1	/* boot progress enabled	*/

/*
 * Configure for RAM tests.
 */
#undef  CONFIG_SYS_DRAM_TEST			/* calls other tests in board.c	*/


/*
 * Status LED for power up status feedback.
 */
#define CONFIG_STATUS_LED	1	/* Status LED enabled		*/

#define STATUS_LED_PAR		im_ioport.iop_ppara
#define STATUS_LED_DIR		im_ioport.iop_pdira
#define STATUS_LED_ODR		im_ioport.iop_podra
#define STATUS_LED_DAT		im_ioport.iop_pdata

#define STATUS_LED_BIT		0x00000800	/* LED 0 is on PA.20	*/
#define STATUS_LED_PERIOD	(CONFIG_SYS_HZ)
#define STATUS_LED_STATE	STATUS_LED_OFF
#define STATUS_LED_BIT1		0x00001000	/* LED 1 is on PA.19	*/
#define STATUS_LED_PERIOD1	(CONFIG_SYS_HZ)
#define STATUS_LED_STATE1	STATUS_LED_OFF
#define STATUS_LED_BIT2		0x00002000	/* LED 2 is on PA.18	*/
#define STATUS_LED_PERIOD2	(CONFIG_SYS_HZ/2)
#define STATUS_LED_STATE2	STATUS_LED_ON

#define STATUS_LED_ACTIVE	0		/* LED on for bit == 0	*/

#define STATUS_LED_YELLOW	0
#define STATUS_LED_GREEN	1
#define STATUS_LED_RED		2
#define STATUS_LED_BOOT		1


/*
 * Select SPI support configuration
 */
#define CONFIG_SOFT_SPI		/* Enable SPI driver */
#define MAX_SPI_BYTES   4	/* Maximum number of bytes we can handle */
#undef  DEBUG_SPI               /* Disable SPI debugging */

/*
 * Software (bit-bang) SPI driver configuration
 */
#ifdef CONFIG_SOFT_SPI

/*
 * Software (bit-bang) SPI driver configuration
 */
#define I2C_SCLK	0x00002000      /* PD 18: Shift clock */
#define I2C_MOSI	0x00004000      /* PD 17: Master Out, Slave In */
#define I2C_MISO	0x00008000      /* PD 16: Master In, Slave Out */

#undef  SPI_INIT			/* no port initialization needed */
#define SPI_READ        ((immr->im_ioport.iop_pdatd & I2C_MISO) != 0)
#define SPI_SDA(bit)    do {						\
			if(bit) immr->im_ioport.iop_pdatd |=  I2C_MOSI; \
			else    immr->im_ioport.iop_pdatd &= ~I2C_MOSI;	\
			} while (0)
#define SPI_SCL(bit)    do {						\
			if(bit) immr->im_ioport.iop_pdatd |=  I2C_SCLK; \
			else    immr->im_ioport.iop_pdatd &= ~I2C_SCLK;	\
			} while (0)
#define SPI_DELAY                       /* No delay is needed */
#endif /* CONFIG_SOFT_SPI */


/*
 * select I2C support configuration
 *
 * Supported configurations are {none, software, hardware} drivers.
 * If the software driver is chosen, there are some additional
 * configuration items that the driver uses to drive the port pins.
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SOFT		/* I2C bit-banged */
#define CONFIG_SYS_I2C_SOFT_SPEED	400000
#define CONFIG_SYS_I2C_SOFT_SLAVE	0x7F
/*
 * Software (bit-bang) I2C driver configuration
 */
#define I2C_PORT	3		/* Port A=0, B=1, C=2, D=3 */
#define I2C_ACTIVE	(iop->pdir |=  0x00010000)
#define I2C_TRISTATE	(iop->pdir &= ~0x00010000)
#define I2C_READ	((iop->pdat & 0x00010000) != 0)
#define I2C_SDA(bit)	if(bit) iop->pdat |=  0x00010000; \
			else    iop->pdat &= ~0x00010000
#define I2C_SCL(bit)	if(bit) iop->pdat |=  0x00020000; \
			else    iop->pdat &= ~0x00020000
#define I2C_DELAY	udelay(20)	/* 1/4 I2C clock duration */

/* Define this to reserve an entire FLASH sector for
 * environment variables. Otherwise, the environment will be
 * put in the same sector as U-Boot, and changing variables
 * will erase U-Boot temporarily
 */
#define CONFIG_ENV_IN_OWN_SECT	1

/* Define this to contain any number of null terminated strings that
 * will be part of the default environment compiled into the boot image.
 */
#define CONFIG_EXTRA_ENV_SETTINGS \
"quiet=0\0" \
"serverip=192.168.123.205\0" \
"ipaddr=192.168.123.203\0" \
"checkhostname=VR8500\0" \
"reprog="\
    "bootp; " \
    "tftpboot 0x140000 /bdi2000/u-boot.bin; " \
    "protect off 60000000 6003FFFF; " \
    "erase 60000000 6003FFFF; " \
    "cp.b 140000 60000000 ${filesize}; " \
    "protect on 60000000 6003FFFF\0" \
"copyenv="\
    "protect off 60040000 6004FFFF; " \
    "erase 60040000 6004FFFF; " \
    "cp.b 40040000 60040000 10000; " \
    "protect on 60040000 6004FFFF\0" \
"copyprog="\
    "protect off 60000000 6003FFFF; " \
    "erase 60000000 6003FFFF; " \
    "cp.b 40000000 60000000 40000; " \
    "protect on 60000000 6003FFFF\0" \
"zapenv="\
    "protect off 40040000 4004FFFF; " \
    "erase 40040000 4004FFFF; " \
    "protect on 40040000 4004FFFF\0" \
"zapotherenv="\
    "protect off 60040000 6004FFFF; " \
    "erase 60040000 6004FFFF; " \
    "protect on 60040000 6004FFFF\0" \
"root-on-initrd="\
    "setenv bootcmd "\
    "version\\;" \
    "echo\\;" \
    "bootp\\;" \
    "setenv bootargs root=/dev/ram0 rw quiet " \
    "ip=\\${ipaddr}:\\${serverip}:\\${gatewayip}:\\${netmask}:\\${hostname}::off\\;" \
    "run boot-hook\\;" \
    "bootm\0" \
"root-on-initrd-debug="\
    "setenv bootcmd "\
    "version\\;" \
    "echo\\;" \
    "bootp\\;" \
    "setenv bootargs root=/dev/ram0 rw debug " \
    "ip=\\${ipaddr}:\\${serverip}:\\${gatewayip}:\\${netmask}:\\${hostname}::off\\;" \
    "run debug-hook\\;" \
    "run boot-hook\\;" \
    "bootm\0" \
"root-on-nfs="\
    "setenv bootcmd "\
    "version\\;" \
    "echo\\;" \
    "bootp\\;" \
    "setenv bootargs root=/dev/nfs rw quiet " \
    "nfsroot=\\${serverip}:\\${rootpath} " \
    "ip=\\${ipaddr}:\\${serverip}:\\${gatewayip}:\\${netmask}:\\${hostname}::off\\;" \
    "run boot-hook\\;" \
    "bootm\0" \
"root-on-nfs-debug="\
    "setenv bootcmd "\
    "version\\;" \
    "echo\\;" \
    "bootp\\;" \
    "setenv bootargs root=/dev/nfs rw debug " \
    "nfsroot=\\${serverip}:\\${rootpath} " \
    "ip=\\${ipaddr}:\\${serverip}:\\${gatewayip}:\\${netmask}:\\${hostname}::off\\;" \
    "run debug-hook\\;" \
    "run boot-hook\\;" \
    "bootm\0" \
"debug-checkout="\
    "setenv checkhostname;" \
    "setenv ethaddr 00:09:70:00:00:01;" \
    "bootp;" \
    "setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} debug " \
    "ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;" \
    "run debug-hook;" \
    "run boot-hook;" \
    "bootm\0" \
"debug-hook="\
    "echo ipaddr    ${ipaddr};" \
    "echo serverip  ${serverip};" \
    "echo gatewayip ${gatewayip};" \
    "echo netmask   ${netmask};" \
    "echo hostname  ${hostname}\0" \
"ana=run adc ; run dac\0" \
"adc=run adc-12 ; run adc-34\0" \
"adc-12=echo ### ADC-12 ; i2c md e 81 e\0" \
"adc-34=echo ### ADC-34 ; i2c md f 81 e\0" \
"dac=echo ### DAC ; i2c md 11 81 5\0" \
"boot-hook=echo\0"

/* What should the console's baud rate be? */
#define CONFIG_BAUDRATE		9600

/* Ethernet MAC address */
#define CONFIG_ETHADDR		00:09:70:00:00:00

/* The default Ethernet MAC address can be overwritten just once  */
#ifdef  CONFIG_ETHADDR
#define CONFIG_OVERWRITE_ETHADDR_ONCE 1
#endif

/*
 * Define this to do some miscellaneous board-specific initialization.
 */
#define CONFIG_MISC_INIT_R

/* Set to a positive value to delay for running BOOTCOMMAND */
#define CONFIG_BOOTDELAY	1	/* autoboot after 1 second */

/* Be selective on what keys can delay or stop the autoboot process
 *     To stop	use: " "
 */
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT	"Autobooting...\n"
#define CONFIG_AUTOBOOT_STOP_STR	" "
#undef  CONFIG_AUTOBOOT_DELAY_STR
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define DEBUG_BOOTKEYS		0

/* Define a command string that is automatically executed when no character
 * is read on the console interface withing "Boot Delay" after reset.
 */
#undef	CONFIG_BOOT_ROOT_INITRD		/* Use ram disk for the root file system */
#define	CONFIG_BOOT_ROOT_NFS		/* Use a NFS mounted root file system */

#ifdef CONFIG_BOOT_ROOT_INITRD
#define CONFIG_BOOTCOMMAND \
	"version;" \
	"echo;" \
	"bootp;" \
	"setenv bootargs root=/dev/ram0 rw quiet " \
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;" \
	"run boot-hook;" \
	"bootm"
#endif /* CONFIG_BOOT_ROOT_INITRD */

#ifdef CONFIG_BOOT_ROOT_NFS
#define CONFIG_BOOTCOMMAND \
	"version;" \
	"echo;" \
	"bootp;" \
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} quiet " \
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;" \
	"run boot-hook;" \
	"bootm"
#endif /* CONFIG_BOOT_ROOT_NFS */

#define CONFIG_BOOTP_RANDOM_DELAY       /* Randomize the BOOTP retry delay */
#define CONFIG_LIB_RAND

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE
#define  CONFIG_BOOTP_DNS
#define  CONFIG_BOOTP_DNS2
#define  CONFIG_BOOTP_SEND_HOSTNAME


/* undef this to save memory */
#define CONFIG_SYS_LONGHELP

/* Monitor Command Prompt */

#undef  CONFIG_SYS_HUSH_PARSER
#ifdef  CONFIG_SYS_HUSH_PARSER
#endif

/* When CONFIG_TIMESTAMP is selected, the timestamp (date and time)
 * of an image is printed by image commands like bootm or iminfo.
 */
#define CONFIG_TIMESTAMP

/* If this variable is defined, an environment variable named "ver"
 * is created by U-Boot showing the U-Boot version.
 */
#define CONFIG_VERSION_VARIABLE


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ELF
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_I2C
#define CONFIG_CMD_SPI
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_PING

#undef CONFIG_CMD_KGDB

#ifdef CONFIG_ETHER_ON_FCC
#define CONFIG_CMD_MII
#endif


/* Where do the internal registers live? */
#define CONFIG_SYS_IMMR		0xF0000000

#undef	CONFIG_WATCHDOG			/* disable the watchdog */

/*****************************************************************************
 *
 * You should not have to modify any of the following settings
 *
 *****************************************************************************/

#define CONFIG_SACSng		1	/* munged for the SACSng */
#define CONFIG_CPM2		1	/* Has a CPM2 */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_BOOTM_HEADER_QUIET 1        /* Suppress the image header dump    */
					/* in the bootm command.             */
#define CONFIG_SYS_BOOTM_PROGESS_QUIET 1       /* Suppress the progress displays,   */
					/* "## <message>" from the bootm cmd */
#define CONFIG_SYS_BOOTP_CHECK_HOSTNAME 1      /* If checkhostname environment is   */
					/* defined, then the hostname param  */
					/* validated against checkhostname.  */
#define CONFIG_SYS_BOOTP_RETRY_COUNT 0x40000000 /* # of timeouts before giving up   */
#define CONFIG_SYS_BOOTP_SHORT_RANDOM_DELAY 1  /* Use a short random delay value    */
					/* (limited to maximum of 1024 msec) */
#define CONFIG_SYS_CHK_FOR_ABORT_AT_LEAST_ONCE 1
					/* Check for abort key presses       */
					/* at least once in dependent of the */
					/* CONFIG_BOOTDELAY value.           */
#define CONFIG_SYS_CONSOLE_INFO_QUIET 1        /* Don't print console @ startup     */
#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN 1      /* Echo the inverted Ethernet link   */
					/* state to the fault LED.           */
#define CONFIG_SYS_FAULT_MII_ADDR 0x02         /* MII addr of the PHY to check for  */
					/* the Ethernet link state.          */
#define CONFIG_SYS_STATUS_FLASH_UNTIL_TFTP_OK 1 /* Keeping the status LED flashing  */
					/* until the TFTP is successful.     */
#define CONFIG_SYS_STATUS_OFF_AFTER_NETBOOT 1  /* After a successful netboot,       */
					/* turn off the STATUS LEDs.         */
#define CONFIG_SYS_TFTP_BLINK_STATUS_ON_DATA_IN 1 /* Blink status LED based on      */
					/* incoming data.                    */
#define CONFIG_SYS_TFTP_BLOCKS_PER_HASH 100    /* For every XX blocks, output a '#' */
					/* to signify that tftp is moving.   */
#define CONFIG_SYS_TFTP_HASHES_PER_FLASH 200   /* For every '#' hashes,             */
					/* flash the status LED.             */
#define CONFIG_SYS_TFTP_HASHES_PER_LINE 65     /* Only output XX '#'s per line      */
					/* during the tftp file transfer.    */
#define CONFIG_SYS_TFTP_PROGESS_QUIET 1        /* Suppress the progress displays    */
					/* '#'s from the tftp command.       */
#define CONFIG_SYS_TFTP_STATUS_QUIET 1         /* Suppress the status displays      */
					/* issued during the tftp command.   */
#define CONFIG_SYS_TFTP_TIMEOUT_COUNT 5        /* How many timeouts TFTP will allow */
					/* before it gives up.               */

#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size	     */
#else
#  define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size	     */
#endif

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	  (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT)+16)

#define CONFIG_SYS_MAXARGS		32	/* max number of command args	*/

#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size	   */

#define CONFIG_SYS_LOAD_ADDR		0x400000   /* default load address */

#define CONFIG_SYS_ALT_MEMTEST                 /* Select full-featured memory test */
#define CONFIG_SYS_MEMTEST_START	0x2000	/* memtest works from the end of */
					/* the exception vector table */
					/* to the end of the DRAM  */
					/* less monitor and malloc area */
#define CONFIG_SYS_STACK_USAGE		0x10000 /* Reserve 64k for the stack usage */
#define CONFIG_SYS_MEM_END_USAGE	( CONFIG_SYS_MONITOR_LEN \
				+ CONFIG_SYS_MALLOC_LEN \
				+ CONFIG_ENV_SECT_SIZE \
				+ CONFIG_SYS_STACK_USAGE )

#define CONFIG_SYS_MEMTEST_END		( CONFIG_SYS_SDRAM_SIZE * 1024 * 1024 \
				- CONFIG_SYS_MEM_END_USAGE )

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CONFIG_SYS_FLASH_BASE	CONFIG_SYS_FLASH0_BASE
#define CONFIG_SYS_FLASH_SIZE	CONFIG_SYS_FLASH0_SIZE
#define CONFIG_SYS_SDRAM_BASE	CONFIG_SYS_SDRAM0_BASE
#define CONFIG_SYS_SDRAM_SIZE	CONFIG_SYS_SDRAM0_SIZE

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 */
#if defined(CONFIG_SYS_SBC_BOOT_LOW)
#  define  CONFIG_SYS_SBC_HRCW_BOOT_FLAGS  (HRCW_CIP | HRCW_BMS)
#else
#  define  CONFIG_SYS_SBC_HRCW_BOOT_FLAGS  (0)
#endif /* defined(CONFIG_SYS_SBC_BOOT_LOW) */

/* get the HRCW ISB field from CONFIG_SYS_IMMR */
#define CONFIG_SYS_SBC_HRCW_IMMR	( ((CONFIG_SYS_IMMR & 0x10000000) >> 10) | \
				  ((CONFIG_SYS_IMMR & 0x01000000) >>  7) | \
				  ((CONFIG_SYS_IMMR & 0x00100000) >>  4) )

#define CONFIG_SYS_HRCW_MASTER		( HRCW_BPS10				| \
				  HRCW_DPPC11				| \
				  CONFIG_SYS_SBC_HRCW_IMMR			| \
				  HRCW_MMR00				| \
				  HRCW_LBPC11				| \
				  HRCW_APPC10				| \
				  HRCW_CS10PC00				| \
				  (CONFIG_SYS_SBC_MODCK_H & HRCW_MODCK_H1111)	| \
				  CONFIG_SYS_SBC_HRCW_BOOT_FLAGS )

/* no slaves */
#define CONFIG_SYS_HRCW_SLAVE1		0
#define CONFIG_SYS_HRCW_SLAVE2		0
#define CONFIG_SYS_HRCW_SLAVE3		0
#define CONFIG_SYS_HRCW_SLAVE4		0
#define CONFIG_SYS_HRCW_SLAVE5		0
#define CONFIG_SYS_HRCW_SLAVE6		0
#define CONFIG_SYS_HRCW_SLAVE7		0

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000	/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 * Note also that the logic that sets CONFIG_SYS_RAMBOOT is platform dependent.
 */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH0_BASE

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#  define CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#define CONFIG_SYS_FLASH_CFI		1	/* Flash is CFI conformant		*/
#undef  CONFIG_SYS_FLASH_PROTECTION		/* use hardware protection		*/
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	(64+4)	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	8000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	1	/* Timeout for Flash Write (in ms)	*/

#ifndef CONFIG_SYS_RAMBOOT
#  define CONFIG_ENV_IS_IN_FLASH	1

#  ifdef CONFIG_ENV_IN_OWN_SECT
#    define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#    define CONFIG_ENV_SECT_SIZE	0x10000
#  else
#    define CONFIG_ENV_ADDR (CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN - CONFIG_ENV_SECT_SIZE)
#    define CONFIG_ENV_SIZE	0x1000	/* Total Size of Environment Sector	*/
#    define CONFIG_ENV_SECT_SIZE	0x10000 /* see README - env sect real size	*/
#  endif /* CONFIG_ENV_IN_OWN_SECT */

#else
#  define CONFIG_ENV_IS_IN_NVRAM	1
#  define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
#  define CONFIG_ENV_SIZE		0x200
#endif /* CONFIG_SYS_RAMBOOT */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC8260 CPU */

#if defined(CONFIG_CMD_KGDB)
# define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * HIDx - Hardware Implementation-dependent Registers			 2-11
 *-----------------------------------------------------------------------
 * HID0 also contains cache control - initially enable both caches and
 * invalidate contents, then the final state leaves only the instruction
 * cache enabled. Note that Power-On and Hard reset invalidate the caches,
 * but Soft reset does not.
 *
 * HID1 has only read-only information - nothing to set.
 */
#define CONFIG_SYS_HID0_INIT	(HID0_ICE  |\
			 HID0_DCE  |\
			 HID0_ICFI |\
			 HID0_DCI  |\
			 HID0_IFEM |\
			 HID0_ABE)

#define CONFIG_SYS_HID0_FINAL	(HID0_ICE  |\
			 HID0_IFEM |\
			 HID0_ABE  |\
			 HID0_EMCP)
#define CONFIG_SYS_HID2	0

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RMR		0

/*-----------------------------------------------------------------------
 * BCR - Bus Configuration					 4-25
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_BCR		(BCR_ETM)

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				 4-31
 *-----------------------------------------------------------------------
 */

#define CONFIG_SYS_SIUMCR	(SIUMCR_DPPC11	|\
			 SIUMCR_L2CPC00 |\
			 SIUMCR_APPC10	|\
			 SIUMCR_MMR00)


/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Watchdog & Bus Monitor Timer max, 60x Bus Monitor enable
 */
#if defined(CONFIG_WATCHDOG)
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC |\
			 SYPCR_BMT  |\
			 SYPCR_PBME |\
			 SYPCR_LBME |\
			 SYPCR_SWRI |\
			 SYPCR_SWP  |\
			 SYPCR_SWE)
#else
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC |\
			 SYPCR_BMT  |\
			 SYPCR_PBME |\
			 SYPCR_LBME |\
			 SYPCR_SWRI |\
			 SYPCR_SWP)
#endif /* CONFIG_WATCHDOG */

/*-----------------------------------------------------------------------
 * TMCNTSC - Time Counter Status and Control			 4-40
 *-----------------------------------------------------------------------
 * Clear once per Second and Alarm Interrupt Status, Set 32KHz timersclk,
 * and enable Time Counter
 */
#define CONFIG_SYS_TMCNTSC	(TMCNTSC_SEC |\
			 TMCNTSC_ALR |\
			 TMCNTSC_TCF |\
			 TMCNTSC_TCE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		 4-42
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Set 32KHz timersclk, and enable
 * Periodic timer
 */
#define CONFIG_SYS_PISCR	(PISCR_PS  |\
			 PISCR_PTF |\
			 PISCR_PTE)

/*-----------------------------------------------------------------------
 * SCCR - System Clock Control					 9-8
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_SCCR	0

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration				13-7
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RCCR	0

/*
 * Initialize Memory Controller:
 *
 * Bank Bus	Machine PortSz	Device
 * ---- ---	------- ------	------
 *  0	60x	GPCM	16 bit	FLASH (primary flash - 2MB)
 *  1	60x	GPCM	-- bit	(Unused)
 *  2	60x	SDRAM	64 bit	SDRAM (DIMM)
 *  3	60x	SDRAM	64 bit	SDRAM (DIMM)
 *  4	60x	GPCM	-- bit	(Unused)
 *  5	60x	GPCM	-- bit	(Unused)
 *  6	60x	GPCM	16 bit	FLASH  (secondary flash - 2MB)
 */

/*-----------------------------------------------------------------------
 * BR0,BR1 - Base Register
 *     Ref: Section 10.3.1 on page 10-14
 * OR0,OR1 - Option Register
 *     Ref: Section 10.3.2 on page 10-18
 *-----------------------------------------------------------------------
 */

/* Bank 0 - Primary FLASH
 */

/* BR0 is configured as follows:
 *
 *     - Base address of 0x40000000
 *     - 16 bit port size
 *     - Data errors checking is disabled
 *     - Read and write access
 *     - GPCM 60x bus
 *     - Access are handled by the memory controller according to MSEL
 *     - Not used for atomic operations
 *     - No data pipelining is done
 *     - Valid
 */
#define CONFIG_SYS_BR0_PRELIM	((CONFIG_SYS_FLASH0_BASE & BRx_BA_MSK) |\
			 BRx_PS_16			|\
			 BRx_MS_GPCM_P			|\
			 BRx_V)

/* OR0 is configured as follows:
 *
 *     - 4 MB
 *     - *BCTL0 is asserted upon access to the current memory bank
 *     - *CW / *WE are negated a quarter of a clock earlier
 *     - *CS is output at the same time as the address lines
 *     - Uses a clock cycle length of 5
 *     - *PSDVAL is generated internally by the memory controller
 *	 unless *GTA is asserted earlier externally.
 *     - Relaxed timing is generated by the GPCM for accesses
 *	 initiated to this memory region.
 *     - One idle clock is inserted between a read access from the
 *	 current bank and the next access.
 */
#define CONFIG_SYS_OR0_PRELIM	(MEG_TO_AM(CONFIG_SYS_FLASH0_SIZE)	|\
			 ORxG_CSNT			|\
			 ORxG_ACS_DIV1			|\
			 ORxG_SCY_5_CLK			|\
			 ORxG_TRLX			|\
			 ORxG_EHTR)

/*-----------------------------------------------------------------------
 * BR2,BR3 - Base Register
 *     Ref: Section 10.3.1 on page 10-14
 * OR2,OR3 - Option Register
 *     Ref: Section 10.3.2 on page 10-16
 *-----------------------------------------------------------------------
 */

/* Bank 2,3 - SDRAM DIMM
 */

/* The BR2 is configured as follows:
 *
 *     - Base address of 0x00000000
 *     - 64 bit port size (60x bus only)
 *     - Data errors checking is disabled
 *     - Read and write access
 *     - SDRAM 60x bus
 *     - Access are handled by the memory controller according to MSEL
 *     - Not used for atomic operations
 *     - No data pipelining is done
 *     - Valid
 */
#define CONFIG_SYS_BR2_PRELIM	((CONFIG_SYS_SDRAM0_BASE & BRx_BA_MSK) |\
			 BRx_PS_64			|\
			 BRx_MS_SDRAM_P			|\
			 BRx_V)

#define CONFIG_SYS_BR3_PRELIM	((CONFIG_SYS_SDRAM0_BASE & BRx_BA_MSK) |\
			 BRx_PS_64			|\
			 BRx_MS_SDRAM_P			|\
			 BRx_V)

/* With a 64 MB DIMM, the OR2 is configured as follows:
 *
 *     - 64 MB
 *     - 4 internal banks per device
 *     - Row start address bit is A8 with PSDMR[PBI] = 0
 *     - 12 row address lines
 *     - Back-to-back page mode
 *     - Internal bank interleaving within save device enabled
 */
#if (CONFIG_SYS_SDRAM0_SIZE == 64)
#define CONFIG_SYS_OR2_PRELIM	(MEG_TO_AM(CONFIG_SYS_SDRAM0_SIZE)	|\
			 ORxS_BPD_4			|\
			 ORxS_ROWST_PBI0_A8		|\
			 ORxS_NUMR_12)
#else
#error "INVALID SDRAM CONFIGURATION"
#endif

/*-----------------------------------------------------------------------
 * PSDMR - 60x Bus SDRAM Mode Register
 *     Ref: Section 10.3.3 on page 10-21
 *-----------------------------------------------------------------------
 */

/* Address that the DIMM SPD memory lives at.
 */
#define SDRAM_SPD_ADDR 0x50

#if (CONFIG_SYS_SDRAM0_SIZE == 64)
/* With a 64 MB DIMM, the PSDMR is configured as follows:
 *
 *     - Bank Based Interleaving,
 *     - Refresh Enable,
 *     - Address Multiplexing where A5 is output on A14 pin
 *	 (A6 on A15, and so on),
 *     - use address pins A14-A16 as bank select,
 *     - A9 is output on SDA10 during an ACTIVATE command,
 *     - earliest timing for ACTIVATE command after REFRESH command is 7 clocks,
 *     - earliest timing for ACTIVATE or REFRESH command after PRECHARGE command
 *	 is 3 clocks,
 *     - earliest timing for READ/WRITE command after ACTIVATE command is
 *	 2 clocks,
 *     - earliest timing for PRECHARGE after last data was read is 1 clock,
 *     - earliest timing for PRECHARGE after last data was written is 1 clock,
 *     - CAS Latency is 2.
 */
#define CONFIG_SYS_PSDMR	(PSDMR_RFEN	      |\
			 PSDMR_SDAM_A14_IS_A5 |\
			 PSDMR_BSMA_A14_A16   |\
			 PSDMR_SDA10_PBI0_A9  |\
			 PSDMR_RFRC_7_CLK     |\
			 PSDMR_PRETOACT_3W    |\
			 PSDMR_ACTTORW_2W     |\
			 PSDMR_LDOTOPRE_1C    |\
			 PSDMR_WRC_1C	      |\
			 PSDMR_CL_2)
#else
#error "INVALID SDRAM CONFIGURATION"
#endif

/*
 * Shoot for approximately 1MHz on the prescaler.
 */
#if (CONFIG_8260_CLKIN >= (60 * 1000 * 1000))
#define CONFIG_SYS_MPTPR	MPTPR_PTP_DIV64
#elif (CONFIG_8260_CLKIN >= (30 * 1000 * 1000))
#define CONFIG_SYS_MPTPR	MPTPR_PTP_DIV32
#else
#warning "Unconfigured bus clock freq: check CONFIG_SYS_MPTPR and CONFIG_SYS_PSRT are OK"
#define CONFIG_SYS_MPTPR	MPTPR_PTP_DIV32
#endif
#define CONFIG_SYS_PSRT	14


/*-----------------------------------------------------------------------
 * BR6 - Base Register
 *     Ref: Section 10.3.1 on page 10-14
 * OR6 - Option Register
 *     Ref: Section 10.3.2 on page 10-18
 *-----------------------------------------------------------------------
 */

/* Bank 6 - Secondary FLASH
 *
 * The secondary FLASH is connected to *CS6
 */
#if (defined(CONFIG_SYS_FLASH1_BASE) && defined(CONFIG_SYS_FLASH1_SIZE))

/* BR6 is configured as follows:
 *
 *     - Base address of 0x60000000
 *     - 16 bit port size
 *     - Data errors checking is disabled
 *     - Read and write access
 *     - GPCM 60x bus
 *     - Access are handled by the memory controller according to MSEL
 *     - Not used for atomic operations
 *     - No data pipelining is done
 *     - Valid
 */
#  define CONFIG_SYS_BR6_PRELIM  ((CONFIG_SYS_FLASH1_BASE & BRx_BA_MSK) |\
			   BRx_PS_16			  |\
			   BRx_MS_GPCM_P		  |\
			   BRx_V)

/* OR6 is configured as follows:
 *
 *     - 2 MB
 *     - *BCTL0 is asserted upon access to the current memory bank
 *     - *CW / *WE are negated a quarter of a clock earlier
 *     - *CS is output at the same time as the address lines
 *     - Uses a clock cycle length of 5
 *     - *PSDVAL is generated internally by the memory controller
 *	 unless *GTA is asserted earlier externally.
 *     - Relaxed timing is generated by the GPCM for accesses
 *	 initiated to this memory region.
 *     - One idle clock is inserted between a read access from the
 *	 current bank and the next access.
 */
#  define CONFIG_SYS_OR6_PRELIM  (MEG_TO_AM(CONFIG_SYS_FLASH1_SIZE)  |\
			   ORxG_CSNT		       |\
			   ORxG_ACS_DIV1	       |\
			   ORxG_SCY_5_CLK	       |\
			   ORxG_TRLX		       |\
			   ORxG_EHTR)
#endif /* (defined(CONFIG_SYS_FLASH1_BASE) && defined(CONFIG_SYS_FLASH1_SIZE)) */

#endif	/* __CONFIG_H */

/******************************************************************************
* A collection of structures, addresses, and values associated with
* the Motorola 850T AdderIIF board.  Copied from the FADS stuff.
* Magnus Damm added defines for 8xxrom and extended bd_info.
* Helmut Buchsbaum added bitvalues for BCSRx
*
* Copyright (c) 1998 Dan Malek (dmalek@jlc.net)
*******************************************************************************
* 2003-JUL: The AdderII is using the following physical memorymap:
*******************************************************************************
* FA200000 -> FA20FFFF : IMAP   internal in the cpu
* FE000000 -> FE400000 : flash  connected to CS0, setup by 8xxrom
* 00000000 -> 00800000 : sdram  setup by 8xxrom
*******************************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

#include <mpc8xx_irq.h>

#define	CONFIG_MPC860		1
#define	CONFIG_MPC860T		1
#define CONFIG_ADDERII		1

/* CPU Clock speed */
#define MPC8XX_FACT		12		/* Multilpy by 12 */
#define MPC8XX_XIN		4000000		/* 4MHz */
#define MPC8XX_HZ		( MPC8XX_FACT * MPC8XX_XIN )

#define CONFIG_8xx_GCLK_FREQ	MPC8XX_HZ
#define CONFIG_SDRAM_50MHZ      1


/* Default Serial Console, baudrate */
#define CONFIG_8xx_CONS_SMC1	1		/* Console is on SMC1 */
#define CONFIG_BAUDRATE		38400
#define CONFIG_LOADS_ECHO	1

/* FEC Ethernet controller configurations */
#define CONFIG_FEC_ETH		1
#define CONFIG_NET_MULTI	1
#define FEC_ENET		1

/* Interrupt level assignments.
*/
#define FEC_INTERRUPT   	SIU_LEVEL3      /* FEC interrupt */

/* Older kernels need clock in MHz newer in Hz */
#define CONFIG_CLOCKS_IN_MHZ	1

/* Monitor Functions */
#define CONFIG_COMMANDS		( CFG_CMD_ENV	| \
				  CFG_CMD_FLASH | \
				  CFG_CMD_MEMORY| \
				  CFG_CMD_NET   | \
				  CFG_CMD_PING  | \
				  CFG_CMD_SDRAM )

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/* Configuration Settings */
#define CFG_PROMPT		"=>"		/* Monitor Command Prompt */

#if ( CONFIG_COMMANDS & CFG_CMD_KGDB )
#define CFG_CBSIZE		1024		/* Console I/P buffer size */
#else
#define CFG_CBSIZE		256
#endif

#define CFG_PBSIZE		( CFG_CBSIZE + sizeof( CFG_PROMPT ) + 16 )
						/* Print buffer size */

#define CFG_MAXARGS		16		/* Max number of cmd args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot args buffer size */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CFG_MEMTEST_START	0x00100000	/* Mem test works on  */
#define CFG_MEMTEST_END		0x00800000	/* 1 ... 8MB in SDRAM */

#define CFG_LOAD_ADDR		0x00100000
#define CFG_HZ          	1000

/******************************************************************************
** Low level configuration settings.
** ( adderss mappings, register init values, etc. )
** You should know what you are doing if you make changes here.
******************************************************************************/
/* Start address  for the final memory configuration set up by startup code
** Please note that CFG_SDRAM_BASE must start at 0
*/

#define CFG_SDRAM_BASE		0x00000000

#define CFG_FLASH_BASE		0xFE000000
#define CFG_FLASH_SIZE		(( uint ) ( 4 * 1024 * 1024 ))	/* 4MB */
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MONITOR_LEN		( 256 << 10 )	/* 256 KByte */
#define CFG_MALLOC_LEN		( 384 << 10 )	/* 384 KByte SDRAM rsvd */
						/* malloc() usage 	*/
/**
** For booting Linux, the board info and command line data
** have to be in the first 8 MB of memory, since this is
** the maximum mapped by the Linux kernel during initialization.
**/
#define CFG_BOOTMAPSZ		( 8 << 20 )    /* Initial Memory map for Linux */

/******************************************************************************
** Flash Organization
******************************************************************************/

#define CFG_MAX_FLASH_BANKS	1		/* Max no of flash mem banks */
#define CFG_MAX_FLASH_SECT	71		/* Max no of sec on 1 chip   */

#define CFG_FLASH_ERASE_TOUT	120000		/* Erase flash timeout (ms)  */
#define CFG_FLASH_WRITE_TOUT	500		/* Write flash timeout (ms)  */

/******************************************************************************
**	U-BOOT Environment variables in Flash
******************************************************************************/
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_OFFSET		0x00040000
#define CFG_ENV_SIZE		0x10000		/* 64KBytes env space */
#define CFG_ENV_SECT_SIZE	0x10000

/******************************************************************************
**	Cache Configuration
******************************************************************************/
#define CFG_CACHELINE_SIZE      16      /* For all MPC8xx CPUs        */
#if ( CONFIG_COMMANDS & CFG_CMD_KGDB )
#define CFG_CACHELINE_SHIFT     4       /* log base 2 of the above value */
#endif

/******************************************************************************
** Internal memory mapped register
******************************************************************************/
#define CFG_IMMR		0xFA200000
#define CFG_IMMR_SIZE		(( uint) ( 62 * 1024 ))	/* 64 KByte res */

/* Definitions for initial stack pointer and data area ( in DPRAM ) */

#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x2F00		/* end of used area in DPRAM */
#define CFG_GBL_DATA_SIZE	64
#define CFG_GBL_DATA_OFFSET	( CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE )
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET


/* SIU Module Configuration Register */
#define CFG_SIUMCR		( SIUMCR_AEME | SIUMCR_MLRC01 | SIUMCR_DBGC10 )

/******************************************************************************
**	SYPCR - System protection and control
**	SYPCR - can be written only once after reset
******************************************************************************/
#if defined( CONFIG_WATCHDOG )
#define CFG_SYPCR		( SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | \
				  SYPCR_SWF | SYPCR_SWE | SYPCR_SWRI | \
				  SYPCR_SWP )
#else
#define CFG_SYPCR		( SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | \
				  SYPCR_SWF | SYPCR_SWP )
#endif

/* TBSCR - Time Base Status and Control Register */
#define CFG_TBSCR		( TBSCR_REFA | TBSCR_REFB | TBSCR_TBE )

/* PISCR - Periodic Interrupt Status and Control */
#define CFG_PISCR       	( PISCR_PS | PISCR_PITF )

/* PLPRCR - PLL, Low-Power, and Reset Control Register */
#define CFG_PLPRCR      	((( MPC8XX_FACT - 1 ) << PLPRCR_MF_SHIFT ) | \
				PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST )

/* SCCR - System Clock and reset Control Register */
#define SCCR_MASK       	SCCR_EBDF11
#define CFG_SCCR		( SCCR_TBS | SCCR_COM00 | SCCR_DFSYNC00 | \
				  SCCR_DFBRG00 | SCCR_DFNL000| SCCR_DFNH000 | \
				  SCCR_DFLCD000 | SCCR_DFALCD00 )
#define CFG_DER         	0

/******************************************************************************
** Because of the way the 860 starts up and assigns CS0 the
** entire address space, we have to set the memory controller
** differently.  Normally, you write the option register
** first, and then enable the chip select by writing the
** base register.  For CS0, you must write the base register
** first, followed by the option register.
******************************************************************************/
/**
 ** Memory Controller Definitions
 ** BR0/1/2... and OR0/1/2...
*/
/* For AdderII BR0 FLASH */

#define CFG_REMAP_OR_AM		0xFF800000		/* OR addr mask */
#define CFG_PRELIM_OR_AM	0xFF800000		/* OR addr mask */

/* Flash Timings: ACS = 11, TRLX = 1, CSNT = 0, SCY = 7 */
#define CFG_OR_TIMING_FLASH 	( OR_ACS_DIV2 | OR_BI | OR_SCY_7_CLK | OR_TRLX )

#define CFG_OR0_REMAP		( CFG_REMAP_OR_AM | CFG_OR_TIMING_FLASH )

#define CFG_OR0_PRELIM		CFG_OR0_REMAP
#define CFG_BR0_PRELIM		(( CFG_FLASH_BASE & BR_BA_MSK ) | \
				   BR_PS_16 | BR_V )

/* For AdderII BR1 SDRAM */

#define CFG_PRELIM_OR1_AM 	0xFF800000
#define CFG_OR1_REMAP		( CFG_PRELIM_OR1_AM | OR_CSNT_SAM | OR_ACS_DIV2 )
#define CFG_OR1_PRELIM  	( CFG_PRELIM_OR1_AM | OR_CSNT_SAM | OR_ACS_DIV2 )
#define CFG_BR1_PRELIM  	( CFG_SDRAM_BASE | BR_MS_UPMA | BR_V )


/*******************************************************************************
* Internal Definitions Boot Flags
*******************************************************************************/
#define BOOTFLAG_COLD   0x01            /* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM   0x02            /* Software reboot                   */


#endif
/* __CONFIG_H */

#ifndef __CONFIG_H
#define __CONFIG_H

#define	CONFIG_SYS_TEXT_BASE	0x80F00000

/*****************************************************************************
 *
 * These settings must match the way _your_ board is set up
 *
 *****************************************************************************/
/* for the AY-Revision which does not use the HRCW */
#define CONFIG_SYS_DEFAULT_IMMR	0x00010000

/* What is the oscillator's (UX2) frequency in Hz? */
#define CONFIG_8260_CLKIN  (66 * 1000 * 1000)

/* How is switch S2 set? We really only want the MODCK[1-3] bits, so
 * only the 3 least significant bits are important.
*/
#define CONFIG_SYS_SBC_S2  0x04

/* What should MODCK_H be? It is dependent on the oscillator
 * frequency, MODCK[1-3], and desired CPM and core frequencies.
 * Some example values (all frequencies are in MHz):
 *
 * MODCK_H   MODCK[1-3]  Osc    CPM    Core
 * 0x2       0x2         33     133    133
 * 0x2       0x4         33     133    200
 * 0x5       0x5         66     133    133
 * 0x5       0x7         66     133    200
 */
#define CONFIG_SYS_SBC_MODCK_H 0x06

#define CONFIG_SYS_SBC_BOOT_LOW 1	/* only for HRCW */
#undef CONFIG_SYS_SBC_BOOT_LOW

/* What should the base address of the main FLASH be and how big is
 * it (in MBytes)? This must contain CONFIG_SYS_TEXT_BASE from board/sbc8260/config.mk
 * The main FLASH is whichever is connected to *CS0. U-Boot expects
 * this to be the SIMM.
 */
#define CONFIG_SYS_FLASH0_BASE 0x80000000
#define CONFIG_SYS_FLASH0_SIZE 16

/* What should the base address of the secondary FLASH be and how big
 * is it (in Mbytes)? The secondary FLASH is whichever is connected
 * to *CS6. U-Boot expects this to be the on board FLASH. If you don't
 * want it enabled, don't define these constants.
 */
#define CONFIG_SYS_FLASH1_BASE 0
#define CONFIG_SYS_FLASH1_SIZE 0
#undef CONFIG_SYS_FLASH1_BASE
#undef CONFIG_SYS_FLASH1_SIZE

/* What should be the base address of SDRAM DIMM and how big is
 * it (in Mbytes)?
*/
#define CONFIG_SYS_SDRAM0_BASE 0x00000000
#define CONFIG_SYS_SDRAM0_SIZE 64

/* What should be the base address of SDRAM DIMM and how big is
 * it (in Mbytes)?
*/
#define CONFIG_SYS_SDRAM1_BASE 0x04000000
#define CONFIG_SYS_SDRAM1_SIZE 32

/* What should be the base address of the LEDs and switch S0?
 * If you don't want them enabled, don't define this.
 */
#define CONFIG_SYS_LED_BASE 0x00000000

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
#define CONFIG_CONS_ON_SMC          /* define if console on SMC */
#undef  CONFIG_CONS_ON_SCC          /* define if console on SCC */
#undef  CONFIG_CONS_NONE            /* define if console on neither */
#define CONFIG_CONS_INDEX    1      /* which SMC/SCC channel for console */

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
#undef  CONFIG_ETHER_ON_SCC           /* define if ethernet on SCC    */
#define CONFIG_ETHER_ON_FCC           /* define if ethernet on FCC    */
#undef  CONFIG_ETHER_NONE             /* define if ethernet on neither */
#define CONFIG_ETHER_INDEX      3     /* which SCC/FCC channel for ethernet */

#if ( CONFIG_ETHER_INDEX == 3 )

/*
 * - Rx-CLK is CLK15
 * - Tx-CLK is CLK16
 * - RAM for BD/Buffers is on the 60x Bus (see 28-13)
 * - Enable Half Duplex in FSMR
 */
# define CONFIG_SYS_CMXFCR_MASK	(CMXFCR_FC3|CMXFCR_RF3CS_MSK|CMXFCR_TF3CS_MSK)
# define CONFIG_SYS_CMXFCR_VALUE	(CMXFCR_RF3CS_CLK15|CMXFCR_TF3CS_CLK16)
# define CONFIG_SYS_CPMFCR_RAMTYPE	0
/*#define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE|FCC_PSMR_LPB) */
# define CONFIG_SYS_FCC_PSMR		0

#else /* CONFIG_ETHER_INDEX */
# error "on RPX Super ethernet must be FCC3"
#endif /* CONFIG_ETHER_INDEX */

#define CONFIG_HARD_I2C         1	/* I2C with hardware support	*/
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CONFIG_SYS_I2C_SLAVE		0x7F


/* Define this to reserve an entire FLASH sector (256 KB) for
 * environment variables. Otherwise, the environment will be
 * put in the same sector as U-Boot, and changing variables
 * will erase U-Boot temporarily
 */
#define CONFIG_ENV_IN_OWN_SECT

/* Define to allow the user to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* What should the console's baud rate be? */
#define CONFIG_BAUDRATE         115200

/* Ethernet MAC address */
#define CONFIG_ETHADDR          08:00:22:50:70:63

#define CONFIG_IPADDR		192.168.1.99
#define CONFIG_SERVERIP         192.168.1.3

/* Set to a positive value to delay for running BOOTCOMMAND */
#define CONFIG_BOOTDELAY        -1

/* undef this to save memory */
#define CONFIG_SYS_LONGHELP

/* Monitor Command Prompt       */
#define CONFIG_SYS_PROMPT              "=> "


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_I2C
#define CONFIG_CMD_REGINFO

#undef CONFIG_CMD_KGDB


/* Where do the internal registers live? */
#define CONFIG_SYS_IMMR               0xF0000000

/* Where do the on board registers (CS4) live? */
#define CONFIG_SYS_REGS_BASE          0xFA000000

/*****************************************************************************
 *
 * You should not have to modify any of the following settings
 *
 *****************************************************************************/

#define CONFIG_MPC8260          1       /* This is an MPC8260 CPU   */
#define CONFIG_RPXSUPER         1       /* on an Embedded Planet RPX Super Board  */
#define CONFIG_CPM2		1	/* Has a CPM2 */

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f	*/
#define CONFIG_RESET_PHY_R	1	/* Call reset_phy()		*/

/*
 * Miscellaneous configurable options
 */
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CBSIZE              1024       /* Console I/O Buffer Size      */
#else
#  define CONFIG_SYS_CBSIZE              256        /* Console I/O Buffer Size      */
#endif

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE        (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT)+16)

#define CONFIG_SYS_MAXARGS       8            /* max number of command args   */

#define CONFIG_SYS_BARGSIZE      CONFIG_SYS_CBSIZE   /* Boot Argument Buffer Size    */

#define CONFIG_SYS_MEMTEST_START 0x04000000   /* memtest works on  */
#define CONFIG_SYS_MEMTEST_END   0x06000000   /* 64-96 MB in SDRAM */

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

#define CONFIG_SYS_LOAD_ADDR     0x100000     /* default load address */
#define CONFIG_SYS_HZ            1000         /* decrementer freq: 1 ms ticks */

/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE      { 9600, 19200, 38400, 57600, 115200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CONFIG_SYS_FLASH_BASE    CONFIG_SYS_FLASH0_BASE
#define CONFIG_SYS_SDRAM_BASE    CONFIG_SYS_SDRAM0_BASE

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 */
#if defined(CONFIG_SYS_SBC_BOOT_LOW)
#  define  CONFIG_SYS_SBC_HRCW_BOOT_FLAGS  (HRCW_CIP | HRCW_BMS)
#else
#  define  CONFIG_SYS_SBC_HRCW_BOOT_FLAGS  (0)
#endif /* defined(CONFIG_SYS_SBC_BOOT_LOW) */

/* get the HRCW ISB field from CONFIG_SYS_IMMR */
#define CONFIG_SYS_SBC_HRCW_IMMR ( ((CONFIG_SYS_IMMR & 0x10000000) >> 10) |\
			    ((CONFIG_SYS_IMMR & 0x01000000) >> 7)  |\
			    ((CONFIG_SYS_IMMR & 0x00100000) >> 4) )

#define CONFIG_SYS_HRCW_MASTER (HRCW_BPS11                           |\
			 HRCW_DPPC11                          |\
			 CONFIG_SYS_SBC_HRCW_IMMR                    |\
			 HRCW_MMR00                           |\
			 HRCW_LBPC11                          |\
			 HRCW_APPC10                          |\
			 HRCW_CS10PC00                        |\
			 (CONFIG_SYS_SBC_MODCK_H & HRCW_MODCK_H1111) |\
			 CONFIG_SYS_SBC_HRCW_BOOT_FLAGS)

/* no slaves */
#define CONFIG_SYS_HRCW_SLAVE1 0
#define CONFIG_SYS_HRCW_SLAVE2 0
#define CONFIG_SYS_HRCW_SLAVE3 0
#define CONFIG_SYS_HRCW_SLAVE4 0
#define CONFIG_SYS_HRCW_SLAVE5 0
#define CONFIG_SYS_HRCW_SLAVE6 0
#define CONFIG_SYS_HRCW_SLAVE7 0

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR       CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_SIZE        0x4000  /* Size of used area in DPRAM    */
#define CONFIG_SYS_GBL_DATA_OFFSET    (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET      CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 * Note also that the logic that sets CONFIG_SYS_RAMBOOT is platform dependent.
 */
#define CONFIG_SYS_MONITOR_BASE        (CONFIG_SYS_FLASH0_BASE + 0x00F00000)

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#  define CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_MONITOR_LEN      (256 << 10)     /* Reserve 256 kB for Monitor   */
#define CONFIG_SYS_MALLOC_LEN       (128 << 10)     /* Reserve 128 kB for malloc()  */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ        (8 << 20)       /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS   1       /* max number of memory banks         */
#define CONFIG_SYS_MAX_FLASH_SECT    71      /* max number of sectors on one chip  */

#define CONFIG_SYS_FLASH_ERASE_TOUT  8000    /* Timeout for Flash Erase (in ms)    */
#define CONFIG_SYS_FLASH_WRITE_TOUT  1       /* Timeout for Flash Write (in ms)    */

#ifndef CONFIG_SYS_RAMBOOT
#  define CONFIG_ENV_IS_IN_FLASH  1

#  ifdef CONFIG_ENV_IN_OWN_SECT
#    define CONFIG_ENV_ADDR       (CONFIG_SYS_MONITOR_BASE + 0x40000)
#    define CONFIG_ENV_SECT_SIZE  0x40000
#  else
#    define CONFIG_ENV_ADDR (CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN - CONFIG_ENV_SECT_SIZE)
#    define CONFIG_ENV_SIZE       0x1000  /* Total Size of Environment Sector */
#    define CONFIG_ENV_SECT_SIZE  0x10000 /* see README - env sect real size */
#  endif /* CONFIG_ENV_IN_OWN_SECT */
#else
#  define CONFIG_ENV_IS_IN_NVRAM  1
#  define CONFIG_ENV_ADDR         (CONFIG_SYS_MONITOR_BASE - 0x1000)
#  define CONFIG_ENV_SIZE         0x200
#endif /* CONFIG_SYS_RAMBOOT */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE      32      /* For MPC8260 CPU */

#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT     5     /* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * HIDx - Hardware Implementation-dependent Registers                    2-11
 *-----------------------------------------------------------------------
 * HID0 also contains cache control - initially enable both caches and
 * invalidate contents, then the final state leaves only the instruction
 * cache enabled. Note that Power-On and Hard reset invalidate the caches,
 * but Soft reset does not.
 *
 * HID1 has only read-only information - nothing to set.
 */
#define CONFIG_SYS_HID0_INIT   (/*HID0_ICE  |*/\
			 /*HID0_DCE  |*/\
			 HID0_ICFI |\
			 HID0_DCI  |\
			 HID0_IFEM |\
			 HID0_ABE)

#define CONFIG_SYS_HID0_FINAL  (/*HID0_ICE  |*/\
			 HID0_IFEM |\
			 HID0_ABE  |\
			 HID0_EMCP)
#define CONFIG_SYS_HID2        0

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RMR         0

/*-----------------------------------------------------------------------
 * BCR - Bus Configuration                                       4-25
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_BCR         (BCR_EBM   |\
			 BCR_PLDP  |\
			 BCR_EAV   |\
			 BCR_NPQM0)

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration                             4-31
 *-----------------------------------------------------------------------
 */

#define CONFIG_SYS_SIUMCR      (SIUMCR_L2CPC01 |\
			 SIUMCR_APPC10  |\
			 SIUMCR_CS10PC01)


/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control                            11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Watchdog & Bus Monitor Timer max, 60x Bus Monitor enable
 */
#define CONFIG_SYS_SYPCR       (SYPCR_SWTC |\
			 SYPCR_BMT  |\
			 SYPCR_PBME |\
			 SYPCR_LBME |\
			 SYPCR_SWRI |\
			 SYPCR_SWP)

/*-----------------------------------------------------------------------
 * TMCNTSC - Time Counter Status and Control                     4-40
 *-----------------------------------------------------------------------
 * Clear once per Second and Alarm Interrupt Status, Set 32KHz timersclk,
 * and enable Time Counter
 */
#define CONFIG_SYS_TMCNTSC     (TMCNTSC_SEC |\
			 TMCNTSC_ALR |\
			 TMCNTSC_TCF |\
			 TMCNTSC_TCE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control                 4-42
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Set 32KHz timersclk, and enable
 * Periodic timer
 */
#define CONFIG_SYS_PISCR       (PISCR_PS  |\
			 PISCR_PTF |\
			 PISCR_PTE)

/*-----------------------------------------------------------------------
 * SCCR - System Clock Control                                   9-8
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_SCCR        (SCCR_DFBRG01)

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration                         13-7
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RCCR        0

/*
 * Init Memory Controller:
 *
 * Bank Bus     Machine PortSz  Device
 * ---- ---     ------- ------  ------
 *  0   60x     GPCM    64 bit  FLASH (BGA - 16MB AMD AM29DL323DB90)
 *  1   60x     SDRAM   64 bit  SDRAM (BGA - 64MB Hitachi HM5225325FBP-B60)
 *  2   Local   SDRAM   32 bit  SDRAM (BGA - 32MB Hitachi HM5225325FBP-B60)
 *  3   unused
 *  4   60x     GPCM     8 bit  Board Regs, LEDs, switches
 *  5   unused
 *  6   unused
 *  7   unused
 *  8   PCMCIA
 *  9   unused
 * 10   unused
 * 11   unused
*/

/* Bank 0 - FLASH
 *
 */
#define CONFIG_SYS_BR0_PRELIM  ((CONFIG_SYS_FLASH0_BASE & BRx_BA_MSK) |\
			 BRx_PS_64                      |\
			 BRx_DECC_NONE                  |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CONFIG_SYS_OR0_PRELIM  (MEG_TO_AM(CONFIG_SYS_FLASH0_SIZE)     |\
			 ORxG_CSNT                      |\
			 ORxG_ACS_DIV1                  |\
			 ORxG_SCY_6_CLK                 |\
			 ORxG_EHTR)

/* Bank 1 - SDRAM
 *
 */
#define CONFIG_SYS_BR1_PRELIM  ((CONFIG_SYS_SDRAM0_BASE & BRx_BA_MSK) |\
			 BRx_PS_64                      |\
			 BRx_MS_SDRAM_P                 |\
			 BRx_V)

#define CONFIG_SYS_OR1_PRELIM  (MEG_TO_AM(CONFIG_SYS_SDRAM0_SIZE)     |\
			 ORxS_BPD_4                     |\
			 ORxS_ROWST_PBI0_A8             |\
			 ORxS_NUMR_12                   |\
			 ORxS_IBID)

#define CONFIG_SYS_PSDMR       0x014DA412
#define CONFIG_SYS_PSRT	0x79


/* Bank 2 - SDRAM
 *
 */
#define CONFIG_SYS_BR2_PRELIM  ((CONFIG_SYS_SDRAM1_BASE & BRx_BA_MSK) |\
			 BRx_PS_32                      |\
			 BRx_MS_SDRAM_L                 |\
			 BRx_V)

#define CONFIG_SYS_OR2_PRELIM  (MEG_TO_AM(CONFIG_SYS_SDRAM1_SIZE)     |\
			 ORxS_BPD_4                     |\
			 ORxS_ROWST_PBI0_A9             |\
			 ORxS_NUMR_12)

#define CONFIG_SYS_LSDMR       0x0169A512
#define CONFIG_SYS_LSRT	0x79

#define CONFIG_SYS_MPTPR	(0x0800 & MPTPR_PTP_MSK)

/* Bank 4 - On board registers
 *
 */
#define CONFIG_SYS_BR4_PRELIM   ((CONFIG_SYS_REGS_BASE & BRx_BA_MSK)  |\
			   BRx_PS_8                     |\
			   BRx_MS_GPCM_P                |\
			   BRx_V)

#define CONFIG_SYS_OR4_PRELIM    (ORxG_AM_MSK                 |\
			   ORxG_CSNT                   |\
			   ORxG_ACS_DIV1               |\
			   ORxG_SCY_5_CLK              |\
			   ORxG_TRLX)

#endif  /* __CONFIG_H */

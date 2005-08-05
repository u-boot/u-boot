#ifndef __CONFIG_H
#define __CONFIG_H


/*****************************************************************************
 *
 * These settings must match the way _your_ board is set up
 *
 *****************************************************************************/
/* for the AY-Revision which does not use the HRCW */
#define CFG_DEFAULT_IMMR	0x00010000

/* What is the oscillator's (UX2) frequency in Hz? */
#define CONFIG_8260_CLKIN  (66 * 1000 * 1000)

/* How is switch S2 set? We really only want the MODCK[1-3] bits, so
 * only the 3 least significant bits are important.
*/
#define CFG_SBC_S2  0x04

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
#define CFG_SBC_MODCK_H 0x06

#define CFG_SBC_BOOT_LOW 1	/* only for HRCW */
#undef CFG_SBC_BOOT_LOW

/* What should the base address of the main FLASH be and how big is
 * it (in MBytes)? This must contain TEXT_BASE from board/sbc8260/config.mk
 * The main FLASH is whichever is connected to *CS0. U-Boot expects
 * this to be the SIMM.
 */
#define CFG_FLASH0_BASE 0x80000000
#define CFG_FLASH0_SIZE 16

/* What should the base address of the secondary FLASH be and how big
 * is it (in Mbytes)? The secondary FLASH is whichever is connected
 * to *CS6. U-Boot expects this to be the on board FLASH. If you don't
 * want it enabled, don't define these constants.
 */
#define CFG_FLASH1_BASE 0
#define CFG_FLASH1_SIZE 0
#undef CFG_FLASH1_BASE
#undef CFG_FLASH1_SIZE

/* What should be the base address of SDRAM DIMM and how big is
 * it (in Mbytes)?
*/
#define CFG_SDRAM0_BASE 0x00000000
#define CFG_SDRAM0_SIZE 64

/* What should be the base address of SDRAM DIMM and how big is
 * it (in Mbytes)?
*/
#define CFG_SDRAM1_BASE 0x04000000
#define CFG_SDRAM1_SIZE 32

/* What should be the base address of the LEDs and switch S0?
 * If you don't want them enabled, don't define this.
 */
#define CFG_LED_BASE 0x00000000

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
 * defined elsewhere (as for the console), or CFG_CMD_NET must be removed
 * from CONFIG_COMMANDS to remove support for networking.
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
# define CFG_CMXFCR_MASK	(CMXFCR_FC3|CMXFCR_RF3CS_MSK|CMXFCR_TF3CS_MSK)
# define CFG_CMXFCR_VALUE	(CMXFCR_RF3CS_CLK15|CMXFCR_TF3CS_CLK16)
# define CFG_CPMFCR_RAMTYPE	0
/*#define CFG_FCC_PSMR		(FCC_PSMR_FDE|FCC_PSMR_LPB) */
# define CFG_FCC_PSMR		0

#else /* CONFIG_ETHER_INDEX */
# error "on RPX Super ethernet must be FCC3"
#endif /* CONFIG_ETHER_INDEX */

#define CONFIG_HARD_I2C         1	/* I2C with hardware support	*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F


/* Define this to reserve an entire FLASH sector (256 KB) for
 * environment variables. Otherwise, the environment will be
 * put in the same sector as U-Boot, and changing variables
 * will erase U-Boot temporarily
 */
#define CFG_ENV_IN_OWN_SECT

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
#define CFG_LONGHELP

/* Monitor Command Prompt       */
#define CFG_PROMPT              "=> "

/* What U-Boot subsytems do you want enabled? */
#define CONFIG_COMMANDS         ( CONFIG_CMD_DFL | \
				  CFG_CMD_IMMAP  | \
				  CFG_CMD_ASKENV | \
				  CFG_CMD_ECHO   | \
				  CFG_CMD_I2C    | \
				  CFG_CMD_REGINFO & \
				 ~CFG_CMD_KGDB )

/* Where do the internal registers live? */
#define CFG_IMMR               0xF0000000

/* Where do the on board registers (CS4) live? */
#define CFG_REGS_BASE          0xFA000000

/*****************************************************************************
 *
 * You should not have to modify any of the following settings
 *
 *****************************************************************************/

#define CONFIG_MPC8260          1       /* This is an MPC8260 CPU   */
#define CONFIG_RPXSUPER         1       /* on an Embedded Planet RPX Super Board  */
#define CONFIG_CPM2		1	/* Has a CPM2 */

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f	*/

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Miscellaneous configurable options
 */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#  define CFG_CBSIZE              1024       /* Console I/O Buffer Size      */
#else
#  define CFG_CBSIZE              256        /* Console I/O Buffer Size      */
#endif

/* Print Buffer Size */
#define CFG_PBSIZE        (CFG_CBSIZE + sizeof(CFG_PROMPT)+16)

#define CFG_MAXARGS       8            /* max number of command args   */

#define CFG_BARGSIZE      CFG_CBSIZE   /* Boot Argument Buffer Size    */

#define CFG_MEMTEST_START 0x04000000   /* memtest works on  */
#define CFG_MEMTEST_END   0x06000000   /* 64-96 MB in SDRAM */

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

#define CFG_LOAD_ADDR     0x100000     /* default load address */
#define CFG_HZ            1000         /* decrementer freq: 1 ms ticks */

/* valid baudrates */
#define CFG_BAUDRATE_TABLE      { 9600, 19200, 38400, 57600, 115200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CFG_FLASH_BASE    CFG_FLASH0_BASE
#define CFG_SDRAM_BASE    CFG_SDRAM0_BASE

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 */
#if defined(CFG_SBC_BOOT_LOW)
#  define  CFG_SBC_HRCW_BOOT_FLAGS  (HRCW_CIP | HRCW_BMS)
#else
#  define  CFG_SBC_HRCW_BOOT_FLAGS  (0)
#endif /* defined(CFG_SBC_BOOT_LOW) */

/* get the HRCW ISB field from CFG_IMMR */
#define CFG_SBC_HRCW_IMMR ( ((CFG_IMMR & 0x10000000) >> 10) |\
			    ((CFG_IMMR & 0x01000000) >> 7)  |\
			    ((CFG_IMMR & 0x00100000) >> 4) )

#define CFG_HRCW_MASTER (HRCW_BPS11                           |\
			 HRCW_DPPC11                          |\
			 CFG_SBC_HRCW_IMMR                    |\
			 HRCW_MMR00                           |\
			 HRCW_LBPC11                          |\
			 HRCW_APPC10                          |\
			 HRCW_CS10PC00                        |\
			 (CFG_SBC_MODCK_H & HRCW_MODCK_H1111) |\
			 CFG_SBC_HRCW_BOOT_FLAGS)

/* no slaves */
#define CFG_HRCW_SLAVE1 0
#define CFG_HRCW_SLAVE2 0
#define CFG_HRCW_SLAVE3 0
#define CFG_HRCW_SLAVE4 0
#define CFG_HRCW_SLAVE5 0
#define CFG_HRCW_SLAVE6 0
#define CFG_HRCW_SLAVE7 0

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR       CFG_IMMR
#define CFG_INIT_RAM_END        0x4000  /* End of used area in DPRAM    */
#define CFG_GBL_DATA_SIZE      128     /* bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET    (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET      CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 * Note also that the logic that sets CFG_RAMBOOT is platform dependent.
 */
#define CFG_MONITOR_BASE        (CFG_FLASH0_BASE + 0x00F00000)

#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#  define CFG_RAMBOOT
#endif

#define CFG_MONITOR_LEN      (256 << 10)     /* Reserve 256 kB for Monitor   */
#define CFG_MALLOC_LEN       (128 << 10)     /* Reserve 128 kB for malloc()  */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ        (8 << 20)       /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS   1       /* max number of memory banks         */
#define CFG_MAX_FLASH_SECT    71      /* max number of sectors on one chip  */

#define CFG_FLASH_ERASE_TOUT  8000    /* Timeout for Flash Erase (in ms)    */
#define CFG_FLASH_WRITE_TOUT  1       /* Timeout for Flash Write (in ms)    */

#ifndef CFG_RAMBOOT
#  define CFG_ENV_IS_IN_FLASH  1

#  ifdef CFG_ENV_IN_OWN_SECT
#    define CFG_ENV_ADDR       (CFG_MONITOR_BASE + 0x40000)
#    define CFG_ENV_SECT_SIZE  0x40000
#  else
#    define CFG_ENV_ADDR (CFG_FLASH_BASE + CFG_MONITOR_LEN - CFG_ENV_SECT_SIZE)
#    define CFG_ENV_SIZE       0x1000  /* Total Size of Environment Sector */
#    define CFG_ENV_SECT_SIZE  0x10000 /* see README - env sect real size */
#  endif /* CFG_ENV_IN_OWN_SECT */
#else
#  define CFG_ENV_IS_IN_NVRAM  1
#  define CFG_ENV_ADDR         (CFG_MONITOR_BASE - 0x1000)
#  define CFG_ENV_SIZE         0x200
#endif /* CFG_RAMBOOT */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE      32      /* For MPC8260 CPU */

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT     5     /* log base 2 of the above value */
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
#define CFG_HID0_INIT   (/*HID0_ICE  |*/\
			 /*HID0_DCE  |*/\
			 HID0_ICFI |\
			 HID0_DCI  |\
			 HID0_IFEM |\
			 HID0_ABE)

#define CFG_HID0_FINAL  (/*HID0_ICE  |*/\
			 HID0_IFEM |\
			 HID0_ABE  |\
			 HID0_EMCP)
#define CFG_HID2        0

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register
 *-----------------------------------------------------------------------
 */
#define CFG_RMR         0

/*-----------------------------------------------------------------------
 * BCR - Bus Configuration                                       4-25
 *-----------------------------------------------------------------------
 */
#define CFG_BCR         (BCR_EBM   |\
			 BCR_PLDP  |\
			 BCR_EAV   |\
			 BCR_NPQM0)

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration                             4-31
 *-----------------------------------------------------------------------
 */

#define CFG_SIUMCR      (SIUMCR_L2CPC01 |\
			 SIUMCR_APPC10  |\
			 SIUMCR_CS10PC01)


/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control                            11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Watchdog & Bus Monitor Timer max, 60x Bus Monitor enable
 */
#define CFG_SYPCR       (SYPCR_SWTC |\
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
#define CFG_TMCNTSC     (TMCNTSC_SEC |\
			 TMCNTSC_ALR |\
			 TMCNTSC_TCF |\
			 TMCNTSC_TCE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control                 4-42
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Set 32KHz timersclk, and enable
 * Periodic timer
 */
#define CFG_PISCR       (PISCR_PS  |\
			 PISCR_PTF |\
			 PISCR_PTE)

/*-----------------------------------------------------------------------
 * SCCR - System Clock Control                                   9-8
 *-----------------------------------------------------------------------
 */
#define CFG_SCCR        (SCCR_DFBRG01)

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration                         13-7
 *-----------------------------------------------------------------------
 */
#define CFG_RCCR        0

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
#define CFG_BR0_PRELIM  ((CFG_FLASH0_BASE & BRx_BA_MSK) |\
			 BRx_PS_64                      |\
			 BRx_DECC_NONE                  |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CFG_OR0_PRELIM  (MEG_TO_AM(CFG_FLASH0_SIZE)     |\
			 ORxG_CSNT                      |\
			 ORxG_ACS_DIV1                  |\
			 ORxG_SCY_6_CLK                 |\
			 ORxG_EHTR)

/* Bank 1 - SDRAM
 *
 */
#define CFG_BR1_PRELIM  ((CFG_SDRAM0_BASE & BRx_BA_MSK) |\
			 BRx_PS_64                      |\
			 BRx_MS_SDRAM_P                 |\
			 BRx_V)

#define CFG_OR1_PRELIM  (MEG_TO_AM(CFG_SDRAM0_SIZE)     |\
			 ORxS_BPD_4                     |\
			 ORxS_ROWST_PBI0_A8             |\
			 ORxS_NUMR_12                   |\
			 ORxS_IBID)

#define CFG_PSDMR       0x014DA412
#define CFG_PSRT	0x79


/* Bank 2 - SDRAM
 *
 */
#define CFG_BR2_PRELIM  ((CFG_SDRAM1_BASE & BRx_BA_MSK) |\
			 BRx_PS_32                      |\
			 BRx_MS_SDRAM_L                 |\
			 BRx_V)

#define CFG_OR2_PRELIM  (MEG_TO_AM(CFG_SDRAM1_SIZE)     |\
			 ORxS_BPD_4                     |\
			 ORxS_ROWST_PBI0_A9             |\
			 ORxS_NUMR_12)

#define CFG_LSDMR       0x0169A512
#define CFG_LSRT	0x79

#define CFG_MPTPR	(0x0800 & MPTPR_PTP_MSK)

/* Bank 4 - On board registers
 *
 */
#define CFG_BR4_PRELIM   ((CFG_REGS_BASE & BRx_BA_MSK)  |\
			   BRx_PS_8                     |\
			   BRx_MS_GPCM_P                |\
			   BRx_V)

#define CFG_OR4_PRELIM    (ORxG_AM_MSK                 |\
			   ORxG_CSNT                   |\
			   ORxG_ACS_DIV1               |\
			   ORxG_SCY_5_CLK              |\
			   ORxG_TRLX)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD   0x01    /* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM   0x02    /* Software reboot                   */

#endif  /* __CONFIG_H */

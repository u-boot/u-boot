/*
 * (C) Copyright 2007-2010
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __KM82XX_COMMON
#define __KM82XX_COMMON

/*
 * Select serial console configuration
 *
 * If either CONFIG_CONS_ON_SMC or CONFIG_CONS_ON_SCC is selected, then
 * CONFIG_CONS_INDEX must be set to the channel number (1-2 for SMC, 1-4
 * for SCC).
 */
#define	CONFIG_CONS_ON_SMC		/* Console is on SMC         */
#undef  CONFIG_CONS_ON_SCC		/* It's not on SCC           */
#undef	CONFIG_CONS_NONE		/* It's not on external UART */
#define CONFIG_CONS_INDEX	2	/* SMC2 is used for console  */
#define CONFIG_SYS_SMC_RXBUFLEN	128
#define CONFIG_SYS_MAXIDLE	10

/*
 * Select ethernet configuration
 *
 * If either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected,
 * then CONFIG_ETHER_INDEX must be set to the channel number (1-4 for
 * SCC, 1-3 for FCC)
 *
 * If CONFIG_ETHER_NONE is defined, then either the ethernet routines
 * must be defined elsewhere (as for the console), or CONFIG_CMD_NET
 * must be unset.
 */
#define	CONFIG_ETHER_ON_SCC		/* Ethernet is on SCC */
#undef	CONFIG_ETHER_ON_FCC		/* Ethernet is not on FCC     */
#undef	CONFIG_ETHER_NONE		/* No external Ethernet   */
#define CONFIG_NET_MULTI

#define CONFIG_ETHER_INDEX	4
#define CONFIG_HAS_ETH0
#define CONFIG_SYS_SCC_TOUT_LOOP	10000000

#define CONFIG_SYS_CMXSCR_VALUE	(CMXSCR_RS4CS_CLK7 | CMXSCR_TS4CS_CLK8)

#ifndef CONFIG_8260_CLKIN
#define CONFIG_8260_CLKIN	66000000	/* in Hz */
#endif

#define BOOTFLASH_START		0xFE000000

#define CONFIG_KM_CONSOLE_TTY	"ttyCPM0"

#define MTDPARTS_DEFAULT	"mtdparts="				\
	"app:"								\
		"768k(u-boot),"						\
		"128k(env),"						\
		"128k(envred),"						\
		"3072k(free),"						\
		"-(" CONFIG_KM_UBI_PARTITION_NAME ")"

/*
 * Default environment settings
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_KM_BOARD_EXTRA_ENV					\
	CONFIG_KM_DEF_ENV						\
	"EEprom_ivm=pca9544a:70:4 \0"					\
	"unlock=yes\0"							\
	"newenv="							\
		"prot off 0xFE0C0000 +0x40000 && "			\
		"era 0xFE0C0000 +0x40000\0"				\
	"arch=ppc_82xx\0"					\
	""

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_MONITOR_LEN		(768 << 10)

#define CONFIG_ENV_IS_IN_FLASH

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + \
					CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_OFFSET	CONFIG_SYS_MONITOR_LEN

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + \
						CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND		(CONFIG_ENV_SIZE)
#endif /* CONFIG_ENV_IS_IN_FLASH */

/* enable I2C and select the hardware/software driver */
#undef	CONFIG_HARD_I2C			/* I2C with hardware support	*/
#define	CONFIG_SOFT_I2C			/* I2C bit-banged		*/
#define CONFIG_SYS_I2C_SPEED		50000	/* I2C speed */
#define CONFIG_SYS_I2C_SLAVE		0x7F	/* I2C slave address */

/*
 * Software (bit-bang) I2C driver configuration
 */

#define I2C_PORT	3		/* Port A=0, B=1, C=2, D=3 */
#define I2C_ACTIVE	(iop->pdir |=  0x00010000)
#define I2C_TRISTATE	(iop->pdir &= ~0x00010000)
#define I2C_READ	((iop->pdat & 0x00010000) != 0)
#define I2C_SDA(bit)	do { \
				if (bit) \
					iop->pdat |=  0x00010000; \
				else \
					iop->pdat &= ~0x00010000; \
			} while (0)
#define I2C_SCL(bit)	do { \
				if (bit) \
					iop->pdat |=  0x00020000; \
				else \
					iop->pdat &= ~0x00020000; \
			} while (0)
#define I2C_DELAY	udelay(5)	/* 1/4 I2C clock duration */

#ifndef __ASSEMBLY__
void set_sda(int state);
void set_scl(int state);
int get_sda(void);
int get_scl(void);
#endif

/* I2C SYSMON (LM75, AD7414 is almost compatible)			*/
#define CONFIG_DTT_LM75			/* ON Semi's LM75		*/
#define CONFIG_DTT_SENSORS	{0}	/* Sensor addresses		*/
#define CONFIG_SYS_DTT_MAX_TEMP	70
#define CONFIG_SYS_DTT_LOW_TEMP	-30
#define CONFIG_SYS_DTT_HYSTERESIS	3
#define CONFIG_SYS_DTT_BUS_NUM		(CONFIG_SYS_MAX_I2C_BUS)

#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1

#define CONFIG_SYS_IMMR		0xF0000000

#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_SIZE	0x2000 /* used size in DPRAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
						GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/* Hard reset configuration word */
#define CONFIG_SYS_HRCW_MASTER		0x0604b211

/* No slaves */
#define CONFIG_SYS_HRCW_SLAVE1		0
#define CONFIG_SYS_HRCW_SLAVE2		0
#define CONFIG_SYS_HRCW_SLAVE3		0
#define CONFIG_SYS_HRCW_SLAVE4		0
#define CONFIG_SYS_HRCW_SLAVE5		0
#define CONFIG_SYS_HRCW_SLAVE6		0
#define CONFIG_SYS_HRCW_SLAVE7		0

/* Initial Memory map for Linux */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)

#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC8260 CPUs */
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5 /* log base 2 of the above value */
#endif

#define CONFIG_SYS_HID0_INIT		0
#define CONFIG_SYS_HID0_FINAL		(HID0_ICE | HID0_IFEM | HID0_ABE)

#define CONFIG_SYS_HID2		0

#define CONFIG_SYS_SIUMCR		0x4020c200
#define CONFIG_SYS_SYPCR		0xFFFFFF83
#define CONFIG_SYS_BCR			0x10000000
#define CONFIG_SYS_SCCR		(SCCR_PCI_MODE | SCCR_PCI_MODCK)

/*
 *-----------------------------------------------------------------------
 * RMR - Reset Mode Register                                     5-5
 *-----------------------------------------------------------------------
 * turn on Checkstop Reset Enable
 */
#define CONFIG_SYS_RMR         0

/*
 *-----------------------------------------------------------------------
 * TMCNTSC - Time Counter Status and Control                     4-40
 *-----------------------------------------------------------------------
 * Clear once per Second and Alarm Interrupt Status, Set 32KHz timersclk,
 * and enable Time Counter
 */
#define CONFIG_SYS_TMCNTSC     (TMCNTSC_SEC|TMCNTSC_ALR|TMCNTSC_TCF|TMCNTSC_TCE)

/*
 *-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control                 4-42
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Set 32KHz timersclk, and enable
 * Periodic timer
 */
#define CONFIG_SYS_PISCR       (PISCR_PS|PISCR_PTF|PISCR_PTE)

/*
 *-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration                         13-7
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RCCR        0

/*
 * Init Memory Controller:
 *
 * Bank Bus     Machine PortSz  Device
 * ---- ---     ------- ------  ------
 *  0   60x     GPCM     8 bit  FLASH
 *  1   60x     SDRAM   32 bit  SDRAM
 *  3   60x     GPCM     8 bit  GPIO/PIGGY
 *  5   60x     GPCM    16 bit  CFG-Flash
 *
 */
/* Bank 0 - FLASH
 */
#define CONFIG_SYS_BR0_PRELIM  ((CONFIG_SYS_FLASH_BASE & BRx_BA_MSK)	|\
			 BRx_PS_8			|\
			 BRx_MS_GPCM_P			|\
			 BRx_V)

#define CONFIG_SYS_OR0_PRELIM  (MEG_TO_AM(CONFIG_SYS_FLASH_SIZE)	|\
			 ORxG_CSNT			|\
			 ORxG_ACS_DIV2			|\
			 ORxG_SCY_5_CLK			|\
			 ORxG_TRLX)

#define CONFIG_SYS_MPTPR       0x1800

/*
 *-----------------------------------------------------------------------------
 * Address for Mode Register Set (MRS) command
 *-----------------------------------------------------------------------------
 */
#define CONFIG_SYS_MRS_OFFS	0x00000110
#define CONFIG_SYS_PSRT        0x0e

#define CONFIG_SYS_BR1_PRELIM ((CONFIG_SYS_SDRAM_BASE & BRx_BA_MSK) |\
			 BRx_PS_64		|\
			 BRx_MS_SDRAM_P		|\
			 BRx_V)

#define CONFIG_SYS_OR1_PRELIM	CONFIG_SYS_OR1

/*
 * UPIO FPGA (GPIO/PIGGY) on CS3 initialization values
 */
#define CONFIG_SYS_KMBEC_FPGA_BASE	0x30000000
#define CONFIG_SYS_KMBEC_FPGA_SIZE	128

#define CONFIG_SYS_BR3_PRELIM	((CONFIG_SYS_KMBEC_FPGA_BASE & BRx_BA_MSK) |\
			 BRx_PS_8 | BRx_MS_GPCM_P | BRx_V)

#define CONFIG_SYS_OR3_PRELIM	(MEG_TO_AM(CONFIG_SYS_KMBEC_FPGA_SIZE) |\
			 ORxG_CSNT | ORxG_ACS_DIV2 |\
			 ORxG_SCY_3_CLK | ORxG_TRLX)

/*
 * BFTICU board FPGA on CS4 initialization values
 */
#define CONFIG_SYS_FPGA_BASE	0x40000000
#define CONFIG_SYS_FPGA_SIZE	1 /*1KB*/

#define CONFIG_SYS_BR4_PRELIM ((CONFIG_SYS_FPGA_BASE & BRx_BA_MSK) |\
			BRx_PS_8 | BRx_MS_GPCM_P | BRx_V)

#define CONFIG_SYS_OR4_PRELIM (P2SZ_TO_AM(CONFIG_SYS_FPGA_SIZE << 10) |\
			 ORxG_CSNT | ORxG_ACS_DIV2 |\
			 ORxG_SCY_3_CLK | ORxG_TRLX)

/*
 * CFG-Flash on CS5 initialization values
 */
#define CONFIG_SYS_BR5_PRELIM	((CONFIG_SYS_FLASH_BASE_1 & BRx_BA_MSK) |\
			 BRx_PS_16 | BRx_MS_GPCM_P | BRx_V)

#define CONFIG_SYS_OR5_PRELIM	(MEG_TO_AM(CONFIG_SYS_FLASH_SIZE_1 + \
				 CONFIG_SYS_FLASH_SIZE_2) |\
				 ORxG_CSNT | ORxG_ACS_DIV2 |\
				 ORxG_SCY_5_CLK | ORxG_TRLX)

#define	CONFIG_SYS_RESET_ADDRESS 0xFDFFFFFC	/* "bad" address */

/* pass open firmware flat tree */
#define CONFIG_FIT		1
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH		"/soc/cpm/serial@11a90"

#endif /* __KM82XX_COMMON */

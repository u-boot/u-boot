/*
 * (C) Copyright 2009
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

/*
 * configuration options, keymile 8xx board specific
 */

#ifndef __CONFIG_KM8XX_H
#define __CONFIG_KM8XX_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_KM8XX		1	/* on a km8xx board */

/* include common defines/options for all Keymile boards */
#include "keymile-common.h"

#if defined(CONFIG_KMSUPX4)
#undef	CONFIG_I2C_MUX			/* no I2C mux on this board */
#endif

#define CONFIG_8xx_GCLK_FREQ		66000000

#define CONFIG_SYS_SMC_UCODE_PATCH	1	/* Relocate SMC1 */
#define CONFIG_SYS_SMC_DPMEM_OFFSET	0x1fc0
#define CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1 */
#define CONFIG_SYS_SMC_RXBUFLEN	128
#define CONFIG_SYS_MAXIDLE	10

#define CONFIG_SYS_CPM_BOOTCOUNT_ADDR	0x1eb0	/* In case of SMC relocation,
						 * the default value is not
						 * working
						 */

#define BOOTFLASH_START	F0000000
#define CONFIG_PRAM	512	/* protected RAM [KBytes] */

#define CONFIG_PREBOOT	"echo;" \
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#define BOOTFLASH_START	F0000000
#define CONFIG_PRAM	512	/* protected RAM [KBytes] */

#if defined(CONFIG_MGSUVD)
#define CONFIG_ENV_IVM	"EEprom_ivm=pca9544a:70:4 \0"
#else
#define CONFIG_ENV_IVM	""
#endif

#define MTDIDS_DEFAULT		"nor0=app"
#define MTDPARTS_DEFAULT \
	"mtdparts=app:384k(u-boot),128k(env),128k(envred),128k(free),"	\
	"1536k(esw0),8704k(rootfs0),1536k(esw1),2432k(rootfs1),640k(var)," \
	"768k(cfg)"

#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_KM_DEF_ENV						\
	"rootpath=/opt/eldk/ppc_8xx\0"					\
	"addcon=setenv bootargs ${bootargs} "				\
		"console=ttyCPM0,${baudrate}\0"				\
	"mtdids=nor0=app \0"						\
	"mtdparts=" MK_STR(MTDPARTS_DEFAULT) "\0"			\
	"partition=nor0,9 \0"						\
	"new_env=prot off F0060000 F009FFFF; era F0060000 F009FFFF \0" 	\
	CONFIG_ENV_IVM							\
	""

#undef CONFIG_RTC_MPC8xx		/* MPC866 does not support RTC	*/

#define	CONFIG_TIMESTAMP		/* but print image timestmps	*/

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xFFF00000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_END	0x2F00	/* End of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_SIZE	64
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xf0000000
#define CONFIG_SYS_MONITOR_LEN		(384 << 10) /* 384 kB for Monitor */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)

/*-----------------------------------------------------------------------
 * FLASH organization
 */
/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_FLASH_SIZE		32
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
/* max num of sects on one chip */
#define CONFIG_SYS_MAX_FLASH_SECT	256

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* (in ms) */

#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_OFFSET	CONFIG_SYS_MONITOR_LEN
#define CONFIG_ENV_SECT_SIZE	0x20000 /* Total Size of Environment Sector */

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET+CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#define CONFIG_ENV_BUFFER_PRINT		1

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16	/* For all MPC8xx CPUs	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	4	/* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#define CONFIG_SYS_SYPCR	0xffffff89

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 */
#if defined(CONFIG_MGSUVD)
#define CONFIG_SYS_SIUMCR	0x00610480
#else
#define CONFIG_SYS_SIUMCR	0x00610400
#endif

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#if defined(CONFIG_MGSUVD)
#define SCCR_MASK	0x01800000
#else
#define SCCR_MASK	0x00000000
#endif
#define CONFIG_SYS_SCCR	0x01800000

#define CONFIG_SYS_DER 0

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0xf0000000	/* FLASH bank #0	*/

/* used to re-map FLASH both when starting from SRAM or FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
#define CONFIG_SYS_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CONFIG_SYS_PRELIM_OR_AM	0xE0000000	/* OR addr mask */

/*
 * FLASH timing: Default value of OR0 after reset
 */
#define CONFIG_SYS_OR0_PRELIM	0xfe000954
#define CONFIG_SYS_BR0_PRELIM	0xf0000401

/*
 * BR1 and OR1 (SDRAM)
 *
 */
#define SDRAM_BASE1_PRELIM	0x00000000	/* SDRAM bank #0	*/
#define SDRAM_MAX_SIZE		(64 << 20)	/* max 64 MB per bank	*/

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care) */
#define CONFIG_SYS_OR_TIMING_SDRAM	0x00000A00

#define CONFIG_SYS_OR1_PRELIM	0xfc000800
#define CONFIG_SYS_BR1_PRELIM	(0x000000C0 | 0x01)

#define CONFIG_SYS_MPTPR	0x0200
/* PTB=16, AMB=001, FIXME 1 RAS precharge cycles, 1 READ loop cycle (not used),
   1 Write loop Cycle (not used), 1 Timer Loop Cycle */
#if defined(CONFIG_MGSUVD)
#define CONFIG_SYS_MBMR	0x10964111
#else
#define CONFIG_SYS_MBMR	0x20964111
#endif
#define CONFIG_SYS_MAR		0x00000088

/*
 * 4096	Rows from SDRAM example configuration
 * 1000	factor s -> ms
 * 64	PTP (pre-divider from MPTPR) from SDRAM example configuration
 * 4	Number of refresh cycles per period
 * 64	Refresh cycle in ms per number of rows
 */
#define CONFIG_SYS_PTA_PER_CLK	((4096 * 64 * 1000) / (4 * 64))

/* GPIO/PIGGY on CS3 initialization values
*/
#define CONFIG_SYS_PIGGY_BASE	(0x30000000)
#if defined(CONFIG_MGSUVD)
#define CONFIG_SYS_OR3_PRELIM	(0xfe000d24)
#define CONFIG_SYS_BR3_PRELIM	(0x30000401)
#else
#define CONFIG_SYS_OR3_PRELIM	(0xf8000d26)
#define CONFIG_SYS_BR3_PRELIM	(0x30000401)
#endif

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02	/* Software reboot		    */

#define CONFIG_SCC3_ENET
#define CONFIG_ETHPRIME		"SCC"
#define CONFIG_HAS_ETH0

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_STDOUT_PATH		"/soc/cpm/serial@a80"

/* enable I2C and select the hardware/software driver */
#undef	CONFIG_HARD_I2C			/* I2C with hardware support */
#define	CONFIG_SOFT_I2C		1	/* I2C bit-banged	*/
/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SPEED		50000
#define CONFIG_SYS_I2C_SLAVE		0x7F
#define I2C_SOFT_DECLARATIONS

/*
 * Software (bit-bang) I2C driver configuration
 */
#define I2C_BASE_DIR	((u16 *)(CONFIG_SYS_PIGGY_BASE + 0x04))
#define I2C_BASE_PORT	((u8 *)(CONFIG_SYS_PIGGY_BASE + 0x09))

#define SDA_BIT		0x40
#define SCL_BIT		0x80
#define SDA_CONF	0x1000
#define SCL_CONF	0x2000

#define I2C_ACTIVE	do {} while (0)
#define I2C_TRISTATE	do {} while (0)
#define I2C_READ	((in_8(I2C_BASE_PORT) & SDA_BIT) == SDA_BIT)
#define I2C_SDA(bit)	if(bit) { \
				clrbits(be16, I2C_BASE_DIR, SDA_CONF); \
			} else { \
				clrbits(8, I2C_BASE_PORT, SDA_BIT); \
				setbits(be16, I2C_BASE_DIR, SDA_CONF); \
			}
#define I2C_SCL(bit)	if(bit) { \
				clrbits(be16, I2C_BASE_DIR, SCL_CONF); \
			} else { \
				clrbits(8, I2C_BASE_PORT, SCL_BIT); \
				setbits(be16, I2C_BASE_DIR, SCL_CONF); \
			}
#define I2C_DELAY	udelay(50)	/* 1/4 I2C clock duration */

#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1

/* I2C SYSMON (LM75, AD7414 is almost compatible)		*/
#define CONFIG_DTT_LM75		1	/* ON Semi's LM75	*/
#if defined(CONFIG_MGSUVD)
#define CONFIG_DTT_SENSORS	{0, 2, 4, 6}	/* Sensor addresses */
#else
#define CONFIG_DTT_SENSORS	{0}	/* Sensor addresses */
#endif
#define CONFIG_SYS_DTT_MAX_TEMP	70
#define CONFIG_SYS_DTT_LOW_TEMP	-30
#define CONFIG_SYS_DTT_HYSTERESIS	3
#define CONFIG_SYS_DTT_BUS_NUM		(CONFIG_SYS_MAX_I2C_BUS)
#endif	/* __CONFIG_KM8XX_H */

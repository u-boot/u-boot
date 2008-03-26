/*
 * (C) Copyright 2008
 * AT91CAP9 definitions
 * Author : ATMEL AT91 application group
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef AT91CAP9_H
#define AT91CAP9_H

typedef volatile unsigned int AT91_REG;

/* Static Memory Controller */
typedef struct _AT91S_SMC {
	AT91_REG	SMC_SETUP0;	/* Setup Register for CS 0 */
	AT91_REG	SMC_PULSE0;	/* Pulse Register for CS 0 */
	AT91_REG	SMC_CYCLE0;	/* Cycle Register for CS 0 */
	AT91_REG	SMC_CTRL0;	/* Control Register for CS 0 */
	AT91_REG	SMC_SETUP1;	/* Setup Register for CS 1 */
	AT91_REG	SMC_PULSE1;	/* Pulse Register for CS 1 */
	AT91_REG	SMC_CYCLE1;	/* Cycle Register for CS 1 */
	AT91_REG	SMC_CTRL1;	/* Control Register for CS 1 */
	AT91_REG	SMC_SETUP2;	/* Setup Register for CS 2 */
	AT91_REG	SMC_PULSE2;	/* Pulse Register for CS 2 */
	AT91_REG	SMC_CYCLE2;	/* Cycle Register for CS 2 */
	AT91_REG	SMC_CTRL2;	/* Control Register for CS 2 */
	AT91_REG	SMC_SETUP3;	/* Setup Register for CS 3 */
	AT91_REG	SMC_PULSE3;	/* Pulse Register for CS 3 */
	AT91_REG	SMC_CYCLE3;	/* Cycle Register for CS 3 */
	AT91_REG	SMC_CTRL3;	/* Control Register for CS 3 */
	AT91_REG	SMC_SETUP4;	/* Setup Register for CS 4 */
	AT91_REG	SMC_PULSE4;	/* Pulse Register for CS 4 */
	AT91_REG	SMC_CYCLE4;	/* Cycle Register for CS 4 */
	AT91_REG	SMC_CTRL4;	/* Control Register for CS 4 */
	AT91_REG	SMC_SETUP5;	/* Setup Register for CS 5 */
	AT91_REG	SMC_PULSE5;	/* Pulse Register for CS 5 */
	AT91_REG	SMC_CYCLE5;	/* Cycle Register for CS 5 */
	AT91_REG	SMC_CTRL5;	/* Control Register for CS 5 */
	AT91_REG	SMC_SETUP6;	/* Setup Register for CS 6 */
	AT91_REG	SMC_PULSE6;	/* Pulse Register for CS 6 */
	AT91_REG	SMC_CYCLE6;	/* Cycle Register for CS 6 */
	AT91_REG	SMC_CTRL6;	/* Control Register for CS 6 */
	AT91_REG	SMC_SETUP7;	/* Setup Register for CS 7 */
	AT91_REG	SMC_PULSE7;	/* Pulse Register for CS 7 */
	AT91_REG	SMC_CYCLE7;	/* Cycle Register for CS 7 */
	AT91_REG	SMC_CTRL7;	/* Control Register for CS 7 */
} AT91S_SMC, *AT91PS_SMC;

/* SMC_SETUP : (SMC Offset: 0x0) Setup Register for CS x */
#define AT91C_SMC_NWESETUP	(0x3F <<  0)	/* NWE Setup Length */
#define AT91C_SMC_NCSSETUPWR	(0x3F <<  8)	/* NCS Setup Length for WRite */
#define AT91C_SMC_NRDSETUP	(0x3F << 16)	/* NRD Setup Length */
#define AT91C_SMC_NCSSETUPRD	(0x3F << 24)	/* NCS Setup Length for ReaD */
/* SMC_PULSE : (SMC Offset: 0x4) Pulse Register for CS x */
#define AT91C_SMC_NWEPULSE	(0x7F <<  0)	/* NWE Pulse Length */
#define AT91C_SMC_NCSPULSEWR	(0x7F <<  8)	/* NCS Pulse Length for WRite */
#define AT91C_SMC_NRDPULSE	(0x7F << 16)	/* NRD Pulse Length */
#define AT91C_SMC_NCSPULSERD	(0x7F << 24)	/* NCS Pulse Length for ReaD */
/* SMC_CYC : (SMC Offset: 0x8) Cycle Register for CS x */
#define AT91C_SMC_NWECYCLE	(0x1FF <<  0)	/* Total Write Cycle Length */
#define AT91C_SMC_NRDCYCLE	(0x1FF << 16)	/* Total Read Cycle Length */
/* SMC_CTRL : (SMC Offset: 0xc) Control Register for CS x */
#define AT91C_SMC_READMODE	(0x1 <<  0)	/* Read Mode */
#define AT91C_SMC_WRITEMODE	(0x1 <<  1)	/* Write Mode */
#define AT91C_SMC_NWAITM	(0x3 <<  5)	/* NWAIT Mode */
		/* External NWAIT disabled */
#define		AT91C_SMC_NWAITM_NWAIT_DISABLE		(0x0 <<  5)
		/* External NWAIT enabled in frozen mode */
#define		AT91C_SMC_NWAITM_NWAIT_ENABLE_FROZEN	(0x2 <<  5)
		/* External NWAIT enabled in ready mode */
#define		AT91C_SMC_NWAITM_NWAIT_ENABLE_READY	(0x3 <<  5)
#define AT91C_SMC_BAT		(0x1 <<  8)	/* Byte Access Type */
		/*
		 * Write controled by ncs, nbs0, nbs1, nbs2, nbs3.
		 * Read controled by ncs, nrd, nbs0, nbs1, nbs2, nbs3.
		 */
#define		AT91C_SMC_BAT_BYTE_SELECT		(0x0 <<  8)
		/*
		 * Write controled by ncs, nwe0, nwe1, nwe2, nwe3.
		 * Read controled by ncs and nrd.
		 */
#define		AT91C_SMC_BAT_BYTE_WRITE		(0x1 <<  8)
#define AT91C_SMC_DBW		(0x3 << 12)	/* Data Bus Width */
#define		AT91C_SMC_DBW_WIDTH_EIGTH_BITS		(0x0 << 12)
#define		AT91C_SMC_DBW_WIDTH_SIXTEEN_BITS	(0x1 << 12)
#define		AT91C_SMC_DBW_WIDTH_THIRTY_TWO_BITS	(0x2 << 12)
#define AT91C_SMC_TDF		(0xF << 16)	/* Data Float Time */
#define AT91C_SMC_TDFEN		(0x1 << 20)	/* TDF Enabled */
#define AT91C_SMC_PMEN		(0x1 << 24)	/* Page Mode Enabled */
#define AT91C_SMC_PS		(0x3 << 28)	/* Page Size */
#define		AT91C_SMC_PS_SIZE_FOUR_BYTES		(0x0 << 28)
#define		AT91C_SMC_PS_SIZE_EIGHT_BYTES		(0x1 << 28)
#define		AT91C_SMC_PS_SIZE_SIXTEEN_BYTES		(0x2 << 28)
#define		AT91C_SMC_PS_SIZE_THIRTY_TWO_BYTES	(0x3 << 28)
/* SMC_SETUP : (SMC Offset: 0x10) Setup Register for CS x */
/* SMC_PULSE : (SMC Offset: 0x14) Pulse Register for CS x */
/* SMC_CYC : (SMC Offset: 0x18) Cycle Register for CS x */
/* SMC_CTRL : (SMC Offset: 0x1c) Control Register for CS x */
/* SMC_SETUP : (SMC Offset: 0x20) Setup Register for CS x */
/* SMC_PULSE : (SMC Offset: 0x24) Pulse Register for CS x */
/* SMC_CYC : (SMC Offset: 0x28) Cycle Register for CS x */
/* SMC_CTRL : (SMC Offset: 0x2c) Control Register for CS x */
/* SMC_SETUP : (SMC Offset: 0x30) Setup Register for CS x */
/* SMC_PULSE : (SMC Offset: 0x34) Pulse Register for CS x */
/* SMC_CYC : (SMC Offset: 0x38) Cycle Register for CS x */
/* SMC_CTRL : (SMC Offset: 0x3c) Control Register for CS x */
/* SMC_SETUP : (SMC Offset: 0x40) Setup Register for CS x */
/* SMC_PULSE : (SMC Offset: 0x44) Pulse Register for CS x */
/* SMC_CYC : (SMC Offset: 0x48) Cycle Register for CS x */
/* SMC_CTRL : (SMC Offset: 0x4c) Control Register for CS x */
/* SMC_SETUP : (SMC Offset: 0x50) Setup Register for CS x */
/* SMC_PULSE : (SMC Offset: 0x54) Pulse Register for CS x */
/* SMC_CYC : (SMC Offset: 0x58) Cycle Register for CS x */
/* SMC_CTRL : (SMC Offset: 0x5c) Control Register for CS x */
/* SMC_SETUP : (SMC Offset: 0x60) Setup Register for CS x */
/* SMC_PULSE : (SMC Offset: 0x64) Pulse Register for CS x */
/* SMC_CYC : (SMC Offset: 0x68) Cycle Register for CS x */
/* SMC_CTRL : (SMC Offset: 0x6c) Control Register for CS x */
/* SMC_SETUP : (SMC Offset: 0x70) Setup Register for CS x */
/* SMC_PULSE : (SMC Offset: 0x74) Pulse Register for CS x */
/* SMC_CYC : (SMC Offset: 0x78) Cycle Register for CS x */
/* SMC_CTRL : (SMC Offset: 0x7c) Control Register for CS x */

/* AHB CCFG */
typedef struct _AT91S_CCFG {
	AT91_REG	Reserved0[1];
	AT91_REG	CCFG_MPBS0;	/* MPB Slave 0 */
	AT91_REG	CCFG_UDPHS;	/* AHB Periphs */
	AT91_REG	CCFG_MPBS1;	/* MPB Slave 1 */
	AT91_REG	CCFG_EBICSA;	/* EBI Chip Select Assignement */
	AT91_REG	Reserved1[2];
	AT91_REG	CCFG_MPBS2;	/* MPB Slave 2 */
	AT91_REG	CCFG_MPBS3;	/* MPB Slave 3 */
	AT91_REG	CCFG_BRIDGE;	/* APB Bridge */
	AT91_REG	Reserved2[49];
	AT91_REG	CCFG_MATRIXVERSION;/* Version */
} AT91S_CCFG, *AT91PS_CCFG;

/* CCFG_UDPHS : (CCFG Offset: 0x8) UDPHS Configuration */
#define AT91C_CCFG_UDPHS_UDP_SELECT	(0x1 << 31)	/* UDPHS or UDP */
#define		AT91C_CCFG_UDPHS_UDP_SELECT_UDPHS	(0x0 << 31)
#define		AT91C_CCFG_UDPHS_UDP_SELECT_UDP		(0x1 << 31)
/* CCFG_EBICSA : (CCFG Offset: 0x10) EBI Chip Select Assignement Register */
#define AT91C_EBI_CS1A			(0x1 <<  1)	/* CS1 Assignment */
#define		AT91C_EBI_CS1A_SMC			(0x0 <<  1)
#define		AT91C_EBI_CS1A_BCRAMC			(0x1 <<  1)
#define AT91C_EBI_CS3A			(0x1 <<  3)	/* CS 3 Assignment */
#define		AT91C_EBI_CS3A_SMC			(0x0 <<  3)
#define		AT91C_EBI_CS3A_SM			(0x1 <<  3)
#define AT91C_EBI_CS4A			(0x1 <<  4)	/* CS4 Assignment */
#define		AT91C_EBI_CS4A_SMC			(0x0 <<  4)
#define		AT91C_EBI_CS4A_CF			(0x1 <<  4)
#define AT91C_EBI_CS5A			(0x1 <<  5)	/* CS 5 Assignment */
#define		AT91C_EBI_CS5A_SMC			(0x0 <<  5)
#define		AT91C_EBI_CS5A_CF			(0x1 <<  5)
#define AT91C_EBI_DBPUC			(0x1 <<  8)	/* Data Bus Pull-up */
#define AT91C_EBI_DDRPUC		(0x1 <<  9)	/* DDDR DQS Pull-up */
#define AT91C_EBI_SUP			(0x1 << 16)	/* EBI Supply */
#define		AT91C_EBI_SUP_1V8			(0x0 << 16)
#define		AT91C_EBI_SUP_3V3			(0x1 << 16)
#define AT91C_EBI_LP			(0x1 << 17)	/* EBI Low Power */
#define		AT91C_EBI_LP_LOW_DRIVE			(0x0 << 17)
#define		AT91C_EBI_LP_STD_DRIVE			(0x1 << 17)
#define AT91C_CCFG_DDR_SDR_SELECT	(0x1 << 31)	/* DDR or SDR */
#define		AT91C_CCFG_DDR_SDR_SELECT_DDR		(0x0 << 31)
#define		AT91C_CCFG_DDR_SDR_SELECT_SDR		(0x1 << 31)
/* CCFG_BRIDGE : (CCFG Offset: 0x24) BRIDGE Configuration */
#define AT91C_CCFG_AES_TDES_SELECT	(0x1 << 31)	/* AES or TDES */
#define		AT91C_CCFG_AES_TDES_SELECT_AES		(0x0 << 31)
#define		AT91C_CCFG_AES_TDES_SELECT_TDES		(0x1 << 31)

/* PIO controller */
typedef struct _AT91S_PIO {
	AT91_REG	PIO_PER;	/* PIO Enable Register */
	AT91_REG	PIO_PDR;	/* PIO Disable Register */
	AT91_REG	PIO_PSR;	/* PIO Status Register */
	AT91_REG	Reserved0[1];
	AT91_REG	PIO_OER;	/* Output Enable Register */
	AT91_REG	PIO_ODR;	/* Output Disable Register */
	AT91_REG	PIO_OSR;	/* Output Status Register */
	AT91_REG	Reserved1[1];
	AT91_REG	PIO_IFER;	/* Input Filter Enable Register */
	AT91_REG	PIO_IFDR;	/* Input Filter Disable Register */
	AT91_REG	PIO_IFSR;	/* Input Filter Status Register */
	AT91_REG	Reserved2[1];
	AT91_REG	PIO_SODR;	/* Set Output Data Register */
	AT91_REG	PIO_CODR;	/* Clear Output Data Register */
	AT91_REG	PIO_ODSR;	/* Output Data Status Register */
	AT91_REG	PIO_PDSR;	/* Pin Data Status Register */
	AT91_REG	PIO_IER;	/* Interrupt Enable Register */
	AT91_REG	PIO_IDR;	/* Interrupt Disable Register */
	AT91_REG	PIO_IMR;	/* Interrupt Mask Register */
	AT91_REG	PIO_ISR;	/* Interrupt Status Register */
	AT91_REG	PIO_MDER;	/* Multi-driver Enable Register */
	AT91_REG	PIO_MDDR;	/* Multi-driver Disable Register */
	AT91_REG	PIO_MDSR;	/* Multi-driver Status Register */
	AT91_REG	Reserved3[1];
	AT91_REG	PIO_PPUDR;	/* Pull-up Disable Register */
	AT91_REG	PIO_PPUER;	/* Pull-up Enable Register */
	AT91_REG	PIO_PPUSR;	/* Pull-up Status Register */
	AT91_REG	Reserved4[1];
	AT91_REG	PIO_ASR;	/* Select A Register */
	AT91_REG	PIO_BSR;	/* Select B Register */
	AT91_REG	PIO_ABSR;	/* AB Select Status Register */
	AT91_REG	Reserved5[9];
	AT91_REG	PIO_OWER;	/* Output Write Enable Register */
	AT91_REG	PIO_OWDR;	/* Output Write Disable Register */
	AT91_REG	PIO_OWSR;	/* Output Write Status Register */
} AT91S_PIO, *AT91PS_PIO;

/* Power Management Controller */
typedef struct _AT91S_PMC {
	AT91_REG	PMC_SCER;	/* System Clock Enable Register */
	AT91_REG	PMC_SCDR;	/* System Clock Disable Register */
	AT91_REG	PMC_SCSR;	/* System Clock Status Register */
	AT91_REG	Reserved0[1];
	AT91_REG	PMC_PCER;	/* Peripheral Clock Enable Register */
	AT91_REG	PMC_PCDR;	/* Peripheral Clock Disable Register */
	AT91_REG	PMC_PCSR;	/* Peripheral Clock Status Register */
	AT91_REG	PMC_UCKR;	/* UTMI Clock Configuration Register */
	AT91_REG	PMC_MOR;	/* Main Oscillator Register */
	AT91_REG	PMC_MCFR;	/* Main Clock  Frequency Register */
	AT91_REG	PMC_PLLAR;	/* PLL A Register */
	AT91_REG	PMC_PLLBR;	/* PLL B Register */
	AT91_REG	PMC_MCKR;	/* Master Clock Register */
	AT91_REG	Reserved1[3];
	AT91_REG	PMC_PCKR[8];	/* Programmable Clock Register */
	AT91_REG	PMC_IER;	/* Interrupt Enable Register */
	AT91_REG	PMC_IDR;	/* Interrupt Disable Register */
	AT91_REG	PMC_SR;		/* Status Register */
	AT91_REG	PMC_IMR;	/* Interrupt Mask Register */
} AT91S_PMC, *AT91PS_PMC;

/* PMC_SCER : (PMC Offset: 0x0) System Clock Enable Register */
#define AT91C_PMC_PCK		(0x1 <<  0)	/* Processor Clock */
#define AT91C_PMC_OTG		(0x1 <<  5)	/* USB OTG Clock */
#define AT91C_PMC_UHP		(0x1 <<  6)	/* USB Host Port Clock */
#define AT91C_PMC_UDP		(0x1 <<  7)	/* USB Device Port Clock */
#define AT91C_PMC_PCK0		(0x1 <<  8)	/* Programmable Clock Output */
#define AT91C_PMC_PCK1		(0x1 <<  9)	/* Programmable Clock Output */
#define AT91C_PMC_PCK2		(0x1 << 10)	/* Programmable Clock Output */
#define AT91C_PMC_PCK3		(0x1 << 11)	/* Programmable Clock Output */
/* PMC_SCDR : (PMC Offset: 0x4) System Clock Disable Register */
/* PMC_SCSR : (PMC Offset: 0x8) System Clock Status Register */
/* CKGR_UCKR : (PMC Offset: 0x1c) UTMI Clock Configuration Register */
/* CKGR_MOR : (PMC Offset: 0x20) Main Oscillator Register */
/* CKGR_MCFR : (PMC Offset: 0x24) Main Clock Frequency Register */
/* CKGR_PLLAR : (PMC Offset: 0x28) PLL A Register */
/* CKGR_PLLBR : (PMC Offset: 0x2c) PLL B Register */
/* PMC_MCKR : (PMC Offset: 0x30) Master Clock Register */
#define AT91C_PMC_CSS		(0x3 <<  0)	/* Clock Selection */
#define		AT91C_PMC_CSS_SLOW_CLK		(0x0 <<  0)	/* Slow Clk */
#define		AT91C_PMC_CSS_MAIN_CLK		(0x1 <<  0)	/* Main Clk */
#define		AT91C_PMC_CSS_PLLA_CLK		(0x2 <<  0)	/* PLL A Clk */
#define		AT91C_PMC_CSS_PLLB_CLK		(0x3 <<  0)	/* PLL B Clk */
#define AT91C_PMC_PRES		(0x7 <<  2)	/* Clock Prescaler */
#define		AT91C_PMC_PRES_CLK		(0x0 <<  2)
#define		AT91C_PMC_PRES_CLK_2		(0x1 <<  2)
#define		AT91C_PMC_PRES_CLK_4		(0x2 <<  2)
#define		AT91C_PMC_PRES_CLK_8		(0x3 <<  2)
#define		AT91C_PMC_PRES_CLK_16		(0x4 <<  2)
#define		AT91C_PMC_PRES_CLK_32		(0x5 <<  2)
#define		AT91C_PMC_PRES_CLK_64		(0x6 <<  2)
#define AT91C_PMC_MDIV		(0x3 <<  8)	/* Master Clock Division */
#define		AT91C_PMC_MDIV_1		(0x0 <<  8)
#define		AT91C_PMC_MDIV_2		(0x1 <<  8)
#define		AT91C_PMC_MDIV_4		(0x2 <<  8)
/* PMC_PCKR : (PMC Offset: 0x40) Programmable Clock Register */
/* PMC_IER : (PMC Offset: 0x60) PMC Interrupt Enable Register */
#define AT91C_PMC_MOSCS		(0x1 <<  0)	/* MOSC mask */
#define AT91C_PMC_LOCKA		(0x1 <<  1)	/* PLL A mask */
#define AT91C_PMC_LOCKB		(0x1 <<  2)	/* PLL B mask */
#define AT91C_PMC_MCKRDY	(0x1 <<  3)	/* Master mask */
#define AT91C_PMC_LOCKU		(0x1 <<  6)	/* PLL UTMI mask */
#define AT91C_PMC_PCK0RDY	(0x1 <<  8)	/* PCK0_RDY mask */
#define AT91C_PMC_PCK1RDY	(0x1 <<  9)	/* PCK1_RDY mask */
#define AT91C_PMC_PCK2RDY	(0x1 << 10)	/* PCK2_RDY mask */
#define AT91C_PMC_PCK3RDY	(0x1 << 11)	/* PCK3_RDY mask */
/* PMC_IDR : (PMC Offset: 0x64) PMC Interrupt Disable Register */
/* PMC_SR : (PMC Offset: 0x68) PMC Status Register */
/* PMC_IMR : (PMC Offset: 0x6c) PMC Interrupt Mask Register */

/* Reset controller */
typedef struct _AT91S_RSTC {
	AT91_REG	RSTC_RCR;	/* Reset Control Register */
	AT91_REG	RSTC_RSR;	/* Reset Status Register */
	AT91_REG	RSTC_RMR;	/* Reset Mode Register */
} AT91S_RSTC, *AT91PS_RSTC;

/* RSTC_RCR : (RSTC Offset: 0x0) Reset Control Register */
#define AT91C_RSTC_PROCRST	(0x1 <<  0)	/* Processor Reset */
#define AT91C_RSTC_ICERST	(0x1 <<  1)	/* ICE Interface Reset */
#define AT91C_RSTC_PERRST	(0x1 <<  2)	/* Peripheral Reset */
#define AT91C_RSTC_EXTRST	(0x1 <<  3)	/* External Reset */
#define AT91C_RSTC_KEY		(0xFF << 24)	/* Password */
/* RSTC_RSR : (RSTC Offset: 0x4) Reset Status Register */
#define AT91C_RSTC_URSTS	(0x1 <<  0)	/* User Reset Status */
#define AT91C_RSTC_RSTTYP	(0x7 <<  8)	/* Reset Type */
#define		AT91C_RSTC_RSTTYP_GENERAL	(0x0 <<  8)
#define		AT91C_RSTC_RSTTYP_WAKEUP	(0x1 <<  8)
#define		AT91C_RSTC_RSTTYP_WATCHDOG	(0x2 <<  8)
#define		AT91C_RSTC_RSTTYP_SOFTWARE	(0x3 <<  8)
#define		AT91C_RSTC_RSTTYP_USER		(0x4 <<  8)
#define AT91C_RSTC_NRSTL	(0x1 << 16)	/* NRST pin level */
#define AT91C_RSTC_SRCMP	(0x1 << 17)	/* Software Rst in Progress. */
/* RSTC_RMR : (RSTC Offset: 0x8) Reset Mode Register */
#define AT91C_RSTC_URSTEN	(0x1 <<  0)	/* User Reset Enable */
#define AT91C_RSTC_URSTIEN	(0x1 <<  4)	/* User Reset Int. Enable */
#define AT91C_RSTC_ERSTL	(0xF <<  8)	/* User Reset Enable */

/* Periodic Timer Controller */
typedef struct _AT91S_PITC {
	AT91_REG	PITC_PIMR;	/* Period Interval Mode Register */
	AT91_REG	PITC_PISR;	/* Period Interval Status Register */
	AT91_REG	PITC_PIVR;	/* Period Interval Value Register */
	AT91_REG	PITC_PIIR;	/* Period Interval Image Register */
} AT91S_PITC, *AT91PS_PITC;

/* PITC_PIMR : (PITC Offset: 0x0) Periodic Interval Mode Register */
#define AT91C_PITC_PIV		(0xFFFFF <<  0)	/* Periodic Interval Value */
#define AT91C_PITC_PITEN	(0x1 << 24)	/* PIT Enable */
#define AT91C_PITC_PITIEN	(0x1 << 25)	/* PIT Interrupt Enable */
/* PITC_PISR : (PITC Offset: 0x4) Periodic Interval Status Register */
#define AT91C_PITC_PITS		(0x1 <<  0)	/* PIT Status */
/* PITC_PIVR : (PITC Offset: 0x8) Periodic Interval Value Register */
#define AT91C_PITC_CPIV		(0xFFFFF <<  0)	/* Current Value */
#define AT91C_PITC_PICNT	(0xFFF << 20)	/* Periodic Interval Counter */
/* PITC_PIIR : (PITC Offset: 0xc) Periodic Interval Image Register */

/* Serial Paraller Interface */
typedef struct _AT91S_SPI {
	AT91_REG	SPI_CR;		/* Control Register */
	AT91_REG	SPI_MR;		/* Mode Register */
	AT91_REG	SPI_RDR;	/* Receive Data Register */
	AT91_REG	SPI_TDR;	/* Transmit Data Register */
	AT91_REG	SPI_SR;		/* Status Register */
	AT91_REG	SPI_IER;	/* Interrupt Enable Register */
	AT91_REG	SPI_IDR;	/* Interrupt Disable Register */
	AT91_REG	SPI_IMR;	/* Interrupt Mask Register */
	AT91_REG	Reserved0[4];
	AT91_REG	SPI_CSR[4];	/* Chip Select Register */
	AT91_REG	Reserved1[48];
	AT91_REG	SPI_RPR;	/* Receive Pointer Register */
	AT91_REG	SPI_RCR;	/* Receive Counter Register */
	AT91_REG	SPI_TPR;	/* Transmit Pointer Register */
	AT91_REG	SPI_TCR;	/* Transmit Counter Register */
	AT91_REG	SPI_RNPR;	/* Receive Next Pointer Register */
	AT91_REG	SPI_RNCR;	/* Receive Next Counter Register */
	AT91_REG	SPI_TNPR;	/* Transmit Next Pointer Register */
	AT91_REG	SPI_TNCR;	/* Transmit Next Counter Register */
	AT91_REG	SPI_PTCR;	/* PDC Transfer Control Register */
	AT91_REG	SPI_PTSR;	/* PDC Transfer Status Register */
} AT91S_SPI, *AT91PS_SPI;

/* SPI_CR : (SPI Offset: 0x0) SPI Control Register */
#define AT91C_SPI_SPIEN		(0x1 <<  0)	/* SPI Enable */
#define AT91C_SPI_SPIDIS	(0x1 <<  1)	/* SPI Disable */
#define AT91C_SPI_SWRST		(0x1 <<  7)	/* SPI Software reset */
#define AT91C_SPI_LASTXFER	(0x1 << 24)	/* SPI Last Transfer */
/* SPI_MR : (SPI Offset: 0x4) SPI Mode Register */
#define AT91C_SPI_MSTR		(0x1 <<  0)	/* Master/Slave Mode */
#define AT91C_SPI_PS		(0x1 <<  1)	/* Peripheral Select */
#define		AT91C_SPI_PS_FIXED		(0x0 <<  1)
#define		AT91C_SPI_PS_VARIABLE		(0x1 <<  1)
#define AT91C_SPI_PCSDEC	(0x1 <<  2)	/* Chip Select Decode */
#define AT91C_SPI_FDIV		(0x1 <<  3)	/* Clock Selection */
#define AT91C_SPI_MODFDIS	(0x1 <<  4)	/* Mode Fault Detection */
#define AT91C_SPI_LLB		(0x1 <<  7)	/* Clock Selection */
#define AT91C_SPI_PCS		(0xF << 16)	/* Peripheral Chip Select */
#define AT91C_SPI_DLYBCS	(0xFF << 24)	/* Delay Between Chip Selects */
/* SPI_RDR : (SPI Offset: 0x8) Receive Data Register */
#define AT91C_SPI_RD		(0xFFFF <<  0)	/* Receive Data */
#define AT91C_SPI_RPCS		(0xF << 16)	/* Peripheral CS Status */
/* SPI_TDR : (SPI Offset: 0xc) Transmit Data Register */
#define AT91C_SPI_TD		(0xFFFF <<  0)	/* Transmit Data */
#define AT91C_SPI_TPCS		(0xF << 16)	/* Peripheral CS Status */
/* SPI_SR : (SPI Offset: 0x10) Status Register */
#define AT91C_SPI_RDRF		(0x1 <<  0)	/* Receive Data Register Full */
#define AT91C_SPI_TDRE		(0x1 <<  1)	/* Trans. Data Register Empty */
#define AT91C_SPI_MODF		(0x1 <<  2)	/* Mode Fault Error */
#define AT91C_SPI_OVRES		(0x1 <<  3)	/* Overrun Error Status */
#define AT91C_SPI_ENDRX		(0x1 <<  4)	/* End of Receiver Transfer */
#define AT91C_SPI_ENDTX		(0x1 <<  5)	/* End of Receiver Transfer */
#define AT91C_SPI_RXBUFF	(0x1 <<  6)	/* RXBUFF Interrupt */
#define AT91C_SPI_TXBUFE	(0x1 <<  7)	/* TXBUFE Interrupt */
#define AT91C_SPI_NSSR		(0x1 <<  8)	/* NSSR Interrupt */
#define AT91C_SPI_TXEMPTY	(0x1 <<  9)	/* TXEMPTY Interrupt */
#define AT91C_SPI_SPIENS	(0x1 << 16)	/* Enable Status */
/* SPI_IER : (SPI Offset: 0x14) Interrupt Enable Register */
/* SPI_IDR : (SPI Offset: 0x18) Interrupt Disable Register */
/* SPI_IMR : (SPI Offset: 0x1c) Interrupt Mask Register */
/* SPI_CSR : (SPI Offset: 0x30) Chip Select Register */
#define AT91C_SPI_CPOL		(0x1 <<  0)	/* Clock Polarity */
#define AT91C_SPI_NCPHA		(0x1 <<  1)	/* Clock Phase */
#define AT91C_SPI_CSAAT		(0x1 <<  3)	/* CS Active After Transfer */
#define AT91C_SPI_BITS		(0xF <<  4)	/* Bits Per Transfer */
#define		AT91C_SPI_BITS_8		(0x0 <<  4)	/* 8 Bits */
#define		AT91C_SPI_BITS_9		(0x1 <<  4)	/* 9 Bits */
#define		AT91C_SPI_BITS_10		(0x2 <<  4)	/* 10 Bits */
#define		AT91C_SPI_BITS_11		(0x3 <<  4)	/* 11 Bits */
#define		AT91C_SPI_BITS_12		(0x4 <<  4)	/* 12 Bits */
#define		AT91C_SPI_BITS_13		(0x5 <<  4)	/* 13 Bits */
#define		AT91C_SPI_BITS_14		(0x6 <<  4)	/* 14 Bits */
#define		AT91C_SPI_BITS_15		(0x7 <<  4)	/* 15 Bits */
#define		AT91C_SPI_BITS_16		(0x8 <<  4)	/* 16 Bits */
#define AT91C_SPI_SCBR		(0xFF <<  8)	/* Serial Clock Baud Rate */
#define AT91C_SPI_DLYBS		(0xFF << 16)	/* Delay Before SPCK */
#define AT91C_SPI_DLYBCT	(0xFF << 24)	/* Delay Between Transfers */
/* SPI_PTCR : PDC Transfer Control Register */
#define AT91C_PDC_RXTEN		(0x1 <<  0)	/* Receiver Transfer Enable */
#define AT91C_PDC_RXTDIS	(0x1 <<  1)	/* Receiver Transfer Disable */
#define AT91C_PDC_TXTEN		(0x1 <<  8)	/* Transm. Transfer Enable */
#define AT91C_PDC_TXTDIS	(0x1 <<  9)	/* Transm. Transfer Disable */

/* PIO definitions */
#define AT91C_PIO_PA0		(1 <<  0)	/* Pin Controlled by PA0 */
#define AT91C_PA0_SPI0_MISO	AT91C_PIO_PA0
#define AT91C_PIO_PA1		(1 <<  1)	/* Pin Controlled by PA1 */
#define AT91C_PA1_SPI0_MOSI	AT91C_PIO_PA1
#define AT91C_PIO_PA2		(1 <<  2)	/* Pin Controlled by PA2 */
#define AT91C_PA2_SPI0_SPCK	AT91C_PIO_PA2
#define AT91C_PIO_PA3		(1 <<  3)	/* Pin Controlled by PA3 */
#define AT91C_PA3_SPI0_NPCS1	AT91C_PIO_PA3
#define AT91C_PIO_PA4		(1 <<  4)	/* Pin Controlled by PA4 */
#define AT91C_PA4_SPI0_NPCS2A	AT91C_PIO_PA4
#define AT91C_PIO_PA5		(1 <<  5)	/* Pin Controlled by PA5 */
#define AT91C_PA5_SPI0_NPCS0	AT91C_PIO_PA5
#define AT91C_PIO_PA10		(1 << 10)	/* Pin Controlled by PA10 */
#define AT91C_PIO_PA11		(1 << 11)	/* Pin Controlled by PA11 */
#define AT91C_PIO_PA22		(1 << 22)	/* Pin Controlled by PA22 */
#define AT91C_PA22_TXD0		AT91C_PIO_PA22
#define AT91C_PIO_PA23		(1 << 23)	/* Pin Controlled by PA23 */
#define AT91C_PA23_RXD0		AT91C_PIO_PA23
#define AT91C_PIO_PA28		(1 << 28)	/* Pin Controlled by PA28 */
#define AT91C_PA28_SPI0_NPCS3A	AT91C_PIO_PA28
#define AT91C_PIO_PB21		(1 << 21)	/* Pin Controlled by PB21 */
#define AT91C_PB21_E_TXCK	AT91C_PIO_PB21
#define AT91C_PIO_PB22		(1 << 22)	/* Pin Controlled by PB22 */
#define AT91C_PB22_E_RXDV	AT91C_PIO_PB22
#define AT91C_PIO_PB23		(1 << 23)	/* Pin Controlled by PB23 */
#define AT91C_PB23_E_TX0	AT91C_PIO_PB23
#define AT91C_PIO_PB24		(1 << 24)	/* Pin Controlled by PB24 */
#define AT91C_PB24_E_TX1	AT91C_PIO_PB24
#define AT91C_PIO_PB25		(1 << 25)	/* Pin Controlled by PB25 */
#define AT91C_PB25_E_RX0	AT91C_PIO_PB25
#define AT91C_PIO_PB26		(1 << 26)	/* Pin Controlled by PB26 */
#define AT91C_PB26_E_RX1	AT91C_PIO_PB26
#define AT91C_PIO_PB27		(1 << 27)	/* Pin Controlled by PB27 */
#define AT91C_PB27_E_RXER	AT91C_PIO_PB27
#define AT91C_PIO_PB28		(1 << 28)	/* Pin Controlled by PB28 */
#define AT91C_PB28_E_TXEN	AT91C_PIO_PB28
#define AT91C_PIO_PB29		(1 << 29)	/* Pin Controlled by PB29 */
#define AT91C_PB29_E_MDC	AT91C_PIO_PB29
#define AT91C_PIO_PB30		(1 << 30)	/* Pin Controlled by PB30 */
#define AT91C_PB30_E_MDIO	AT91C_PIO_PB30
#define AT91C_PIO_PB31		(1 << 31)	/* Pin Controlled by PB31 */
#define AT91C_PIO_PC29		(1 << 29)	/* Pin Controlled by PC29 */
#define AT91C_PIO_PC30		(1 << 30)	/* Pin Controlled by PC30 */
#define AT91C_PC30_DRXD		AT91C_PIO_PC30
#define AT91C_PIO_PC31		(1 << 31)	/* Pin Controlled by PC31 */
#define AT91C_PC31_DTXD		AT91C_PIO_PC31
#define AT91C_PIO_PD0		(1 <<  0)	/* Pin Controlled by PD0 */
#define AT91C_PD0_TXD1		AT91C_PIO_PD0
#define AT91C_PD0_SPI0_NPCS2D	AT91C_PIO_PD0
#define AT91C_PIO_PD1		(1 <<  1)	/* Pin Controlled by PD1 */
#define AT91C_PD1_RXD1		AT91C_PIO_PD1
#define AT91C_PD1_SPI0_NPCS3D	AT91C_PIO_PD1
#define AT91C_PIO_PD2		(1 <<  2)	/* Pin Controlled by PD2 */
#define AT91C_PD2_TXD2		AT91C_PIO_PD2
#define AT91C_PIO_PD3		(1 <<  3)	/* Pin Controlled by PD3 */
#define AT91C_PD3_RXD2		AT91C_PIO_PD3
#define AT91C_PIO_PD15		(1 << 15)	/* Pin Controlled by PD15 */

/* Peripheral ID */
#define AT91C_ID_SYS		 1	/* System Controller */
#define AT91C_ID_PIOABCD	 2	/* Parallel IO Controller A, B, C, D */
#define AT91C_ID_US0		 8	/* USART 0 */
#define AT91C_ID_US1		 9	/* USART 1 */
#define AT91C_ID_US2		10	/* USART 2 */
#define AT91C_ID_SPI0		15	/* Serial Peripheral Interface 0 */
#define AT91C_ID_EMAC		22	/* Ethernet Mac */
#define AT91C_ID_UHP		29	/* USB Host Port */

/* Base addresses */
#define AT91C_BASE_SMC		((AT91PS_SMC)	0xFFFFE800)	/* SMC */
#define AT91C_BASE_CCFG		((AT91PS_CCFG)	0xFFFFEB10)	/* CCFG */
#define AT91C_BASE_DBGU		((unsigned long)0xFFFFEE00)	/* DBGU */
#define AT91C_BASE_PIOA		((AT91PS_PIO)	0xFFFFF200)	/* PIOA */
#define AT91C_BASE_PIOB		((AT91PS_PIO)	0xFFFFF400)	/* PIOB */
#define AT91C_BASE_PIOC		((AT91PS_PIO)	0xFFFFF600)	/* PIOC */
#define AT91C_BASE_PIOD		((AT91PS_PIO)	0xFFFFF800)	/* PIOD */
#define AT91C_BASE_PMC		((AT91PS_PMC)	0xFFFFFC00)	/* PMC */
#define AT91C_BASE_RSTC		((AT91PS_RSTC)	0xFFFFFD00)	/* RSTC */
#define AT91C_BASE_PITC		((AT91PS_PITC)	0xFFFFFD30)	/* PITC */
#define AT91C_BASE_US0		((unsigned long)0xFFF8C000)	/* US0 */
#define AT91C_BASE_US1		((unsigned long)0xFFF90000)	/* US1 */
#define AT91C_BASE_US2		((unsigned long)0xFFF94000)	/* US2 */
#define AT91C_BASE_SPI0		((AT91PS_SPI)	0xFFFA4000)	/* SPI0 */
#define AT91C_BASE_MACB		((unsigned long)0xFFFBC000)	/* MACB */

#endif

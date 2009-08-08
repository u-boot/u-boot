/*
 * (C) Copyright 2006-2008
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _MEM_H_
#define _MEM_H_

#define CS0		0x0
#define CS1		0x1 /* mirror CS1 regs appear offset 0x30 from CS0 */

#ifndef __ASSEMBLY__
enum {
	STACKED = 0,
	IP_DDR = 1,
	COMBO_DDR = 2,
	IP_SDR = 3,
};
#endif /* __ASSEMBLY__ */

#define EARLY_INIT	1

/* Slower full frequency range default timings for x32 operation*/
#define SDP_SDRC_SHARING	0x00000100
#define SDP_SDRC_MR_0_SDR	0x00000031

/* optimized timings good for current shipping parts */
#define SDP_3430_SDRC_RFR_CTRL_165MHz	0x0004e201 /* 7.8us/6ns - 50=0x4e2 */

#define DLL_OFFSET		0
#define DLL_WRITEDDRCLKX2DIS	1
#define DLL_ENADLL		1
#define DLL_LOCKDLL		0
#define DLL_DLLPHASE_72		0
#define DLL_DLLPHASE_90		1

/* rkw - need to find of 90/72 degree recommendation for speed like before */
#define SDP_SDRC_DLLAB_CTRL	((DLL_ENADLL << 3) | \
				(DLL_LOCKDLL << 2) | (DLL_DLLPHASE_90 << 1))

/* Infineon part of 3430SDP (165MHz optimized) 6.06ns
 *   ACTIMA
 *	TDAL = Twr/Tck + Trp/tck = 15/6 + 18/6 = 2.5 + 3 = 5.5 -> 6
 *	TDPL (Twr) = 15/6	= 2.5 -> 3
 *	TRRD = 12/6	= 2
 *	TRCD = 18/6	= 3
 *	TRP = 18/6	= 3
 *	TRAS = 42/6	= 7
 *	TRC = 60/6	= 10
 *	TRFC = 72/6	= 12
 *   ACTIMB
 *	TCKE = 2
 *	XSR = 120/6 = 20
 */
#define TDAL_165	6
#define TDPL_165	3
#define TRRD_165	2
#define TRCD_165	3
#define TRP_165		3
#define TRAS_165	7
#define TRC_165		10
#define TRFC_165	21
#define V_ACTIMA_165	((TRFC_165 << 27) | (TRC_165 << 22) | \
			(TRAS_165 << 18) | (TRP_165 << 15) |  \
			(TRCD_165 << 12) | (TRRD_165 << 9) |  \
			(TDPL_165 << 6) | (TDAL_165))

#define TWTR_165	1
#define TCKE_165	1
#define TXP_165		5
#define XSR_165		23
#define V_ACTIMB_165	(((TCKE_165 << 12) | (XSR_165 << 0)) |	\
			(TXP_165 << 8) | (TWTR_165 << 16))

#define SDP_SDRC_ACTIM_CTRLA_0	V_ACTIMA_165
#define SDP_SDRC_ACTIM_CTRLB_0	V_ACTIMB_165
#define SDP_SDRC_RFR_CTRL	SDP_3430_SDRC_RFR_CTRL_165MHz

/*
 * GPMC settings -
 * Definitions is as per the following format
 * #define <PART>_GPMC_CONFIG<x> <value>
 * Where:
 * PART is the part name e.g. STNOR - Intel Strata Flash
 * x is GPMC config registers from 1 to 6 (there will be 6 macros)
 * Value is corresponding value
 *
 * For every valid PRCM configuration there should be only one definition of
 * the same. if values are independent of the board, this definition will be
 * present in this file if values are dependent on the board, then this should
 * go into corresponding mem-boardName.h file
 *
 * Currently valid part Names are (PART):
 * STNOR - Intel Strata Flash
 * SMNAND - Samsung NAND
 * MPDB - H4 MPDB board
 * SBNOR - Sibley NOR
 * MNAND - Micron Large page x16 NAND
 * ONNAND - Samsung One NAND
 *
 * include/configs/file.h contains the defn - for all CS we are interested
 * #define OMAP34XX_GPMC_CSx PART
 * #define OMAP34XX_GPMC_CSx_SIZE Size
 * #define OMAP34XX_GPMC_CSx_MAP Map
 * Where:
 * x - CS number
 * PART - Part Name as defined above
 * SIZE - how big is the mapping to be
 *   GPMC_SIZE_128M - 0x8
 *   GPMC_SIZE_64M  - 0xC
 *   GPMC_SIZE_32M  - 0xE
 *   GPMC_SIZE_16M  - 0xF
 * MAP  - Map this CS to which address(GPMC address space)- Absolute address
 *   >>24 before being used.
 */
#define GPMC_SIZE_128M	0x8
#define GPMC_SIZE_64M	0xC
#define GPMC_SIZE_32M	0xE
#define GPMC_SIZE_16M	0xF

#define SMNAND_GPMC_CONFIG1	0x00000800
#define SMNAND_GPMC_CONFIG2	0x00141400
#define SMNAND_GPMC_CONFIG3	0x00141400
#define SMNAND_GPMC_CONFIG4	0x0F010F01
#define SMNAND_GPMC_CONFIG5	0x010C1414
#define SMNAND_GPMC_CONFIG6	0x1F0F0A80
#define SMNAND_GPMC_CONFIG7	0x00000C44

#define M_NAND_GPMC_CONFIG1	0x00001800
#define M_NAND_GPMC_CONFIG2	0x00141400
#define M_NAND_GPMC_CONFIG3	0x00141400
#define M_NAND_GPMC_CONFIG4	0x0F010F01
#define M_NAND_GPMC_CONFIG5	0x010C1414
#define M_NAND_GPMC_CONFIG6	0x1f0f0A80
#define M_NAND_GPMC_CONFIG7	0x00000C44

#define STNOR_GPMC_CONFIG1	0x3
#define STNOR_GPMC_CONFIG2	0x00151501
#define STNOR_GPMC_CONFIG3	0x00060602
#define STNOR_GPMC_CONFIG4	0x11091109
#define STNOR_GPMC_CONFIG5	0x01141F1F
#define STNOR_GPMC_CONFIG6	0x000004c4

#define SIBNOR_GPMC_CONFIG1	0x1200
#define SIBNOR_GPMC_CONFIG2	0x001f1f00
#define SIBNOR_GPMC_CONFIG3	0x00080802
#define SIBNOR_GPMC_CONFIG4	0x1C091C09
#define SIBNOR_GPMC_CONFIG5	0x01131F1F
#define SIBNOR_GPMC_CONFIG6	0x1F0F03C2

#define SDPV2_MPDB_GPMC_CONFIG1	0x00611200
#define SDPV2_MPDB_GPMC_CONFIG2	0x001F1F01
#define SDPV2_MPDB_GPMC_CONFIG3	0x00080803
#define SDPV2_MPDB_GPMC_CONFIG4	0x1D091D09
#define SDPV2_MPDB_GPMC_CONFIG5	0x041D1F1F
#define SDPV2_MPDB_GPMC_CONFIG6	0x1D0904C4

#define MPDB_GPMC_CONFIG1	0x00011000
#define MPDB_GPMC_CONFIG2	0x001f1f01
#define MPDB_GPMC_CONFIG3	0x00080803
#define MPDB_GPMC_CONFIG4	0x1c0b1c0a
#define MPDB_GPMC_CONFIG5	0x041f1F1F
#define MPDB_GPMC_CONFIG6	0x1F0F04C4

#define P2_GPMC_CONFIG1	0x0
#define P2_GPMC_CONFIG2	0x0
#define P2_GPMC_CONFIG3	0x0
#define P2_GPMC_CONFIG4	0x0
#define P2_GPMC_CONFIG5	0x0
#define P2_GPMC_CONFIG6	0x0

#define ONENAND_GPMC_CONFIG1	0x00001200
#define ONENAND_GPMC_CONFIG2	0x000F0F01
#define ONENAND_GPMC_CONFIG3	0x00030301
#define ONENAND_GPMC_CONFIG4	0x0F040F04
#define ONENAND_GPMC_CONFIG5	0x010F1010
#define ONENAND_GPMC_CONFIG6	0x1F060000

#define NET_GPMC_CONFIG1	0x00001000
#define NET_GPMC_CONFIG2	0x001e1e01
#define NET_GPMC_CONFIG3	0x00080300
#define NET_GPMC_CONFIG4	0x1c091c09
#define NET_GPMC_CONFIG5	0x04181f1f
#define NET_GPMC_CONFIG6	0x00000FCF
#define NET_GPMC_CONFIG7	0x00000f6c

/* max number of GPMC Chip Selects */
#define GPMC_MAX_CS	8
/* max number of GPMC regs */
#define GPMC_MAX_REG	7

#define PISMO1_NOR	1
#define PISMO1_NAND	2
#define PISMO2_CS0	3
#define PISMO2_CS1	4
#define PISMO1_ONENAND	5
#define DBG_MPDB	6
#define PISMO2_NAND_CS0 7
#define PISMO2_NAND_CS1 8

/* make it readable for the gpmc_init */
#define PISMO1_NOR_BASE		FLASH_BASE
#define PISMO1_NAND_BASE	NAND_BASE
#define PISMO2_CS0_BASE		PISMO2_MAP1
#define PISMO1_ONEN_BASE	ONENAND_MAP
#define DBG_MPDB_BASE		DEBUG_BASE

#endif /* endif _MEM_H_ */

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
#define SDRC_SHARING	0x00000100
#define SDRC_MR_0_SDR	0x00000031

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
#define INFINEON_TDAL_165	6
#define INFINEON_TDPL_165	3
#define INFINEON_TRRD_165	2
#define INFINEON_TRCD_165	3
#define INFINEON_TRP_165	3
#define INFINEON_TRAS_165	7
#define INFINEON_TRC_165	10
#define INFINEON_TRFC_165	12
#define INFINEON_V_ACTIMA_165	((INFINEON_TRFC_165 << 27) |		\
		(INFINEON_TRC_165 << 22) | (INFINEON_TRAS_165 << 18) |	\
		(INFINEON_TRP_165 << 15) | (INFINEON_TRCD_165 << 12) |	\
		(INFINEON_TRRD_165 << 9) | (INFINEON_TDPL_165 << 6) |	\
		(INFINEON_TDAL_165))

#define INFINEON_TWTR_165	1
#define INFINEON_TCKE_165	2
#define INFINEON_TXP_165	2
#define INFINEON_XSR_165	20
#define INFINEON_V_ACTIMB_165	((INFINEON_TCKE_165 << 12) |		\
		(INFINEON_XSR_165 << 0) | (INFINEON_TXP_165 << 8) |	\
		(INFINEON_TWTR_165 << 16))

/* Micron part of 3430 EVM (165MHz optimized) 6.06ns
 * ACTIMA
 *	TDAL = Twr/Tck + Trp/tck= 15/6 + 18 /6 = 2.5 + 3 = 5.5 -> 6
 *	TDPL (Twr)	= 15/6	= 2.5 -> 3
 *	TRRD		= 12/6	= 2
 *	TRCD		= 18/6	= 3
 *	TRP		= 18/6	= 3
 *	TRAS		= 42/6	= 7
 *	TRC		= 60/6	= 10
 *	TRFC		= 125/6	= 21
 * ACTIMB
 *	TWTR		= 1
 *	TCKE		= 1
 *	TXSR		= 138/6	= 23
 *	TXP		= 25/6	= 4.1 ~5
 */
#define MICRON_TDAL_165		6
#define MICRON_TDPL_165		3
#define MICRON_TRRD_165		2
#define MICRON_TRCD_165		3
#define MICRON_TRP_165		3
#define MICRON_TRAS_165		7
#define MICRON_TRC_165		10
#define MICRON_TRFC_165		21
#define MICRON_V_ACTIMA_165 ((MICRON_TRFC_165 << 27) |			\
		(MICRON_TRC_165 << 22) | (MICRON_TRAS_165 << 18) |	\
		(MICRON_TRP_165 << 15) | (MICRON_TRCD_165 << 12) |	\
		(MICRON_TRRD_165 << 9) | (MICRON_TDPL_165 << 6) |	\
		(MICRON_TDAL_165))

#define MICRON_TWTR_165		1
#define MICRON_TCKE_165		1
#define MICRON_XSR_165		23
#define MICRON_TXP_165		5
#define MICRON_V_ACTIMB_165 ((MICRON_TCKE_165 << 12) |			\
		(MICRON_XSR_165 << 0) | (MICRON_TXP_165 << 8) |	\
		(MICRON_TWTR_165 << 16))

#define MICRON_RAMTYPE			0x1
#define MICRON_DDRTYPE			0x0
#define MICRON_DEEPPD			0x1
#define MICRON_B32NOT16			0x1
#define MICRON_BANKALLOCATION	0x2
#define MICRON_RAMSIZE			((PHYS_SDRAM_1_SIZE/(1024*1024))/2)
#define MICRON_ADDRMUXLEGACY	0x1
#define MICRON_CASWIDTH			0x5
#define MICRON_RASWIDTH			0x2
#define MICRON_LOCKSTATUS		0x0
#define MICRON_V_MCFG ((MICRON_LOCKSTATUS << 30) | (MICRON_RASWIDTH << 24) | \
	(MICRON_CASWIDTH << 20) | (MICRON_ADDRMUXLEGACY << 19) | \
	(MICRON_RAMSIZE << 8) | (MICRON_BANKALLOCATION << 6) | \
	(MICRON_B32NOT16 << 4) | (MICRON_DEEPPD << 3) | \
	(MICRON_DDRTYPE << 2) | (MICRON_RAMTYPE))

#define MICRON_ARCV				2030
#define MICRON_ARE				0x1
#define MICRON_V_RFR_CTRL ((MICRON_ARCV << 8) | (MICRON_ARE))

#define MICRON_BL				0x2
#define MICRON_SIL				0x0
#define MICRON_CASL				0x3
#define MICRON_WBST				0x0
#define MICRON_V_MR ((MICRON_WBST << 9) | (MICRON_CASL << 4) | \
	(MICRON_SIL << 3) | (MICRON_BL))

/*
 * NUMONYX part of IGEP v2 (165MHz optimized) 6.06ns
 *   ACTIMA
 *      TDAL = Twr/Tck + Trp/tck = 15/6 + 18/6 = 2.5 + 3 = 5.5 -> 6
 *      TDPL (Twr) = 15/6 = 2.5 -> 3
 *      TRRD = 12/6 = 2
 *      TRCD = 22.5/6 = 3.75 -> 4
 *      TRP  = 18/6 = 3
 *      TRAS = 42/6 = 7
 *      TRC  = 60/6 = 10
 *      TRFC = 140/6 = 23.3 -> 24
 *   ACTIMB
 *	TWTR = 2
 *	TCKE = 2
 *	TXSR = 200/6 =  33.3 -> 34
 *	TXP  = 1.0 + 1.1 = 2.1 -> 3
 */
#define NUMONYX_TDAL_165   6
#define NUMONYX_TDPL_165   3
#define NUMONYX_TRRD_165   2
#define NUMONYX_TRCD_165   4
#define NUMONYX_TRP_165    3
#define NUMONYX_TRAS_165   7
#define NUMONYX_TRC_165   10
#define NUMONYX_TRFC_165  24
#define NUMONYX_V_ACTIMA_165 ((NUMONYX_TRFC_165 << 27) | \
		(NUMONYX_TRC_165 << 22) | (NUMONYX_TRAS_165 << 18) | \
		(NUMONYX_TRP_165 << 15) | (NUMONYX_TRCD_165 << 12) | \
		(NUMONYX_TRRD_165 << 9) | (NUMONYX_TDPL_165 << 6) | \
		(NUMONYX_TDAL_165))

#define NUMONYX_TWTR_165   2
#define NUMONYX_TCKE_165   2
#define NUMONYX_TXP_165    3
#define NUMONYX_XSR_165    34
#define NUMONYX_V_ACTIMB_165 ((NUMONYX_TCKE_165 << 12) | \
		(NUMONYX_XSR_165 << 0) | (NUMONYX_TXP_165 << 8) | \
		(NUMONYX_TWTR_165 << 16))

#ifdef CONFIG_OMAP3_INFINEON_DDR
#define V_ACTIMA_165 INFINEON_V_ACTIMA_165
#define V_ACTIMB_165 INFINEON_V_ACTIMB_165
#endif

#ifdef CONFIG_OMAP3_MICRON_DDR
#define V_ACTIMA_165 MICRON_V_ACTIMA_165
#define V_ACTIMB_165 MICRON_V_ACTIMB_165
#define V_MCFG			MICRON_V_MCFG
#define V_RFR_CTRL		MICRON_V_RFR_CTRL
#define V_MR			MICRON_V_MR
#endif

#ifdef CONFIG_OMAP3_NUMONYX_DDR
#define V_ACTIMA_165 NUMONYX_V_ACTIMA_165
#define V_ACTIMB_165 NUMONYX_V_ACTIMB_165
#endif

#if !defined(V_ACTIMA_165) || !defined(V_ACTIMB_165)
#error "Please choose the right DDR type in config header"
#endif

#if defined(CONFIG_SPL_BUILD) && (!defined(V_MCFG) || !defined(V_RFR_CTRL))
#error "Please choose the right DDR type in config header"
#endif

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

#ifndef __ASSEMBLY__

/* Function prototypes */
void mem_init(void);

u32 is_mem_sdr(void);
u32 mem_ok(u32 cs);

u32 get_sdr_cs_size(u32);
u32 get_sdr_cs_offset(u32);

#endif	/* __ASSEMBLY__ */

#endif /* endif _MEM_H_ */

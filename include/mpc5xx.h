/*
 * (C) Copyright 2003
 * Martin Winistoerfer, martinwinistoerfer@gmx.ch.
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

/*
 * File:		mpc5xx.h
 *
 * Discription:		mpc5xx specific definitions
 *
 */

#ifndef __MPC5XX_H__
#define __MPC5XX_H__


/*-----------------------------------------------------------------------
 * Exception offsets (PowerPC standard)
 */
#define EXC_OFF_SYS_RESET	0x0100	/* System reset				*/
#define _START_OFFSET		EXC_OFF_SYS_RESET

/*-----------------------------------------------------------------------
 * ISB bit in IMMR to set internal memory map
 */

#define CFG_ISB			((CFG_IMMR / 0x00400000) << 1)

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control Register
 */
#define SYPCR_SWTC	0xffff0000	/* Software Watchdog Timer Count	*/
#define SYPCR_BMT	0x0000ff00	/* Bus Monitor Timing			*/
#define SYPCR_BME	0x00000080	/* Bus Monitor Enable			*/
#define SYPCR_SWF	0x00000008	/* Software Watchdog Freeze		*/
#define SYPCR_SWE	0x00000004	/* Software Watchdog Enable		*/
#define SYPCR_SWRI	0x00000002	/* Software Watchdog Reset/Int Select	*/
#define SYPCR_SWP	0x00000001	/* Software Watchdog Prescale		*/

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration Register
 */
#define SIUMCR_EARB	0x80000000	/* External Arbitration			*/
#define SIUMCR_EARP0	0x00000000	/* External Arbi. Request priority 0	*/
#define SIUMCR_EARP1	0x10000000	/* External Arbi. Request priority 1	*/
#define SIUMCR_EARP2	0x20000000	/* External Arbi. Request priority 2	*/
#define SIUMCR_EARP3	0x30000000	/* External Arbi. Request priority 3	*/
#define SIUMCR_EARP4	0x40000000	/* External Arbi. Request priority 4	*/
#define SIUMCR_EARP5	0x50000000	/* External Arbi. Request priority 5	*/
#define SIUMCR_EARP6	0x60000000	/* External Arbi. Request priority 6	*/
#define SIUMCR_EARP7	0x70000000	/* External Arbi. Request priority 7	*/
#define SIUMCR_DSHW	0x00800000	/* Data Showcycles			*/
#define SIUMCR_DBGC00	0x00000000	/* Debug pins configuration		*/
#define SIUMCR_DBGC01	0x00200000	/* - " -				*/
#define SIUMCR_DBGC10	0x00400000	/* - " -				*/
#define SIUMCR_DBGC11	0x00600000	/* - " -				*/
#define SIUMCR_DBPC00	0x00000000	/* Debug Port pins Config.		*/
#define SIUMCR_DBPC01	0x00080000	/* - " -				*/
#define SIUMCR_DBPC10	0x00100000	/* - " -				*/
#define SIUMCR_DBPC11	0x00180000	/* - " -				*/
#define SIUMCR_GPC00	0x00000000	/* General Pins Config 			*/
#define SIUMCR_GPC01	0x00020000	/* General Pins Config 			*/
#define SIUMCR_GPC10	0x00040000	/* General Pins Config 			*/
#define SIUMCR_GPC11	0x00060000	/* General Pins Config 			*/
#define SIUMCR_DLK	0x00010000	/* Debug Register Lock			*/
#define SIUMCR_SC00	0x00000000	/* Multi Chip 32 bit			*/
#define SIUMCR_SC01	0x00004000	/* Muilt Chip 16 bit			*/
#define SIUMCR_SC10	0x00004000	/* Single adress show			*/
#define SIUMCR_SC11	0x00006000	/* Single adress			*/
#define SIUMCR_RCTX	0x00001000	/* Data Parity pins Config.		*/
#define SIUMCR_MLRC00	0x00000000	/* Multi Level Reserva. Ctrl		*/
#define SIUMCR_MLRC01	0x00000400	/* - " -				*/
#define SIUMCR_MLRC10	0x00000800	/* - " -				*/
#define SIUMCR_MLRC11	0x00000c00	/* - " -				*/
#define SIUMCR_MTSC	0x00000100	/* Memory transfer      		*/

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control Register
 */
#define TBSCR_REFA	((ushort)0x0080)	/* Reference Interrupt Status A	*/
#define TBSCR_REFB	((ushort)0x0040)	/* Reference Interrupt Status B */
#define TBSCR_TBF	((ushort)0x0002)	/* Time Base stops while FREEZE */

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control Register
 */
#define PISCR_PITF	((ushort)0x0002)	/* PIT stops when FREEZE	*/
#define PISCR_PS	0x0080			/* Periodic Interrupt Status	*/

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register
 */
#define PLPRCR_MF_MSK	0xfff00000	/* MF mask				*/
#define PLPRCR_DIVF_MSK	0x0000001f	/* DIVF mask				*/
#define PLPRCR_CSRC_MSK 0x00000400	/* CSRC mask				*/
#define PLPRCR_MF_SHIFT 0x00000014	/* Multiplication factor shift value	*/
#define PLPRCR_DIVF_0   0x00000000	/* Division factor 0			*/
#define PLPRCR_MF_9     0x00900000	/* Mulitipliaction factor 9		*/
#define PLPRCR_TEXPS	0x00004000	/* TEXP Status				*/
#define PLPRCR_TMIST	0x00001000	/* Timers Interrupt Status		*/
#define PLPRCR_CSR	0x00000080	/* CheskStop Reset value		*/
#define PLPRCR_SPLSS	0x00008000	/* SPLL Lock Status Sticky bit		*/

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register
 */
#define SCCR_DFNL_MSK	0x00000070	/* DFNL mask				*/
#define SCCR_DFNH_MSK	0x00000007  	/* DFNH mask				*/
#define SCCR_DFNL_SHIFT 0x0000004	/* DFNL shift value			*/
#define SCCR_RTSEL	0x00100000	/* RTC circuit input source select	*/
#define SCCR_EBDF00	0x00000000	/* Division factor 1. CLKOUT is GCLK2   */
#define SCCR_EBDF11	0x00060000	/* reserved				*/
#define SCCR_TBS	0x02000000	/* Time Base Source			*/
#define SCCR_RTDIV	0x01000000	/* RTC Clock Divide 			*/
#define SCCR_COM00	0x00000000	/* full strength CLKOUT output buffer	*/
#define SCCR_COM01	0x20000000	/* half strength CLKOUT output buffer	*/
#define SCCR_DFNL000	0x00000000	/* Division by 2 (default = minimum)	*/
#define SCCR_DFNH000	0x00000000	/* Division by 1 (default = minimum)	*/

/*-----------------------------------------------------------------------
 * MC - Memory Controller
 */
#define BR_V		0x00000001	/* Bank valid 				*/
#define BR_BI		0x00000002	/* Burst inhibit 			*/
#define BR_PS_8		0x00000400	/* 8 bit port size 			*/
#define BR_PS_16	0x00000800	/* 16 bit port size 			*/
#define BR_PS_32	0x00000000	/* 32 bit port size 			*/
#define BR_LBDIR	0x00000008	/* Late burst data in progess		*/
#define BR_SETA		0x00000004	/* External Data Acknowledge		*/
#define OR_SCY_3	0x00000030	/* 3 clock cycles wait states		*/
#define OR_SCY_1	0x00000000	/* 1 clock cycle wait state		*/
#define OR_SCY_8	0x00000080	/* 8 clock cycles wait states		*/
#define OR_TRLX		0x00000001	/* Timing relaxed			*/
#define OR_BSCY		0x00000060	/* Burst beats length in clocks		*/
#define OR_ACS_10	0x00000600	/* Adress to chip-select setup		*/
#define OR_CSNT		0x00000800	/* Chip-select negotation time		*/
#define OR_ETHR		0x00000100	/* Extended hold time on read		*/
#define OR_ADDR_MK_FF	0xFF000000
#define OR_ADDR_MK_FFFF	0xFFFF0000

/*-----------------------------------------------------------------------
 * UMCR - UIMB Module Configuration Register
 */
#define UMCR_FSPEED 	0x00000000	/* Full speed. Opposit of UMCR_HSPEED	*/
#define UMCR_HSPEED 	0x10000000	/* Half speed				*/

/*-----------------------------------------------------------------------
 * ICTRL - I-Bus Support Control Register
 */
#define ICTRL_ISCT_SER_7 0x00000007	/* All indirect change of flow		*/


#define NR_IRQS		0		/* Place this later in a separate file */

/*-----------------------------------------------------------------------
 * SCI - Serial communication interface
 */

#define SCI_TDRE	0x0100		/* Transmit data register empty 	*/
#define SCI_TE		0x0008		/* Transmitter enabled 			*/
#define SCI_RE		0x0004		/* Receiver enabled			*/
#define SCI_RDRF	0x0040		/* Receive data register full 		*/
#define SCI_PE		0x0400		/* Parity enable 			*/
#define SCI_SCXBR_MK	0x1fff		/* Baudrate mask 			*/
#define SCI_SCXDR_MK	0x00ff		/* Data register mask 			*/
#define SCI_M_11	0x0200		/* Frame size is 11 bit			*/
#define SCI_M_10	0x0000		/* Frame size is 10 bit			*/
#define SCI_PORT_1	((int)1)	/* Place this later somewhere better 	*/
#define SCI_PORT_2	((int)2)

#endif	/* __MPC5XX_H__ */

/*
 * include/mc9328.h
 *
 * (c) Copyright 2004
 * Techware Information Technology, Inc.
 * http://www.techware.com.tw/
 *
 * Ming-Len Wu <minglen_wu@techware.com.tw>
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

#ifndef __MC9328_H__
#define __MC9328_H__

typedef volatile unsigned long	VU32;
typedef VU32	*		P_VU32;

#define __REG(x)		(*((volatile u32 *)(x)))

/*
 * MX1 Chip selects & internal memory's
 */
#define MX1_DMI_PHYS	0x00000000	/* double map image	*/
#define MX1_BROM_PHYS	0x00100000	/* Bootstrape ROM	*/
#define MX1_ESRAM_PHYS	0x00300000	/* Embedded SRAM (128KB)*/

#define MX1_CSD0_PHYS	0x08000000	/* CSD0 64MB (SDRAM)	*/
#define MX1_CSD1_PHYS	0x0C000000	/* CSD1 64MB (SDRAM)	*/
#define MX1_CS0_PHYS	0x10000000	/* CS0 32MB (Flash)	*/
#define MX1_CS1_PHYS	0x12000000	/* CS1 16MB (Flash)	*/
#define MX1_CS2_PHYS	0x13000000	/* CS2 16MB (Ext SRAM)	*/
#define MX1_CS3_PHYS	0x14000000	/* CS3 16MB (Spare)	*/
#define MX1_CS4_PHYS	0x15000000	/* CS4 16MB (Spare)	*/
#define MX1_CS5_PHYS	0x16000000	/* CS5 16MB (Spare)	*/

/*
 *	MX1 Watchdog registers
 */
#define MX1_WCR		__REG(0x00201000)  /* Watchdog Control Register		*/
#define MX1_WSR		__REG(0x00201004)  /* Watchdog Service Register		*/
#define MX1_WSTR	__REG(0x00201008)  /* Watchdog Status Register		*/

/*
 *	MX1 Timer registers
 */
#define MX1_TCTL1	__REG(0x00202000)  /* Timer 1 Control Register		*/
#define MX1_TPRER1	__REG(0x00202004)  /* Timer 1 Prescaler Register	*/
#define MX1_TCMP1	__REG(0x00202008)  /* Timer 1 Compare Register		*/
#define MX1_TCR1	__REG(0x0020200C)  /* Timer 1 Capture Register		*/
#define MX1_TCN1	__REG(0x00202010)  /* Timer 1 Counter Register		*/
#define MX1_TSTAT1	__REG(0x00202014)  /* Timer 1 Status Register		*/

#define MX1_TCTL2	__REG(0x00203000)  /* Timer 2 Control Register		*/
#define MX1_TPRER2	__REG(0x00203004)  /* Timer 2 Prescaler Register	*/
#define MX1_TCMP2	__REG(0x00203008)  /* Timer 2 Compare Register		*/
#define MX1_TCR2	__REG(0x0020300C)  /* Timer 2 Capture Register		*/
#define MX1_TCN2	__REG(0x00203010)  /* Timer 2 Counter Register		*/
#define MX1_TSTAT2	__REG(0x00203014)  /* Timer 2 Status Register		*/

/*
 *	MX1 RTC registers
 */
#define MX1_HOURMIN	__REG(0x00204000)  /* RTC Hour & Min Counter Registers	*/
#define MX1_SECONDS	__REG(0x00204004)  /* RTC Seconds Counter Registers	*/
#define MX1_ALRM_HM	__REG(0x00204008)  /* RTC Hour & Min Alarm Registers	*/
#define MX1_ALRM_SEC	__REG(0x0020400C)  /* RTC Seconds Alarm Registers	*/
#define MX1_RCCTL	__REG(0x00204010)  /* RTC Control Registers		*/
#define MX1_RTCISR	__REG(0x00204014)  /* RTC Interrupt Status Registers	*/
#define MX1_RTCIENR	__REG(0x00204018)  /* RTC Interrupt Enable Registers	*/
#define MX1_STPWCH	__REG(0x0020401C)  /* RTC Stopwatch Minutes Registers	*/
#define MX1_DAYR	__REG(0x00204020)  /* RTC Days Counter Registers	*/
#define MX1_DAYALARM	__REG(0x00204020)  /* RTC Day Alarm Registers		*/

/*
 *	MX1 LCD Controller registers
 */
#define MX1_SSA		__REG(0x00205000)  /* Screen Start Address Register	*/
#define MX1_SIZE	__REG(0x00205004)  /* Size Register			*/
#define MX1_VPW		__REG(0x00205008)  /* Virtual Page Width Register	*/
#define MX1_CPOS	__REG(0x0020500C)  /* LCD Cursor Position  Register	*/
#define MX1_LCWHB	__REG(0x00205010)  /* LCD Cursor Width Height & Blink Register	*/
#define MX1_LCHCC	__REG(0x00205014)  /* LCD Color Cursor Mapping Register */
#define MX1_PCR		__REG(0x00205018)  /* LCD Panel Configuration Register	*/
#define MX1_HCR		__REG(0x0020501C)  /* Horizontal Configuration Register */
#define MX1_VCR		__REG(0x00205020)  /* Vertical Configuration Register	*/
#define MX1_POS		__REG(0x00205024)  /* Panning Offset Register		*/
#define MX1_LGPMR	__REG(0x00205028)  /* LCD Gray Palette Mapping Register */
#define MX1_PWMR	__REG(0x0020502C)  /* PWM Contrast Control Register	*/
#define MX1_DMACR	__REG(0x00205030)  /* DMA Control Register		*/
#define MX1_RMCR	__REG(0x00205034)  /* Refresh Mode Control Register	*/
#define MX1_LCDICR	__REG(0x00205038)  /* Interrupt Configuration Register	*/
#define MX1_LCDISR	__REG(0x00205040)  /* Interrupt Status Register		*/

/*
 *	MX1 UART registers
 */

/* UART 1 */
#define MX1_URX0D_1	__REG(0x00206000)  /* UART 1 Receiver Register 0	*/
#define MX1_URX1D_1	__REG(0x00206004)  /* UART 1 Receiver Register 1	*/
#define MX1_URX2D_1	__REG(0x00206008)  /* UART 1 Receiver Register 2	*/
#define MX1_URX3D_1	__REG(0x0020600C)  /* UART 1 Receiver Register 3	*/
#define MX1_URX4D_1	__REG(0x00206010)  /* UART 1 Receiver Register 4	*/
#define MX1_URX5D_1	__REG(0x00206014)  /* UART 1 Receiver Register 5	*/
#define MX1_URX6D_1	__REG(0x00206018)  /* UART 1 Receiver Register 6	*/
#define MX1_URX7D_1	__REG(0x0020601C)  /* UART 1 Receiver Register 7	*/
#define MX1_URX8D_1	__REG(0x00206020)  /* UART 1 Receiver Register 8	*/
#define MX1_URX9D_1	__REG(0x00206024)  /* UART 1 Receiver Register 9	*/
#define MX1_URX10D_1	__REG(0x00206028)  /* UART 1 Receiver Register 10	*/
#define MX1_URX11D_1	__REG(0x0020602C)  /* UART 1 Receiver Register 11	*/
#define MX1_URX12D_1	__REG(0x00206030)  /* UART 1 Receiver Register 12	*/
#define MX1_URX13D_1	__REG(0x00206034)  /* UART 1 Receiver Register 13	*/
#define MX1_URX14D_1	__REG(0x00206038)  /* UART 1 Receiver Register 14	*/
#define MX1_URX15D_1	__REG(0x0020603c)  /* UART 1 Receiver Register 15	*/

#define MX1_UTX0D_1	__REG(0x00206040)  /* UART 1 Transmitter Register 0	*/
#define MX1_UTX1D_1	__REG(0x00206044)  /* UART 1 Transmitter Register 1	*/
#define MX1_UTX2D_1	__REG(0x00206048)  /* UART 1 Transmitter Register 2	*/
#define MX1_UTX3D_1	__REG(0x0020604C)  /* UART 1 Transmitter Register 3	*/
#define MX1_UTX4D_1	__REG(0x00206050)  /* UART 1 Transmitter Register 4	*/
#define MX1_UTX5D_1	__REG(0x00206054)  /* UART 1 Transmitter Register 5	*/
#define MX1_UTX6D_1	__REG(0x00206058)  /* UART 1 Transmitter Register 6	*/
#define MX1_UTX7D_1	__REG(0x0020605C)  /* UART 1 Transmitter Register 7	*/
#define MX1_UTX8D_1	__REG(0x00206060)  /* UART 1 Transmitter Register 8	*/
#define MX1_UTX9D_1	__REG(0x00206064)  /* UART 1 Transmitter Register 9	*/
#define MX1_UTX10D_1	__REG(0x00206068)  /* UART 1 Transmitter Register 10	*/
#define MX1_UTX11D_1	__REG(0x0020606C)  /* UART 1 Transmitter Register 11	*/
#define MX1_UTX12D_1	__REG(0x00206060)  /* UART 1 Transmitter Register 12	*/
#define MX1_UTX13D_1	__REG(0x00206074)  /* UART 1 Transmitter Register 13	*/
#define MX1_UTX14D_1	__REG(0x00206078)  /* UART 1 Transmitter Register 14	*/
#define MX1_UTX15D_1	__REG(0x0020607c)  /* UART 1 Transmitter Register 15	*/

#define MX1_UCR1_1	__REG(0x00206080)  /* UART 1 Control Register 1		*/
#define MX1_UCR2_1	__REG(0x00206084)  /* UART 1 Control Register 2		*/
#define MX1_UCR3_1	__REG(0x00206088)  /* UART 1 Control Register 3		*/
#define MX1_UCR4_1	__REG(0x0020608C)  /* UART 1 Control Register 4		*/
#define MX1_UFCR_1	__REG(0x00206090)  /* UART 1 FIFO Control Register	*/
#define MX1_USR1_1	__REG(0x00206094)  /* UART 1 Status  Register 1		*/
#define MX1_USR2_1	__REG(0x00206098)  /* UART 1 Status  Register 2		*/
#define MX1_UESC_1	__REG(0x0020609C)  /* UART 1 Escape Character Register	*/
#define MX1_UTIM_1	__REG(0x002060A0)  /* UART 1 Escape Timer Register	*/
#define MX1_UBIR_1	__REG(0x002060A4)  /* UART 1 BRM Incremental Register	*/
#define MX1_UBMR_1	__REG(0x002060A8)  /* UART 1 BRM Modulator Register	*/
#define MX1_UBRC_1	__REG(0x002060AC)  /* UART 1 Baud Rate Count Register	*/
#define MX1_BIPR1_1	__REG(0x002060B0)  /* UART 1 BRM Incremental Preset Register 1	*/
#define MX1_BIPR2_1	__REG(0x002060B4)  /* UART 1 BRM Incremental Preset Register 2	*/
#define MX1_BIPR3_1	__REG(0x002060B8)  /* UART 1 BRM Incremental Preset Register 3	*/
#define MX1_BIPR4_1	__REG(0x002060BC)  /* UART 1 BRM Incremental Preset Register 4	*/
#define MX1_BMPR1_1	__REG(0x002060C0)  /* UART 1 BRM Modulator Preset Register 1	*/
#define MX1_BMPR2_1	__REG(0x002060C4)  /* UART 1 BRM Modulator Preset Register 2	*/
#define MX1_BMPR3_1	__REG(0x002060C8)  /* UART 1 BRM Modulator Preset Register 3	*/
#define MX1_BMPR4_1	__REG(0x002060CC)  /* UART 1 BRM Modulator Preset Register 4	*/
#define MX1_UTS_1	__REG(0x002060D0)  /* UART 1 Test Register 1		*/

/* UART 2 */
#define MX1_URX0D_2	__REG(0x00207000)  /* UART 2 Receiver Register 0	*/
#define MX1_URX1D_2	__REG(0x00207004)  /* UART 2 Receiver Register 1	*/
#define MX1_URX2D_2	__REG(0x00207008)  /* UART 2 Receiver Register 2	*/
#define MX1_URX3D_2	__REG(0x0020700C)  /* UART 2 Receiver Register 3	*/
#define MX1_URX4D_2	__REG(0x00207010)  /* UART 2 Receiver Register 4	*/
#define MX1_URX5D_2	__REG(0x00207014)  /* UART 2 Receiver Register 5	*/
#define MX1_URX6D_2	__REG(0x00207018)  /* UART 2 Receiver Register 6	*/
#define MX1_URX7D_2	__REG(0x0020701C)  /* UART 2 Receiver Register 7	*/
#define MX1_URX8D_2	__REG(0x00207020)  /* UART 2 Receiver Register 8	*/
#define MX1_URX9D_2	__REG(0x00207024)  /* UART 2 Receiver Register 9	*/
#define MX1_URX10D_2	__REG(0x00207028)  /* UART 2 Receiver Register 10	*/
#define MX1_URX11D_2	__REG(0x0020702C)  /* UART 2 Receiver Register 11	*/
#define MX1_URX12D_2	__REG(0x00207030)  /* UART 2 Receiver Register 12	*/
#define MX1_URX13D_2	__REG(0x00207034)  /* UART 2 Receiver Register 13	*/
#define MX1_URX14D_2	__REG(0x00207038)  /* UART 2 Receiver Register 14	*/
#define MX1_URX15D_2	__REG(0x0020703c)  /* UART 2 Receiver Register 15	*/

#define MX1_UTX0D_2	__REG(0x00207040)  /* UART 2 Transmitter Register 0	*/
#define MX1_UTX1D_2	__REG(0x00207044)  /* UART 2 Transmitter Register 1	*/
#define MX1_UTX2D_2	__REG(0x00207048)  /* UART 2 Transmitter Register 2	*/
#define MX1_UTX3D_2	__REG(0x0020704C)  /* UART 2 Transmitter Register 3	*/
#define MX1_UTX4D_2	__REG(0x00207050)  /* UART 2 Transmitter Register 4	*/
#define MX1_UTX5D_2	__REG(0x00207054)  /* UART 2 Transmitter Register 5	*/
#define MX1_UTX6D_2	__REG(0x00207058)  /* UART 2 Transmitter Register 6	*/
#define MX1_UTX7D_2	__REG(0x0020705C)  /* UART 2 Transmitter Register 7	*/
#define MX1_UTX8D_2	__REG(0x00207060)  /* UART 2 Transmitter Register 8	*/
#define MX1_UTX9D_2	__REG(0x00207064)  /* UART 2 Transmitter Register 9	*/
#define MX1_UTX10D_2	__REG(0x00207068)  /* UART 2 Transmitter Register 10	*/
#define MX1_UTX11D_2	__REG(0x0020706C)  /* UART 2 Transmitter Register 11	*/
#define MX1_UTX12D_2	__REG(0x00207060)  /* UART 2 Transmitter Register 12	*/
#define MX1_UTX13D_2	__REG(0x00207074)  /* UART 2 Transmitter Register 13	*/
#define MX1_UTX14D_2	__REG(0x00207078)  /* UART 2 Transmitter Register 14	*/
#define MX1_UTX15D_2	__REG(0x0020707c)  /* UART 2 Transmitter Register 15	*/

#define MX1_UCR1_2	__REG(0x00207080)  /* UART 2 Control Register 1		*/
#define MX1_UCR2_2	__REG(0x00207084)  /* UART 2 Control Register 2		*/
#define MX1_UCR3_2	__REG(0x00207088)  /* UART 2 Control Register 3		*/
#define MX1_UCR4_2	__REG(0x0020708C)  /* UART 2 Control Register 4		*/
#define MX1_UFCR_2	__REG(0x00207090)  /* UART 2 FIFO Control Register	*/
#define MX1_USR1_2	__REG(0x00207094)  /* UART 2 Status  Register 1		*/
#define MX1_USR2_2	__REG(0x00207098)  /* UART 2 Status  Register 2		*/
#define MX1_UESC_2	__REG(0x0020709C)  /* UART 2 Escape Character Register	*/
#define MX1_UTIM_2	__REG(0x002070A0)  /* UART 2 Escape Timer Register	*/
#define MX1_UBIR_2	__REG(0x002070A4)  /* UART 2 BRM Incremental Register	*/
#define MX1_UBMR_2	__REG(0x002070A8)  /* UART 2 BRM Modulator Register	*/
#define MX1_UBRC_2	__REG(0x002070AC)  /* UART 2 Baud Rate Count Register	*/
#define MX1_BIPR1_2	__REG(0x002070B0)  /* UART 2 BRM Incremental Preset Register 1	*/
#define MX1_BIPR2_2	__REG(0x002070B4)  /* UART 2 BRM Incremental Preset Register 2	*/
#define MX1_BIPR3_2	__REG(0x002070B8)  /* UART 2 BRM Incremental Preset Register 3	*/
#define MX1_BIPR4_2	__REG(0x002070BC)  /* UART 2 BRM Incremental Preset Register 4	*/
#define MX1_BMPR1_2	__REG(0x002070C0)  /* UART 2 BRM Modulator Preset Register 1	*/
#define MX1_BMPR2_2	__REG(0x002070C4)  /* UART 2 BRM Modulator Preset Register 2	*/
#define MX1_BMPR3_2	__REG(0x002070C8)  /* UART 2 BRM Modulator Preset Register 3	*/
#define MX1_BMPR4_2	__REG(0x002070CC)  /* UART 2 BRM Modulator Preset Register 4	*/
#define MX1_UTS_2	__REG(0x002070D0)  /* UART 2 Test Register 1		*/

/*
 *	MX1 PWM registers
 */
#define MX1_PWMC	__REG(0x00208000)  /* PWM Control Register		*/
#define MX1_PWMS	__REG(0x00208004)  /* PWM Sample Register		*/
#define MX1_PWMP	__REG(0x00208008)  /* PWM Period Register		*/
#define MX1_PWMCNT	__REG(0x0020800C)  /* PWM Counter Register		*/

/*
 *	MX1 DMAC registers
 */
#define MX1_DCR		__REG(0x00209000)  /* DMA Control Register		*/
#define MX1_DISR	__REG(0x00209004)  /* DMA Interrupt Status Register	*/
#define MX1_DIMR	__REG(0x00209008)  /* DMA Interrupt Mask Register	*/
#define MX1_DBTOSR	__REG(0x0020900C)  /* DMA Burst Time-Out Status Register	*/
#define MX1_DRTOSR	__REG(0x00209010)  /* DMA Request Time-Out Status Register	*/
#define MX1_DSESR	__REG(0x00209014)  /* DMA Request Time-Out Status Register	*/
#define MX1_DBOSR	__REG(0x00209018)  /* DMA Buffer Overflow Status Register	*/
#define MX1_DBTOCR	__REG(0x0020901C)  /* DMA Burst Time-Out Control Register	*/

#define MX1_WSRA	__REG(0x00209040)  /* DMA W-Size Register A		*/
#define MX1_XSRA	__REG(0x00209044)  /* DMA X-Size Register A		*/
#define MX1_YSRA	__REG(0x00209048)  /* DMA Y-Size Register A		*/

#define MX1_WSRB	__REG(0x0020904C)  /* DMA W-Size Register B		*/
#define MX1_XSRB	__REG(0x00209050)  /* DMA X-Size Register B		*/
#define MX1_YSRB	__REG(0x00209054)  /* DMA Y-Size Register B		*/

/* Channel 0 */

#define MX1_SAR0	__REG(0x00209080)  /* Channel 0 Source Address Register */
#define MX1_DAR0	__REG(0x00209084)  /* Channel 0 Destination Address Register	*/
#define MX1_CNTR0	__REG(0x00209088)  /* Channel 0 Count Register		*/
#define MX1_CCR0	__REG(0x0020908C)  /* Channel 0 Control Register	*/
#define MX1_RSSR0	__REG(0x00209090)  /* Channel 0 Request Source Select Register	*/
#define MX1_BLR0	__REG(0x00209094)  /* Channel 0 Burst Length  Register	*/
#define MX1_RTOR0	__REG(0x00209098)  /* Channel 0 Request Time-Out Register	*/
#define MX1_BUCR0	__REG(0x00209098)  /* Channel 0 Bus Utilization Control Register	*/

/* Channel 1 */

#define MX1_SAR1	__REG(0x002090C0)  /* Channel 1 Source Address Register */
#define MX1_DAR1	__REG(0x002090C4)  /* Channel 1 Destination Address Register	*/
#define MX1_CNTR1	__REG(0x002090C8)  /* Channel 1 Count Register		*/
#define MX1_CCR1	__REG(0x002090CC)  /* Channel 1 Control Register	*/
#define MX1_RSSR1	__REG(0x002090D0)  /* Channel 1 Request Source Select Register	*/
#define MX1_BLR1	__REG(0x002090D4)  /* Channel 1 Burst Length  Register	*/
#define MX1_RTOR1	__REG(0x002090D8)  /* Channel 1 Request Time-Out Register	*/
#define MX1_BUCR1	__REG(0x002090D8)  /* Channel 1 Bus Utilization Control Register	*/

/* Channel 2 */

#define MX1_SAR2	__REG(0x00209100)  /* Channel 2 Source Address Register */
#define MX1_DAR2	__REG(0x00209104)  /* Channel 2 Destination Address Register	*/
#define MX1_CNTR2	__REG(0x00209108)  /* Channel 2 Count Register		*/
#define MX1_CCR2	__REG(0x0020910C)  /* Channel 2 Control Register	*/
#define MX1_RSSR2	__REG(0x00209110)  /* Channel 2 Request Source Select Register	*/
#define MX1_BLR2	__REG(0x00209114)  /* Channel 2 Burst Length  Register	*/
#define MX1_RTOR2	__REG(0x00209118)  /* Channel 2 Request Time-Out Register	*/
#define MX1_BUCR2	__REG(0x00209118)  /* Channel 2 Bus Utilization Control Register	*/

/* Channel 3 */

#define MX1_SAR3	__REG(0x00209140)  /* Channel 3 Source Address Register */
#define MX1_DAR3	__REG(0x00209144)  /* Channel 3 Destination Address Register	*/
#define MX1_CNTR3	__REG(0x00209148)  /* Channel 3 Count Register		*/
#define MX1_CCR3	__REG(0x0020914C)  /* Channel 3 Control Register	*/
#define MX1_RSSR3	__REG(0x00209150)  /* Channel 3 Request Source Select Register	*/
#define MX1_BLR3	__REG(0x00209154)  /* Channel 3 Burst Length  Register	*/
#define MX1_RTOR3	__REG(0x00209158)  /* Channel 3 Request Time-Out Register	*/
#define MX1_BUCR3	__REG(0x00209158)  /* Channel 3 Bus Utilization Control Register	*/

/* Channel 4 */

#define MX1_SAR4	__REG(0x00209180)  /* Channel 4 Source Address Register */
#define MX1_DAR4	__REG(0x00209184)  /* Channel 4 Destination Address Register	*/
#define MX1_CNTR4	__REG(0x00209188)  /* Channel 4 Count Register		*/
#define MX1_CCR4	__REG(0x0020918C)  /* Channel 4 Control Register	*/
#define MX1_RSSR4	__REG(0x00209190)  /* Channel 4 Request Source Select Register	*/
#define MX1_BLR4	__REG(0x00209194)  /* Channel 4 Burst Length  Register	*/
#define MX1_RTOR4	__REG(0x00209198)  /* Channel 4 Request Time-Out Register	*/
#define MX1_BUCR4	__REG(0x00209198)  /* Channel 4 Bus Utilization Control Register	*/

/* Channel 5 */

#define MX1_SAR5	__REG(0x002091C0)  /* Channel 5 Source Address Register */
#define MX1_DAR5	__REG(0x002091C4)  /* Channel 5 Destination Address Register	*/
#define MX1_CNTR5	__REG(0x002091C8)  /* Channel 5 Count Register		*/
#define MX1_CCR5	__REG(0x002091CC)  /* Channel 5 Control Register	*/
#define MX1_RSSR5	__REG(0x002091D0)  /* Channel 5 Request Source Select Register	*/
#define MX1_BLR5	__REG(0x002091D4)  /* Channel 5 Burst Length  Register	*/
#define MX1_RTOR5	__REG(0x002091D8)  /* Channel 5 Request Time-Out Register	*/
#define MX1_BUCR5	__REG(0x002091D8)  /* Channel 5 Bus Utilization Control Register	*/

/* Channel 6 */

#define MX1_SAR6	__REG(0x00209200)  /* Channel 6 Source Address Register */
#define MX1_DAR6	__REG(0x00209204)  /* Channel 6 Destination Address Register	*/
#define MX1_CNTR6	__REG(0x00209208)  /* Channel 6 Count Register		*/
#define MX1_CCR6	__REG(0x0020920C)  /* Channel 6 Control Register	*/
#define MX1_RSSR6	__REG(0x00209210)  /* Channel 6 Request Source Select Register	*/
#define MX1_BLR6	__REG(0x00209214)  /* Channel 6 Burst Length  Register	*/
#define MX1_RTOR6	__REG(0x00209218)  /* Channel 6 Request Time-Out Register	*/
#define MX1_BUCR6	__REG(0x00209218)  /* Channel 6 Bus Utilization Control Register	*/

/* Channel 7 */

#define MX1_SAR7	__REG(0x00209240)  /* Channel 7 Source Address Register */
#define MX1_DAR7	__REG(0x00209244)  /* Channel 7 Destination Address Register	*/
#define MX1_CNTR7	__REG(0x00209248)  /* Channel 7 Count Register		*/
#define MX1_CCR7	__REG(0x0020924C)  /* Channel 7 Control Register	*/
#define MX1_RSSR7	__REG(0x00209250)  /* Channel 7 Request Source Select Register	*/
#define MX1_BLR7	__REG(0x00209254)  /* Channel 7 Burst Length  Register	*/
#define MX1_RTOR7	__REG(0x00209258)  /* Channel 7 Request Time-Out Register	*/
#define MX1_BUCR7	__REG(0x00209258)  /* Channel 7 Bus Utilization Control Register	*/

/* Channel 8 */

#define MX1_SAR8	__REG(0x00209280)  /* Channel 8 Source Address Register */
#define MX1_DAR8	__REG(0x00209284)  /* Channel 8 Destination Address Register	*/
#define MX1_CNTR8	__REG(0x00209288)  /* Channel 8 Count Register		*/
#define MX1_CCR8	__REG(0x0020928C)  /* Channel 8 Control Register	*/
#define MX1_RSSR8	__REG(0x00209290)  /* Channel 8 Request Source Select Register	*/
#define MX1_BLR8	__REG(0x00209294)  /* Channel 8 Burst Length  Register	*/
#define MX1_RTOR8	__REG(0x00209298)  /* Channel 8 Request Time-Out Register	*/
#define MX1_BUCR8	__REG(0x00209298)  /* Channel 8 Bus Utilization Control Register	*/

/* Channel 9 */

#define MX1_SAR9	__REG(0x002092C0)  /* Channel 9 Source Address Register */
#define MX1_DAR9	__REG(0x002092C4)  /* Channel 9 Destination Address Register	*/
#define MX1_CNTR9	__REG(0x002092C8)  /* Channel 9 Count Register		*/
#define MX1_CCR9	__REG(0x002092CC)  /* Channel 9 Control Register	*/
#define MX1_RSSR9	__REG(0x002092D0)  /* Channel 9 Request Source Select Register	*/
#define MX1_BLR9	__REG(0x002092D4)  /* Channel 9 Burst Length  Register	*/
#define MX1_RTOR9	__REG(0x002092D8)  /* Channel 9 Request Time-Out Register	*/
#define MX1_BUCR9	__REG(0x002092D8)  /* Channel 9 Bus Utilization Control Register	*/

/* Channel 10 */

#define MX1_SAR10	__REG(0x00209300)  /* Channel 10 Source Address Register */
#define MX1_DAR10	__REG(0x00209304)  /* Channel 10 Destination Address Register	*/
#define MX1_CNTR10	__REG(0x00209308)  /* Channel 10 Count Register			*/
#define MX1_CCR10	__REG(0x0020930C)  /* Channel 10 Control Register	*/
#define MX1_RSSR10	__REG(0x00209310)  /* Channel 10 Request Source Select Register		*/
#define MX1_BLR10	__REG(0x00209314)  /* Channel 10 Burst Length  Register		*/
#define MX1_RTOR10	__REG(0x00209318)  /* Channel 10 Request Time-Out Register	*/
#define MX1_BUCR10	__REG(0x00209318)  /* Channel 10 Bus Utilization Control Register	*/

#define MX1_TCR		__REG(0x00209340)  /* Test Control Register		*/
#define MX1_TFIFOAR	__REG(0x00209344)  /* Test FIFO A  Register		*/
#define MX1_TDRR	__REG(0x00209348)  /* Test DMA Request Register		*/
#define MX1_TDIPR	__REG(0x0020934C)  /* Test DMA In Progress Register	*/
#define MX1_TFIFOBR	__REG(0x00209350)  /* Test FIFO B Register		*/

/*
 *	MX1 SIM registers
 */

#define MX1_PORT_CNTL	__REG(0x00211000)  /* Port Control Register		*/
#define MX1_CNTL	__REG(0x00211004)  /* Control Register			*/
#define MX1_RCV_THRESHOLD __REG(0x00211008)/* Receive Threshold	 Register	*/
#define MX1_ENABLE	__REG(0x0021100C)  /* Transmit/Receive Enable Register	*/
#define MX1_XMT_STATUS	__REG(0x00211010)  /* Transmit Status  Register		*/
#define MX1_RCV_STATUS	__REG(0x00211014)  /* Receive Status  Register		*/
#define MX1_SIM_INT_MASK	__REG(0x00211018)  /* Interrupt Mask Register		*/
#define MX1_XMT_BUF	__REG(0x0021101C)  /* Port Transmit Buffer Register	*/
#define MX1_RCV_BUF	__REG(0x00211020)  /* Receive Buffer Register		*/
#define MX1_PORT_DETECT __REG(0x00211024)  /* Detect Register			*/
#define MX1_XMT_THRESHOLD __REG(0x00211028)/* Transmit Threshold Register	*/
#define MX1_GUARD_CNTL	__REG(0x0021102C)  /* Transmit Guard Control  Register	*/
#define MX1_OD_CONFIG	__REG(0x00211030)  /* Open-Drain Configuration Control Register */
#define MX1_RESET_CNTL	__REG(0x00211034)  /* Reset  Control Register		*/
#define MX1_CHAR_WAIT	__REG(0x00211038)  /* Charactor Wait Timer Register	*/
#define MX1_GPCNT	__REG(0x0021103C)  /* General Purpose Counter  Register */
#define MX1_DIVISOR	__REG(0x00211040)  /* Divisor Register			*/

/*
 *	MX1 USBD registers
 */

#define MX1_USB_FRAME	__REG(0x00212000)  /* USB Frame Number and Match Register	*/
#define MX1_USB_SPEC	__REG(0x00212004)  /* USB Spec & Release Number Register	*/
#define MX1_USB_STAT	__REG(0x00212008)  /* USB Status Register		*/
#define MX1_USB_CTRL	__REG(0x0021200C)  /* USB Control Register		*/
#define MX1_USB_DADR	__REG(0x00212010)  /* USB Descriptor RAM Address Register	*/
#define MX1_USB_DDAT	__REG(0x00212014)  /* USB Descriptor RAM/Endpoint buffer Data  Register */
#define MX1_USB_INTR	__REG(0x00212018)  /* USB Interrupt Status Register	*/
#define MX1_USB_MASK	__REG(0x0021201C)  /* USB Interrupt Mask Register	*/
#define MX1_USB_ENAB	__REG(0x00212024)  /* USB Enable Register		*/

/* Endpoint 0  */
#define MX1_USB_EP0_STAT __REG(0x00212030) /* Endpoint 0 Status/Control Register	*/
#define MX1_USB_EP0_INTR __REG(0x00212034) /* Endpoint 0 Interrupt Status  Register	*/
#define MX1_USB_EP0_MASK __REG(0x00212038) /* Endpoint 0 Interrupt Mask	 Register	*/
#define MX1_USB_EP0_FDAT __REG(0x0021203C) /* Endpoint 0 FIFO Data Register	*/
#define MX1_USB_EP0_FSTAT __REG(0x00212040) /* Endpoint 0 FIFO Status Register	*/
#define MX1_USB_EP0_FCTRL __REG(0x00212044) /* Endpoint 0 FIFO Control Register */
#define MX1_USB_EP0_LRFP __REG(0x00212048) /* Endpoint 0 Last Read Frame Pointer Register	*/
#define MX1_USB_EP0_LWFP __REG(0x0021204C) /* Endpoint 0 Last Write Frame Pointer Register	*/
#define MX1_USB_EP0_FALRM __REG(0x00212050) /* Endpoint 0 FIFO Alarm  Register	*/
#define MX1_USB_EP0_FRDP __REG(0x00212054) /* Endpoint 0 FIFO Read Pointer Register	*/
#define MX1_USB_EP0_FWRP __REG(0x00212058) /* Endpoint 0 FIFO Write Pointer Register	*/

/* Endpoint 1  */
#define MX1_USB_EP1_STAT __REG(0x00212060) /* Endpoint 1 Status/Control Register	*/
#define MX1_USB_EP1_INTR __REG(0x00212064) /* Endpoint 1 Interrupt Status  Register	*/
#define MX1_USB_EP1_MASK __REG(0x00212068) /* Endpoint 1 Interrupt Mask	 Register	*/
#define MX1_USB_EP1_FDAT __REG(0x0021206C) /* Endpoint 1 FIFO Data Register	*/
#define MX1_USB_EP1_FSTAT __REG(0x00212070) /* Endpoint 1 FIFO Status Register	*/
#define MX1_USB_EP1_FCTRL __REG(0x00212074) /* Endpoint 1 FIFO Control Register */
#define MX1_USB_EP1_LRFP __REG(0x00212078) /* Endpoint 1 Last Read Frame Pointer Register	*/
#define MX1_USB_EP1_LWFP __REG(0x0021207C) /* Endpoint 1 Last Write Frame Pointer Register	*/
#define MX1_USB_EP1_FALRM __REG(0x00212080) /* Endpoint 1 FIFO Alarm  Register	*/
#define MX1_USB_EP1_FRDP __REG(0x00212084) /* Endpoint 1 FIFO Read Pointer Register	*/
#define MX1_USB_EP1_FWRP __REG(0x00212088) /* Endpoint 1 FIFO Write Pointer Register	*/

/* Endpoint 2  */
#define MX1_USB_EP2_STAT __REG(0x00212090) /* Endpoint 2 Status/Control Register	*/
#define MX1_USB_EP2_INTR __REG(0x00212094) /* Endpoint 2 Interrupt Status  Register	*/
#define MX1_USB_EP2_MASK __REG(0x00212098) /* Endpoint 2 Interrupt Mask	 Register	*/
#define MX1_USB_EP2_FDAT __REG(0x0021209C) /* Endpoint 2 FIFO Data Register	*/
#define MX1_USB_EP2_FSTAT __REG(0x002120A0) /* Endpoint 2 FIFO Status Register	*/
#define MX1_USB_EP2_FCTRL __REG(0x002120A4) /* Endpoint 2 FIFO Control Register */
#define MX1_USB_EP2_LRFP __REG(0x002120A8) /* Endpoint 2 Last Read Frame Pointer Register	*/
#define MX1_USB_EP2_LWFP __REG(0x002120AC) /* Endpoint 2 Last Write Frame Pointer Register	*/
#define MX1_USB_EP2_FALRM __REG(0x002120B0) /* Endpoint 2 FIFO Alarm  Register	*/
#define MX1_USB_EP2_FRDP __REG(0x002120B4) /* Endpoint 2 FIFO Read Pointer Register	*/
#define MX1_USB_EP2_FWRP __REG(0x002120B8) /* Endpoint 2 FIFO Write Pointer Register	*/

/* Endpoint 3  */
#define MX1_USB_EP3_STAT __REG(0x002120C0) /* Endpoint 3 Status/Control Register	*/
#define MX1_USB_EP3_INTR __REG(0x002120C4) /* Endpoint 3 Interrupt Status  Register	*/
#define MX1_USB_EP3_MASK __REG(0x002120C8) /* Endpoint 3 Interrupt Mask	 Register	*/
#define MX1_USB_EP3_FDAT __REG(0x002120CC) /* Endpoint 3 FIFO Data Register	*/
#define MX1_USB_EP3_FSTAT __REG(0x002120D0) /* Endpoint 3 FIFO Status Register	*/
#define MX1_USB_EP3_FCTRL __REG(0x002120D4) /* Endpoint 3 FIFO Control Register */
#define MX1_USB_EP3_LRFP __REG(0x002120D8) /* Endpoint 3 Last Read Frame Pointer Register	*/
#define MX1_USB_EP3_LWFP __REG(0x002120DC) /* Endpoint 3 Last Write Frame Pointer Register	*/
#define MX1_USB_EP3_FALRM __REG(0x002120E0) /* Endpoint 3 FIFO Alarm  Register	*/
#define MX1_USB_EP3_FRDP __REG(0x002120E4) /* Endpoint 3 FIFO Read Pointer Register	*/
#define MX1_USB_EP3_FWRP __REG(0x002120E8) /* Endpoint 3 FIFO Write Pointer Register	*/

/* Endpoint 4  */
#define MX1_USB_EP4_STAT __REG(0x002120F0) /* Endpoint 4 Status/Control Register	*/
#define MX1_USB_EP4_INTR __REG(0x002120F4) /* Endpoint 4 Interrupt Status  Register	*/
#define MX1_USB_EP4_MASK __REG(0x002120F8) /* Endpoint 4 Interrupt Mask	 Register	*/
#define MX1_USB_EP4_FDAT __REG(0x002120FC) /* Endpoint 4 FIFO Data Register	*/
#define MX1_USB_EP4_FSTAT __REG(0x00212100) /* Endpoint 4 FIFO Status Register	*/
#define MX1_USB_EP4_FCTRL __REG(0x00212104) /* Endpoint 4 FIFO Control Register */
#define MX1_USB_EP4_LRFP __REG(0x00212108) /* Endpoint 4 Last Read Frame Pointer Register	*/
#define MX1_USB_EP4_LWFP __REG(0x0021210C) /* Endpoint 4 Last Write Frame Pointer Register	*/
#define MX1_USB_EP4_FALRM __REG(0x00212110) /* Endpoint 4 FIFO Alarm  Register	*/
#define MX1_USB_EP4_FRDP __REG(0x00212114) /* Endpoint 4 FIFO Read Pointer Register	*/
#define MX1_USB_EP4_FWRP __REG(0x00212118) /* Endpoint 4 FIFO Write Pointer Register	*/

/* Endpoint 5  */
#define MX1_USB_EP5_STAT __REG(0x00212120) /* Endpoint 5 Status/Control Register	*/
#define MX1_USB_EP5_INTR __REG(0x00212124) /* Endpoint 5 Interrupt Status  Register	*/
#define MX1_USB_EP5_MASK __REG(0x00212128) /* Endpoint 5 Interrupt Mask	 Register	*/
#define MX1_USB_EP5_FDAT __REG(0x0021212C) /* Endpoint 5 FIFO Data Register	*/
#define MX1_USB_EP5_FSTAT __REG(0x00212130) /* Endpoint 5 FIFO Status Register	*/
#define MX1_USB_EP5_FCTRL __REG(0x00212134) /* Endpoint 5 FIFO Control Register */
#define MX1_USB_EP5_LRFP __REG(0x00212138) /* Endpoint 5 Last Read Frame Pointer Register	*/
#define MX1_USB_EP5_LWFP __REG(0x0021213C) /* Endpoint 5 Last Write Frame Pointer Register	*/
#define MX1_USB_EP5_FALRM __REG(0x00212140) /* Endpoint 5 FIFO Alarm  Register	*/
#define MX1_USB_EP5_FRDP __REG(0x00212144) /* Endpoint 5 FIFO Read Pointer Register	*/
#define MX1_USB_EP5_FWRP __REG(0x00212148) /* Endpoint 5 FIFO Write Pointer Register	*/

/*
 *	MX1 SPI 1 registers
 */
#define MX1_RXDATAREG1	__REG(0x00213000)  /* SPI 1 Rx Data Register		*/
#define MX1_TXDATAREG1	__REG(0x00213004)  /* SPI 1 Tx Data Register		*/
#define MX1_CONTROLREG1 __REG(0x00213008)  /* SPI 1 Control Register		*/
#define MX1_INTREG1	__REG(0x0021300C)  /* SPI 1 Interrupt Control/Status Register	*/
#define MX1_TESTREG1	__REG(0x00213010)  /* SPI 1 Test Register		*/
#define MX1_PERIODREG1	__REG(0x00213014)  /* SPI 1 Sample Period Control Register	*/
#define MX1_DMAREG1	__REG(0x00213018)  /* SPI 1 DMA Control	 Register	*/
#define MX1_RESETREG1	__REG(0x00213018)  /* SPI 1 Soft Reset Register		*/

/*
 *	MX1 MMC/SDHC registers
 */
#define MX1_STR_STP_CLK __REG(0x00214000)  /* MMC/SD Clock Control Register	*/
#define MX1_STATUS	__REG(0x00214004)  /* MMC/SD Status Register		*/
#define MX1_CLK_RATE	__REG(0x00214008)  /* MMC/SD Clock Rate Register	*/
#define MX1_CMD_DAT_CONT __REG(0x0021400C)  /* MMC/SD Command & Data Control Register	*/
#define MX1_RES_TO	__REG(0x00214010)  /* MMC/SD Response Time Out Register */
#define MX1_READ_TO	__REG(0x00214014)  /* MMC/SD Read Time Out Register	*/
#define MX1_BLK_LEN	__REG(0x00214018)  /* MMC/SD Block Length Register	*/
#define MX1_NOB		__REG(0x0021401C)  /* MMC/SD Number of Block Register	*/
#define MX1_REV_NO	__REG(0x00214020)  /* MMC/SD Revision Number Register	*/
#define MX1_MMC_INT_MASK	__REG(0x00214024)  /* MMC/SD Interrupt Mask Register	*/
#define MX1_CMD		__REG(0x00214028)  /* MMC/SD Command Number Register	*/
#define MX1_ARGH	__REG(0x0021402C)  /* MMC/SD Higher Argument Register	*/
#define MX1_ARGL	__REG(0x00214030)  /* MMC/SD Lower Argument Register	*/
#define MX1_RES_FIFO	__REG(0x00214034)  /* MMC/SD Response FIFO Register	*/
#define MX1_BUFFER_ACCESS __REG(0x00214038)  /* MMC/SD Buffer Access Register	*/

/*
 *	MX1 ASP registers
 */
#define MX1_ASP_PADFIFO __REG(0x00215000)  /* Pen Sample FIFO			*/
#define MX1_ASP_VADFIFO __REG(0x00215004)  /* Voice ADC Register		*/
#define MX1_ASP_VDAFIFO __REG(0x00215008)  /* Voice DAC Register		*/
#define MX1_ASP_VADCOEF __REG(0x0021500C)  /* Voice ADC FIR Coefficients RAM	*/
#define MX1_ASP_ACNTLCR __REG(0x00215010)  /* Control Register			*/
#define MX1_ASP_PSMPLRG __REG(0x00215014)  /* Pen A/D Sample Rate Control Register	*/
#define MX1_ASP_ICNTLR	__REG(0x00215018)  /* Interrupt Control Register	*/
#define MX1_ASP_ISTATR	__REG(0x0021501C)  /* Interrupt/Error Status Register	*/
#define MX1_ASP_VADGAIN __REG(0x00215020)  /* Voice ADC Control Register	*/
#define MX1_ASP_VDAGAIN __REG(0x00215024)  /* Voice DAC Control Register	*/
#define MX1_ASP_VDACOEF __REG(0x00215028)  /* Voice DAC FIR Coefficients RAM	*/
#define MX1_ASP_CLKDIV	__REG(0x0021502C)  /* Clock Divide Register		*/
#define MX1_ASP_CMPCNTL __REG(0x0021502C)  /* Compare Control Register		*/

/*
 *	MX1 BTA registers
 */

/*
 *	MX1 I2C registers
 */
#define MX1_IADR	__REG(0x00217000)  /* I2C Address Register		*/
#define MX1_IFDR	__REG(0x00217004)  /* I2C Frequency Divider Register	*/
#define MX1_I2CR	__REG(0x00217008)  /* I2C Control Register		*/
#define MX1_I2CSR	__REG(0x0021700C)  /* I2C Status Register		*/
#define MX1_I2DR	__REG(0x00217010)  /* I2C Data I/O Register		*/

/*
 *	MX1 SSI registers
 */
#define MX1_STX		__REG(0x00218000)  /* SSI Transmit Data Register	*/
#define MX1_SRX		__REG(0x00218004)  /* SSI Receive Data Register		*/
#define MX1_SCSR	__REG(0x00218008)  /* SSI Control/Status Register	*/
#define MX1_STCR	__REG(0x0021800C)  /* SSI Transmit Configuration Register	*/
#define MX1_SRCR	__REG(0x00218010)  /* SSI Recieve Configuration Register	*/
#define MX1_STCCR	__REG(0x00218014)  /* SSI Transmit Clock Control Register	*/
#define MX1_SRCCR	__REG(0x00218018)  /* SSI Receive Clock Control Register	*/
#define MX1_STSR	__REG(0x0021801C)  /* SSI Time Slot Register		*/
#define MX1_SFCSR	__REG(0x00218020)  /* SSI FIFO Control/Status Register	*/
#define MX1_SOR		__REG(0x00218024)  /* SSI Option Register		*/

/*
 *	MX1 SPI 2 registers
 */
#define MX1_RXDATAREG2	__REG(0x00219000)  /* SPI 2 Rx Data Register		*/
#define MX1_TXDATAREG2	__REG(0x00219004)  /* SPI 2 Tx Data Register		*/
#define MX1_CONTROLREG2 __REG(0x00219008)  /* SPI 2 Control Register		*/
#define MX1_INTREG2	__REG(0x0021900C)  /* SPI 2 Interrupt Control/Status Register	*/
#define MX1_TESTREG2	__REG(0x00219010)  /* SPI 2 Test Register		*/
#define MX1_PERIODREG2	__REG(0x00219014)  /* SPI 2 Sample Period Control Register	*/
#define MX1_DMAREG2	__REG(0x00219018)  /* SPI 2 DMA Control	 Register	*/
#define MX1_RESETREG2	__REG(0x00219018)  /* SPI 2 Soft Reset Register		*/

/*
 *	MX1 MSHC registers
 */
#define MX1_MSCMD	__REG(0x0021A000)  /* Memory Stick Command Register	*/
#define MX1_MSCS	__REG(0x0021A002)  /* Memory Stick Control/Status Register	*/
#define MX1_MSTDATA	__REG(0x0021A004)  /* Memory Stick Transmit FIFO Data Register	*/
#define MX1_MSRDATA	__REG(0x0021A004)  /* Memory Stick Recieve FIFO Data Register	*/
#define MX1_MSICS	__REG(0x0021A006)  /* Memory Stick Interrupt Control/Status Register	*/
#define MX1_MSPPCD	__REG(0x0021A008)  /* Memory Stick Parallel Port Control/Data Register	*/
#define MX1_MSC2	__REG(0x0021A00A)  /* Memory Stick Control 2 Register	*/
#define MX1_MSACD	__REG(0x0021A00C)  /* Memory Stick Auto Command Register	*/
#define MX1_MSFAECS	__REG(0x0021A00E)  /* Memory Stick FIFO Access Error Control/Status Register	*/
#define MX1_MSCLKD	__REG(0x0021A010)  /* Memory Stick Serial Clock divider Register	*/
#define MX1_MSDRQC	__REG(0x0021A012)  /* Memory Stick DMA Request Control Register */

/*
 *	MX1 PLLCLK registers
 */
#define MX1_CSCR	__REG(0x0021B000)  /* Clock Source Control Register	*/
#define MX1_MPCTL0	__REG(0x0021B004)  /* MCU PLL Control Register 0	*/
#define MX1_MPCTL1	__REG(0x0021B008)  /* MCU PLL & System Clock Control Register 1 */
#define MX1_UPCTL0	__REG(0x0021B00C)  /* USB PLL Control Register 0	*/
#define MX1_UPCTL1	__REG(0x0021B010)  /* USB PLL Control Register 1	*/
#define MX1_PCDR	__REG(0x0021B020)  /* Peripheral Clock Divider Register */

/*
 *	MX1 RESET registers
 */
#define MX1_RSR		__REG(0x0021B800)  /* Reset Source Register		*/

/*
 *	MX1 SYS CTRL registers
 */
#define MX1_SIDR	__REG(0x0021B804)  /* Silicon ID Register		*/
#define MX1_FMCR	__REG(0x0021B808)  /* Function MultiPlexing Control Register	*/
#define MX1_GPCR	__REG(0x0021B80C)  /* Global Peripheral Control Register	*/

/*
 *	MX1 GPIO registers
 */

/* Port A */
#define MX1_DDIR_A	__REG(0x0021C000)  /* Port A Data Direction Register	*/
#define MX1_OCR1_A	__REG(0x0021C004)  /* Port A Output Configuration Register 1	*/
#define MX1_OCR2_A	__REG(0x0021C008)  /* Port A Output Configuration Register 2	*/
#define MX1_ICONFA1_A	__REG(0x0021C00C)  /* Port A Input Configuration Register A1	*/
#define MX1_ICONFA2_A	__REG(0x0021C010)  /* Port A Input Configuration Register A2	*/
#define MX1_ICONFB1_A	__REG(0x0021C014)  /* Port A Input Configuration Register B1	*/
#define MX1_ICONFB2_A	__REG(0x0021C018)  /* Port A Input Configuration Register B2	*/
#define MX1_DR_A	__REG(0x0021C01C)  /* Port A Data Register		*/
#define MX1_GIUS_A	__REG(0x0021C020)  /* Port A GPIO In Use Register	*/
#define MX1_SSR_A	__REG(0x0021C024)  /* Port A Sample Status Register	*/
#define MX1_ICR1_A	__REG(0x0021C028)  /* Port A Interrupt Configuration Register 1 */
#define MX1_ICR2_A	__REG(0x0021C02C)  /* Port A Interrupt Configuration Register 2 */
#define MX1_IMR_A	__REG(0x0021C030)  /* Port A Interrupt Mask Register	*/
#define MX1_ISR_A	__REG(0x0021C034)  /* Port A Interrupt Status Register	*/
#define MX1_GPR_A	__REG(0x0021C038)  /* Port A General Purpose Register	*/
#define MX1_SWR_A	__REG(0x0021C03C)  /* Port A Software Reset Register	*/
#define MX1_PUEN_A	__REG(0x0021C040)  /* Port A Pull Up Enable Register	*/

/* Port B */
#define MX1_DDIR_B	__REG(0x0021C100)  /* Port B Data Direction Register	*/
#define MX1_OCR1_B	__REG(0x0021C104)  /* Port B Output Configuration Register 1	*/
#define MX1_OCR2_B	__REG(0x0021C108)  /* Port B Output Configuration Register 2	*/
#define MX1_ICONFA1_B	__REG(0x0021C10C)  /* Port B Input Configuration Register A1	*/
#define MX1_ICONFA2_B	__REG(0x0021C110)  /* Port B Input Configuration Register A2	*/
#define MX1_ICONFB1_B	__REG(0x0021C114)  /* Port B Input Configuration Register B1	*/
#define MX1_ICONFB2_B	__REG(0x0021C118)  /* Port B Input Configuration Register B2	*/
#define MX1_DR_B	__REG(0x0021C11C)  /* Port B Data Register		*/
#define MX1_GIUS_B	__REG(0x0021C120)  /* Port B GPIO In Use Register	*/
#define MX1_SSR_B	__REG(0x0021C124)  /* Port B Sample Status Register	*/
#define MX1_ICR1_B	__REG(0x0021C128)  /* Port B Interrupt Configuration Register 1 */
#define MX1_ICR2_B	__REG(0x0021C12C)  /* Port B Interrupt Configuration Register 2 */
#define MX1_IMR_B	__REG(0x0021C130)  /* Port B Interrupt Mask Register	*/
#define MX1_ISR_B	__REG(0x0021C134)  /* Port B Interrupt Status Register	*/
#define MX1_GPR_B	__REG(0x0021C138)  /* Port B General Purpose Register	*/
#define MX1_SWR_B	__REG(0x0021C13C)  /* Port B Software Reset Register	*/
#define MX1_PUEN_B	__REG(0x0021C140)  /* Port B Pull Up Enable Register	*/

/* Port C */
#define MX1_DDIR_C	__REG(0x0021C200)  /* Port C Data Direction Register	*/
#define MX1_OCR1_C	__REG(0x0021C204)  /* Port C Output Configuration Register 1	*/
#define MX1_OCR2_C	__REG(0x0021C208)  /* Port C Output Configuration Register 2	*/
#define MX1_ICONFA1_C	__REG(0x0021C20C)  /* Port C Input Configuration Register A1	*/
#define MX1_ICONFA2_C	__REG(0x0021C210)  /* Port C Input Configuration Register A2	*/
#define MX1_ICONFB1_C	__REG(0x0021C214)  /* Port C Input Configuration Register B1	*/
#define MX1_ICONFB2_C	__REG(0x0021C218)  /* Port C Input Configuration Register B2	*/
#define MX1_DR_C	__REG(0x0021C21C)  /* Port C Data Register		*/
#define MX1_GIUS_C	__REG(0x0021C220)  /* Port C GPIO In Use Register	*/
#define MX1_SSR_C	__REG(0x0021C224)  /* Port C Sample Status Register	*/
#define MX1_ICR1_C	__REG(0x0021C228)  /* Port C Interrupt Configuration Register 1 */
#define MX1_ICR2_C	__REG(0x0021C22C)  /* Port C Interrupt Configuration Register 2 */
#define MX1_IMR_C	__REG(0x0021C230)  /* Port C Interrupt Mask Register	*/
#define MX1_ISR_C	__REG(0x0021C234)  /* Port C Interrupt Status Register	*/
#define MX1_GPR_C	__REG(0x0021C238)  /* Port C General Purpose Register	*/
#define MX1_SWR_C	__REG(0x0021C23C)  /* Port C Software Reset Register	*/
#define MX1_PUEN_C	__REG(0x0021C240)  /* Port C Pull Up Enable Register	*/

/* Port D */
#define MX1_DDIR_D	__REG(0x0021C300)  /* Port D Data Direction Register	*/
#define MX1_OCR1_D	__REG(0x0021C304)  /* Port D Output Configuration Register 1	*/
#define MX1_OCR2_D	__REG(0x0021C308)  /* Port D Output Configuration Register 2	*/
#define MX1_ICONFA1_D	__REG(0x0021C30C)  /* Port D Input Configuration Register A1	*/
#define MX1_ICONFA2_D	__REG(0x0021C310)  /* Port D Input Configuration Register A2	*/
#define MX1_ICONFB1_D	__REG(0x0021C314)  /* Port D Input Configuration Register B1	*/
#define MX1_ICONFB2_D	__REG(0x0021C318)  /* Port D Input Configuration Register B2	*/
#define MX1_DR_D	__REG(0x0021C31C)  /* Port D Data Register		*/
#define MX1_GIUS_D	__REG(0x0021C320)  /* Port D GPIO In Use Register	*/
#define MX1_SSR_D	__REG(0x0021C324)  /* Port D Sample Status Register	*/
#define MX1_ICR1_D	__REG(0x0021C328)  /* Port D Interrupt Configuration Register 1 */
#define MX1_ICR2_D	__REG(0x0021C32C)  /* Port D Interrupt Configuration Register 2 */
#define MX1_IMR_D	__REG(0x0021C330)  /* Port D Interrupt Mask Register	*/
#define MX1_ISR_D	__REG(0x0021C334)  /* Port D Interrupt Status Register	*/
#define MX1_GPR_D	__REG(0x0021C338)  /* Port D General Purpose Register	*/
#define MX1_SWR_D	__REG(0x0021C33C)  /* Port D Software Reset Register	*/
#define MX1_PUEN_D	__REG(0x0021C340)  /* Port D Pull Up Enable Register	*/

/*
 *	MX1 EIM registers
 */
#define MX1_CS0U	__REG(0x00220000)  /* Chip Select 0 Upper Control Register	*/
#define MX1_CS0L	__REG(0x00220004)  /* Chip Select 0 Lower Control Register	*/
#define MX1_CS1U	__REG(0x00220008)  /* Chip Select 1 Upper Control Register	*/
#define MX1_CS1L	__REG(0x0022000C)  /* Chip Select 1 Lower Control Register	*/
#define MX1_CS2U	__REG(0x00220010)  /* Chip Select 2 Upper Control Register	*/
#define MX1_CS2L	__REG(0x00220014)  /* Chip Select 2 Lower Control Register	*/
#define MX1_CS3U	__REG(0x00220018)  /* Chip Select 3 Upper Control Register	*/
#define MX1_CS3L	__REG(0x0022001C)  /* Chip Select 3 Lower Control Register	*/
#define MX1_CS4U	__REG(0x00220020)  /* Chip Select 4 Upper Control Register	*/
#define MX1_CS4L	__REG(0x00220024)  /* Chip Select 4 Lower Control Register	*/
#define MX1_CS5U	__REG(0x00220028)  /* Chip Select 5 Upper Control Register	*/
#define MX1_CS5L	__REG(0x0022002C)  /* Chip Select 5 Lower Control Register	*/
#define MX1_WEIM	__REG(0x00220030)  /* weim cONFIGURATION Register	*/

/*
 *	MX1 SDRAMC registers
 */
#define MX1_SDCTL0	__REG(0x00221000)  /* SDRAM 0 Control Register		*/
#define MX1_SDCTL1	__REG(0x00221004)  /* SDRAM 1 Control Register		*/
#define MX1_MISCELLANEOUS __REG(0x00221014)  /* Miscellaneous Register		*/
#define MX1_SDRST	__REG(0x00221018)  /* SDRAM Reset Register		*/

/*
 *	MX1 MMA registers
 */
#define MX1_MMA_MAC_MOD __REG(0x00222000)  /* MMA MAC Module Register		*/
#define MX1_MMA_MAC_CTRL __REG(0x00222004)  /* MMA MAC Control Register		*/
#define MX1_MMA_MAC_MULT __REG(0x00222008)  /* MMA MAC Multiply Counter Register	*/
#define MX1_MMA_MAC_ACCU __REG(0x0022200C)  /* MMA MAC Accumulate Counter Register	*/
#define MX1_MMA_MAC_INTR __REG(0x00222010)  /* MMA MAC Interrupt Register	*/
#define MX1_MMA_MAC_INTR_MASK __REG(0x00222014)	 /* MMA MAC Interrupt Mask Register	*/
#define MX1_MMA_MAC_FIFO __REG(0x00222018)  /* MMA MAC FIFO Register		*/
#define MX1_MMA_MAC_FIFO_STAT __REG(0x0022201C)	 /* MMA MAC FIFO Status Register	*/
#define MX1_MMA_MAC_BURST __REG(0x00222020)  /* MMA MAC Burst Count Register	*/
#define MX1_MMA_MAC_BITSEL __REG(0x00222024)  /* MMA MAC Bit Select Register	*/

#define MX1_MMA_MAC_XBASE __REG(0x00222200)  /* MMA MAC X Base Address Register */
#define MX1_MMA_MAC_XINDEX __REG(0x00222204)  /* MMA MAC X Index Register	*/
#define MX1_MMA_MAC_XLENGTH __REG(0x00222208)  /* MMA MAC X Length Register	*/
#define MX1_MMA_MAC_XMODIFY __REG(0x0022220C)  /* MMA MAC X Modify Register	*/
#define MX1_MMA_MAC_XINCR __REG(0x00222210)  /* MMA MAC X Increment Register	*/
#define MX1_MMA_MAC_XCOUNT __REG(0x00222214)  /* MMA MAC X Count Register	*/

#define MX1_MMA_MAC_YBASE __REG(0x00222300)  /* MMA MAC Y Base Address Register */
#define MX1_MMA_MAC_YINDEX __REG(0x00222304)  /* MMA MAC Y Index Register	*/
#define MX1_MMA_MAC_YLENGTH __REG(0x00222308)  /* MMA MAC Y Length Register	*/
#define MX1_MMA_MAC_YMODIFY __REG(0x0022230C)  /* MMA MAC Y Modify Register	*/
#define MX1_MMA_MAC_YINCR __REG(0x00222310)  /* MMA MAC Y Increment Register	*/
#define MX1_MMA_MAC_YCOUNT __REG(0x00222314)  /* MMA MAC Y Count Register	*/

#define MX1_MMA_DCTCTRL __REG(0x00222400)  /* DCT/iDCT Control Register		*/
#define MX1_MMA_DCTVERSION __REG(0x00222404)  /* DCT/iDCT Version Register	*/
#define MX1_MMA_DCTIRQENA __REG(0x00222408)  /* DCT/iDCT IRQ Enable Register	*/
#define MX1_MMA_DCTIRQSTAT __REG(0x0022240C)  /* DCT/iDCT IRQ Status Register	*/
#define MX1_MMA_DCTSRCDATA __REG(0x00222410)  /* DCT/iDCT Source Data Address	*/
#define MX1_MMA_DCTDESDATA __REG(0x00222414)  /* DCT/iDCT Destination Data Address	*/
#define MX1_MMA_DCTXOFF __REG(0x00222418)  /* DCT/iDCT X-Offset Address		*/
#define MX1_MMA_DCTYOFF __REG(0x0022241C)  /* DCT/iDCT Y-Offset Address		*/
#define MX1_MMA_DCTXYCNT __REG(0x00222420)  /* DCT/iDCT XY Count		*/
#define MX1_MMA_DCTSKIP __REG(0x00222424)  /* DCT/iDCT Skip Address		*/
#define MX1_MMA_DCTFIFO __REG(0x00222500)  /* DCT/iDCT Data FIFO		*/

/*
 *	MX1 AITC registers
 */
#define MX1_INTCNTL	__REG(0x00223000)  /* Interrupt Control Register	*/
#define MX1_NIMASK	__REG(0x00223004)  /* Normal Interrupt Mask Register	*/
#define MX1_INTENNUM	__REG(0x00223008)  /* Interrupt Enable Number Register	*/
#define MX1_INTDISNUM	__REG(0x0022300C)  /* Interrupt Disable Number Register */
#define MX1_INTENABLEH	__REG(0x00223010)  /* Interrupt Enable Register High	*/
#define MX1_INTENABLEL	__REG(0x00223014)  /* Interrupt Enable Register Low	*/
#define MX1_INTTYPEH	__REG(0x00223018)  /* Interrupt Type Register High	*/
#define MX1_INTTYPEL	__REG(0x0022301C)  /* Interrupt Type Register Low	*/
#define MX1_NIPRIORITY7 __REG(0x00223020)  /* Normal Interrupt Priority Level Register 7*/
#define MX1_NIPRIORITY6 __REG(0x00223024)  /* Normal Interrupt Priority Level Register 6*/
#define MX1_NIPRIORITY5 __REG(0x00223028)  /* Normal Interrupt Priority Level Register 5*/
#define MX1_NIPRIORITY4 __REG(0x0022302C)  /* Normal Interrupt Priority Level Register 4*/
#define MX1_NIPRIORITY3 __REG(0x00223030)  /* Normal Interrupt Priority Level Register 3*/
#define MX1_NIPRIORITY2 __REG(0x00223034)  /* Normal Interrupt Priority Level Register 2*/
#define MX1_NIPRIORITY1 __REG(0x00223038)  /* Normal Interrupt Priority Level Register 1*/
#define MX1_NIPRIORITY0 __REG(0x0022303C)  /* Normal Interrupt Priority Level Register 0*/
#define MX1_NIVECSR	__REG(0x00223040)  /* Normal Interrupt Vector & Status Register */
#define MX1_FIVECSR	__REG(0x00223044)  /* Fast Interrupt Vector & Status Register	*/
#define MX1_INTSRCH	__REG(0x00223048)  /* Interrupt Source Register High	*/
#define MX1_INTSRCL	__REG(0x0022304C)  /* Interrupt Source Register Low	*/
#define MX1_INTFRCH	__REG(0x00223050)  /* Interrupt Force Register High	*/
#define MX1_INTFRCL	__REG(0x00223054)  /* Interrupt Force Register Low	*/
#define MX1_NIPNDH	__REG(0x00223058)  /* Normal Interrupt Pending Register High	*/
#define MX1_NIPNDL	__REG(0x0022305C)  /* Normal Interrupt Pending Register Low	*/
#define MX1_FIPNDH	__REG(0x00223060)  /* Fast Interrupt Pending Register High	*/
#define MX1_FIPNDL	__REG(0x00223064)  /* Fast Interrupt Pending Register Low	*/

/*
 *	MX1 CSI registers
 */
#define MX1_CSICR1	__REG(0x00224000)  /* CSI Control Register 1		*/
#define MX1_CSICR2	__REG(0x00224004)  /* CSI Control Register 2		*/
#define MX1_CSISR	__REG(0x00224008)  /* CSI Status Register 1		*/
#define MX1_CSISTATR	__REG(0x0022400C)  /* CSI Statistic FIFO Register 1	*/
#define MX1_CSIRXR	__REG(0x00224010)  /* CSI RxFIFO Register 1		*/

#endif	/*  __MC9328_H__ */

#if 0
/*
	MX1 dma definition
*/

#define MAX_DMA_ADDRESS		0xffffffff

/*#define MAX_DMA_CHANNELS	0 */

#define MAX_DMA_CHANNELS		11
#define MAX_DMA_2D_REGSET		2

/* MX1 DMA module registers' address */

#define MX1_DMA_BASE		IO_ADDRESS(0x00209000)
#define MX1_DMA_DCR		(MX1_DMA_BASE + 0x00)		/* DMA control register */
#define MX1_DMA_DISR		(MX1_DMA_BASE + 0x04)		/* DMA interrupt status register */
#define MX1_DMA_DIMR		(MX1_DMA_BASE + 0x08)		/* DMA interrupt mask register */
#define MX1_DMA_DBTOSR		(MX1_DMA_BASE + 0x0C)		/* DMA burst time-out status register */
#define MX1_DMA_DRTOSR		(MX1_DMA_BASE + 0x10)		/* DMA request time-out status register */
#define MX1_DMA_DSESR		(MX1_DMA_BASE + 0x14)		/* DMA transfer error status register */
#define MX1_DMA_DBOSR		(MX1_DMA_BASE + 0x18)		/* DMA buffer overflow status register */
#define MX1_DMA_DBTOCR		(MX1_DMA_BASE + 0x1C)		/* DMA burst time-out control register */
#define MX1_DMA_WSRA		(MX1_DMA_BASE + 0x40)		/* W-size register A */
#define MX1_DMA_XSRA		(MX1_DMA_BASE + 0x44)		/* X-size register A */
#define MX1_DMA_YSRA		(MX1_DMA_BASE + 0x48)		/* Y-size register A */
#define MX1_DMA_WSRB		(MX1_DMA_BASE + 0x4C)		/* W-size register B */
#define MX1_DMA_XSRB		(MX1_DMA_BASE + 0x50)		/* X-size register B */
#define MX1_DMA_YSRB		(MX1_DMA_BASE + 0x54)		/* Y-size register B */

#define MX1_DMA_SAR0		(MX1_DMA_BASE + 0x80)		/* source address register 0 */
#define MX1_DMA_DAR0		(MX1_DMA_BASE + 0x84)		/* destination address register 0 */
#define MX1_DMA_CNTR0		(MX1_DMA_BASE + 0x88)		/* count register 0 */
#define MX1_DMA_CCR0		(MX1_DMA_BASE + 0x8C)		/* channel control register 0 */
#define MX1_DMA_RSSR0		(MX1_DMA_BASE + 0x90)		/* request source select register 0 */
#define MX1_DMA_BLR0		(MX1_DMA_BASE + 0x94)		/* burst length register 0 */
#define MX1_DMA_RTOR0		(MX1_DMA_BASE + 0x98)		/* request time-out register 0 */
#define MX1_DMA_BUCR0		(MX1_DMA_BASE + 0x98)		/* bus utilization control register 0 */

/* register set 1 to 10 are offseted by 0x40 each = 0x10 pointers away */

#define DMA_REG_SET_OFS		0x10

/* MX1 DMA module registers */
#define _reg_DMA_DCR		(*((P_VU32)MX1_DMA_DCR))
#define _reg_DMA_DISR		(*((P_VU32)MX1_DMA_DISR))
#define _reg_DMA_DIMR		(*((P_VU32)MX1_DMA_DIMR))
#define _reg_DMA_DBTOSR		(*((P_VU32)MX1_DMA_DBTOSR))
#define _reg_DMA_DRTOSR		(*((P_VU32)MX1_DMA_DRTOSR))
#define _reg_DMA_DSESR		(*((P_VU32)MX1_DMA_DSESR))
#define _reg_DMA_DBOSR		(*((P_VU32)MX1_DMA_DBOSR))
#define _reg_DMA_DBTOCR		(*((P_VU32)MX1_DMA_DBTOCR))
#define _reg_DMA_WSRA		(*((P_VU32)MX1_DMA_WSRA))
#define _reg_DMA_XSRA		(*((P_VU32)MX1_DMA_XSRA))
#define _reg_DMA_YSRA		(*((P_VU32)MX1_DMA_YSRA))
#define _reg_DMA_WSRB		(*((P_VU32)MX1_DMA_WSRB))
#define _reg_DMA_XSRB		(*((P_VU32)MX1_DMA_XSRB))
#define _reg_DMA_YSRB		(*((P_VU32)MX1_DMA_YSRB))
#define _reg_DMA_SAR0		(*((P_VU32)MX1_DMA_SAR0))
#define _reg_DMA_DAR0		(*((P_VU32)MX1_DMA_DAR0))
#define _reg_DMA_CNTR0		(*((P_VU32)MX1_DMA_CNTR0))
#define _reg_DMA_CCR0		(*((P_VU32)MX1_DMA_CCR0))
#define _reg_DMA_RSSR0		(*((P_VU32)MX1_DMA_RSSR0))
#define _reg_DMA_BLR0		(*((P_VU32)MX1_DMA_BLR0))
#define _reg_DMA_RTOR0		(*((P_VU32)MX1_DMA_RTOR0))
#define _reg_DMA_BUCR0		(*((P_VU32)MX1_DMA_BUCR0))

/*  DMA error type definition */
#define MX1_DMA_ERR_BTO		0	/* burst time-out */
#define MX1_DMA_ERR_RTO		1	/* request time-out */
#define MX1_DMA_ERR_TE		2	/* transfer error */
#define MX1_DMA_ERR_BO		3	/* buffer overflow */

/* Embedded SRAM */

#define MX1_SRAM_BASE		0x00300000
#define MX1_SRAM_SIZE		0x00020000

#define

#define MX1ADS_SFLASH_BASE	0x0C000000
#define MX1ADS_SFLASH_SIZE	SZ_16M

#define MX1ADS_IO_BASE		0x00200000
#define MX1ADS_IO_SIZE		SZ_256K

#define MX1ADS_VID_BASE		0x00300000
#define MX1ADS_VID_SIZE		0x26000

#define MX1ADS_VID_START	IO_ADDRESS(MX1ADS_VID_BASE)

#define MX1_GPIO_BASE		0x0021C000	/* GPIO */
#define MX1_EXT_UART_BASE	0x15000000	/* external UART */
#define MX1_TMR1_BASE		0x00202000	/* Timer1 */
#define MX1ADS_FLASH_BASE	0x0C000000	/* sync FLASH */
#define MX1_ESRAM_BASE		0x00300000	/* embedded SRAM */
#define MX1ADS_SDRAM_DISK_BASE	0x0B000000	/* SDRAM disk base (last 16M of SDRAM) */

/* ------------------------------------------------------------------------
 *  Motorola MX1 system registers
 * ------------------------------------------------------------------------
 *
 */

/*
 *  Register offests.
 *
 */

#define MX1ADS_AIPI1_OFFSET		0x00000
#define MX1ADS_WDT_OFFSET		0x01000
#define MX1ADS_TIM1_OFFSET		0x02000
#define MX1ADS_TIM2_OFFSET		0x03000
#define MX1ADS_RTC_OFFSET		0x04000
#define MX1ADS_LCDC_OFFSET		0x05000
#define MX1ADS_UART1_OFFSET		0x06000
#define MX1ADS_UART2_OFFSET		0x07000
#define MX1ADS_PWM_OFFSET		0x08000
#define MX1ADS_DMAC_OFFSET		0x09000
#define MX1ADS_AIPI2_OFFSET		0x10000
#define MX1ADS_SIM_OFFSET		0x11000
#define MX1ADS_USBD_OFFSET		0x12000
#define MX1ADS_SPI1_OFFSET		0x13000
#define MX1ADS_MMC_OFFSET		0x14000
#define MX1ADS_ASP_OFFSET		0x15000
#define MX1ADS_BTA_OFFSET		0x16000
#define MX1ADS_I2C_OFFSET		0x17000
#define MX1ADS_SSI_OFFSET		0x18000
#define MX1ADS_SPI2_OFFSET		0x19000
#define MX1ADS_MSHC_OFFSET		0x1A000
#define MX1ADS_PLL_OFFSET		0x1B000
#define MX1ADS_GPIO_OFFSET		0x1C000
#define MX1ADS_EIM_OFFSET		0x20000
#define MX1ADS_SDRAMC_OFFSET		0x21000
#define MX1ADS_MMA_OFFSET		0x22000
#define MX1ADS_AITC_OFFSET		0x23000
#define MX1ADS_CSI_OFFSET		0x24000

/*
 *  Register BASEs, based on OFFSETs
 *
 */

#define MX1ADS_AIPI1_BASE		(MX1ADS_AIPI1_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_WDT_BASE			(MX1ADS_WDT_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_TIM1_BASE		(MX1ADS_TIM1_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_TIM2_BASE		(MX1ADS_TIM2_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_RTC_BASE			(MX1ADS_RTC_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_LCDC_BASE		(MX1ADS_LCDC_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_UART1_BASE		(MX1ADS_UART1_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_UART2_BASE		(MX1ADS_UART2_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_PWM_BASE			(MX1ADS_PWM_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_DMAC_BASE		(MX1ADS_DMAC_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_AIPI2_BASE		(MX1ADS_AIPI2_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_SIM_BASE			(MX1ADS_SIM_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_USBD_BASE		(MX1ADS_USBD_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_SPI1_BASE		(MX1ADS_SPI1_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_MMC_BASE			(MX1ADS_MMC_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_ASP_BASE			(MX1ADS_ASP_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_BTA_BASE			(MX1ADS_BTA_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_I2C_BASE			(MX1ADS_I2C_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_SSI_BASE			(MX1ADS_SSI_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_SPI2_BASE		(MX1ADS_SPI2_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_MSHC_BASE		(MX1ADS_MSHC_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_PLL_BASE			(MX1ADS_PLL_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_GPIO_BASE		(MX1ADS_GPIO_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_EIM_BASE			(MX1ADS_EIM_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_SDRAMC_BASE		(MX1ADS_SDRAMC_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_MMA_BASE			(MX1ADS_MMA_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_AITC_BASE		(MX1ADS_AITC_OFFSET + MX1ADS_IO_BASE)
#define MX1ADS_CSI_BASE			(MX1ADS_CSI_OFFSET + MX1ADS_IO_BASE)

/*
 *  MX1 Interrupt numbers
 *
 */
#define INT_SOFTINT			0
#define CSI_INT				6
#define DSPA_MAC_INT			7
#define DSPA_INT			8
#define COMP_INT			9
#define MSHC_XINT			10
#define GPIO_INT_PORTA			11
#define GPIO_INT_PORTB			12
#define GPIO_INT_PORTC			13
#define LCDC_INT			14
#define SIM_INT				15
#define SIM_DATA_INT			16
#define RTC_INT				17
#define RTC_SAMINT			18
#define UART2_MINT_PFERR		19
#define UART2_MINT_RTS			20
#define UART2_MINT_DTR			21
#define UART2_MINT_UARTC		22
#define UART2_MINT_TX			23
#define UART2_MINT_RX			24
#define UART1_MINT_PFERR		25
#define UART1_MINT_RTS			26
#define UART1_MINT_DTR			27
#define UART1_MINT_UARTC		28
#define UART1_MINT_TX			29
#define UART1_MINT_RX			30
#define VOICE_DAC_INT			31
#define VOICE_ADC_INT			32
#define PEN_DATA_INT			33
#define PWM_INT				34
#define SDHC_INT			35
#define I2C_INT				39
#define CSPI_INT			41
#define SSI_TX_INT			42
#define SSI_TX_ERR_INT			43
#define SSI_RX_INT			44
#define SSI_RX_ERR_INT			45
#define TOUCH_INT			46
#define USBD_INT0			47
#define USBD_INT1			48
#define USBD_INT2			49
#define USBD_INT3			50
#define USBD_INT4			51
#define USBD_INT5			52
#define USBD_INT6			53
#define BTSYS_INT			55
#define BTTIM_INT			56
#define BTWUI_INT			57
#define TIMER2_INT			58
#define TIMER1_INT			59
#define DMA_ERR				60
#define DMA_INT				61
#define GPIO_INT_PORTD			62

#define MAXIRQNUM			62
#define MAXFIQNUM			62
#define MAXSWINUM			62

#define TICKS_PER_uSEC			24

/*
 *  These are useconds NOT ticks.
 *
 */
#define mSEC_1				1000
#define mSEC_5				(mSEC_1 * 5)
#define mSEC_10				(mSEC_1 * 10)
#define mSEC_25				(mSEC_1 * 25)
#define SEC_1				(mSEC_1 * 1000)

#endif

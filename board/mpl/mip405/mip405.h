/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
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
 *
 */
 /****************************************************************************
 * Global routines used for MIP405
 *****************************************************************************/
#ifndef __ASSEMBLY__
/*int switch_cs(unsigned char boot);*/

extern int  mem_test(unsigned long start, unsigned long ramsize,int mode);

void user_led0(unsigned char on);


#endif
/* timings */
/* PLD (CS7) */
#define PLD_BME	0	/* Burst disable */
#define PLD_TWE	5	/* 5 * 30ns 120ns Waitstates (access=TWT+1+TH) */
#define PLD_CSN	1	/* Chipselect is driven inactive for 1 Cycle BTW transfers */
#define PLD_OEN	1	/* Cycles from CS low to OE low   */
#define PLD_WBN	1	/* Cycles from CS low to WE low   */
#define PLD_WBF	1	/* Cycles from WE high to CS high */
#define PLD_TH	2	/* Number of hold cycles after transfer */
#define PLD_RE	0	/* Ready disabled */
#define PLD_SOR	1	/* Sample on Ready disabled */
#define PLD_BEM	0	/* Byte Write only active on Write cycles */
#define PLD_PEN	0	/* Parity disable */
#define PLD_AP	((PLD_BME << 31) + (PLD_TWE << 23) + (PLD_CSN << 18) + (PLD_OEN << 16) + (PLD_WBN << 14) + \
					(PLD_WBF << 12) + (PLD_TH << 9) + (PLD_RE << 8) + (PLD_SOR << 7) + (PLD_BEM << 6) + (PLD_PEN << 5))

/* Size: 0=1MB, 1=2MB, 2=4MB, 3=8MB, 4=16MB, 5=32MB, 6=64MB, 7=128MB */
#define PLD_BS	0	/* 1 MByte */
/* Usage: 0=disabled, 1=Read only, 2=Write Only, 3=R/W */
#define PLD_BU	3	/* R/W */
/* Bus width: 0=8Bit, 1=16Bit, 2=32Bit, 3=Reserved */
#define PLD_BW	0	/* 16Bit */
#define PLD_CR	((PER_PLD_ADDR & 0xfff00000) + (PLD_BS << 17) + (PLD_BU << 15) + (PLD_BW << 13))


/* timings */

#define PER_BOARD_ADDR (PER_UART1_ADDR+(1024*1024))
/* Dummy CS to get the board revision */
#define BOARD_BME	0	/* Burst disable */
#define BOARD_TWE	255	/* 255 * 30ns 120ns Waitstates (access=TWT+1+TH) */
#define BOARD_CSN	1	/* Chipselect is driven inactive for 1 Cycle BTW transfers */
#define BOARD_OEN	1	/* Cycles from CS low to OE low   */
#define BOARD_WBN	1	/* Cycles from CS low to WE low   */
#define BOARD_WBF	1	/* Cycles from WE high to CS high */
#define BOARD_TH	2	/* Number of hold cycles after transfer */
#define BOARD_RE	0	/* Ready disabled */
#define BOARD_SOR	1	/* Sample on Ready disabled */
#define BOARD_BEM	0	/* Byte Write only active on Write cycles */
#define BOARD_PEN	0	/* Parity disable */
#define BOARD_AP	((BOARD_BME << 31) + (BOARD_TWE << 23) + (BOARD_CSN << 18) + (BOARD_OEN << 16) + (BOARD_WBN << 14) + \
					(BOARD_WBF << 12) + (BOARD_TH << 9) + (BOARD_RE << 8) + (BOARD_SOR << 7) + (BOARD_BEM << 6) + (BOARD_PEN << 5))

/* Size: 0=1MB, 1=2MB, 2=4MB, 3=8MB, 4=16MB, 5=32MB, 6=64MB, 7=128MB */
#define BOARD_BS	0	/* 1 MByte */
/* Usage: 0=disabled, 1=Read only, 2=Write Only, 3=R/W */
#define BOARD_BU	3	/* R/W */
/* Bus width: 0=8Bit, 1=16Bit, 2=32Bit, 3=Reserved */
#define BOARD_BW	0	/* 16Bit */
#define BOARD_CR	((PER_BOARD_ADDR & 0xfff00000) + (BOARD_BS << 17) + (BOARD_BU << 15) + (BOARD_BW << 13))


/* UART0 CS2 */
#define UART0_BME	0	/* Burst disable */
#define UART0_TWE	7	/* 7 * 30ns 210ns Waitstates (access=TWT+1+TH) */
#define UART0_CSN	1	/* Chipselect is driven inactive for 1 Cycle BTW transfers */
#define UART0_OEN	1	/* Cycles from CS low to OE low   */
#define UART0_WBN	1	/* Cycles from CS low to WE low   */
#define UART0_WBF	1	/* Cycles from WE high to CS high */
#define UART0_TH	2	/* Number of hold cycles after transfer */
#define UART0_RE	0	/* Ready disabled */
#define UART0_SOR	1	/* Sample on Ready disabled */
#define UART0_BEM	0	/* Byte Write only active on Write cycles */
#define UART0_PEN	0	/* Parity disable */
#define UART0_AP	((UART0_BME << 31) + (UART0_TWE << 23) + (UART0_CSN << 18) + (UART0_OEN << 16) + (UART0_WBN << 14) + \
					(UART0_WBF << 12) + (UART0_TH << 9) + (UART0_RE << 8) + (UART0_SOR << 7) + (UART0_BEM << 6) + (UART0_PEN << 5))

/* Size: 0=1MB, 1=2MB, 2=4MB, 3=8MB, 4=16MB, 5=32MB, 6=64MB, 7=128MB */
#define UART0_BS	0	/* 1 MByte */
/* Usage: 0=disabled, 1=Read only, 2=Write Only, 3=R/W */
#define UART0_BU	3	/* R/W */
/* Bus width: 0=8Bit, 1=16Bit, 2=32Bit, 3=Reserved */
#define UART0_BW	0	/* 8Bit */
#define UART0_CR	((PER_UART0_ADDR & 0xfff00000) + (UART0_BS << 17) + (UART0_BU << 15) + (UART0_BW << 13))

/* UART1 CS3 */
#define UART1_AP UART0_AP /* same timing as UART0 */
#define UART1_CR	((PER_UART1_ADDR & 0xfff00000) + (UART0_BS << 17) + (UART0_BU << 15) + (UART0_BW << 13))


/* Flash CS0 or CS 1 */
/* 0x7F8FFE80 slowest timing at all... */
#define FLASH_BME_B	1	/* Burst enable */
#define FLASH_FWT_B	0x6	/* 6 * 30ns 210ns First Wait Access */
#define FLASH_BWT_B	0x6	/* 6 * 30ns 210ns Burst Wait Access */
#define FLASH_BME	0	/* Burst disable */
#define FLASH_TWE	0xb/* 11 * 30ns 330ns Waitstates (access=TWT+1+TH) */
#define FLASH_CSN	0	/* Chipselect is driven inactive for 1 Cycle BTW transfers */
#define FLASH_OEN	1	/* Cycles from CS low to OE low   */
#define FLASH_WBN	1	/* Cycles from CS low to WE low   */
#define FLASH_WBF	1	/* Cycles from WE high to CS high */
#define FLASH_TH	2	/* Number of hold cycles after transfer */
#define FLASH_RE	0	/* Ready disabled */
#define FLASH_SOR	1	/* Sample on Ready disabled */
#define FLASH_BEM	0	/* Byte Write only active on Write cycles */
#define FLASH_PEN	0	/* Parity disable */
/* Access Parameter Register for non Boot */
#define FLASH_AP	((FLASH_BME << 31) + (FLASH_TWE << 23) + (FLASH_CSN << 18) + (FLASH_OEN << 16) + (FLASH_WBN << 14) + \
					(FLASH_WBF << 12) + (FLASH_TH << 9) + (FLASH_RE << 8) + (FLASH_SOR << 7) + (FLASH_BEM << 6) + (FLASH_PEN << 5))
/* Access Parameter Register for Boot */
#define FLASH_AP_B	((FLASH_BME_B << 31) + (FLASH_FWT_B << 26) + (FLASH_BWT_B << 23) + (FLASH_CSN << 18) + (FLASH_OEN << 16) + (FLASH_WBN << 14) + \
					(FLASH_WBF << 12) + (FLASH_TH << 9) + (FLASH_RE << 8) + (FLASH_SOR << 7) + (FLASH_BEM << 6) + (FLASH_PEN << 5))

/* Size: 0=1MB, 1=2MB, 2=4MB, 3=8MB, 4=16MB, 5=32MB, 6=64MB, 7=128MB */
#define FLASH_BS	FLASH_SIZE_PRELIM	/* 4 MByte */
/* Usage: 0=disabled, 1=Read only, 2=Write Only, 3=R/W */
#define FLASH_BU	3	/* R/W */
/* Bus width: 0=8Bit, 1=16Bit, 2=32Bit, 3=Reserved */
#define FLASH_BW	1	/* 16Bit */
/* CR register for Boot */
#define FLASH_CR_B	((FLASH_BASE_PRELIM & 0xfff00000) + (FLASH_BS << 17) + (FLASH_BU << 15) + (FLASH_BW << 13))
/* CR register for non Boot */
#define FLASH_CR	((MULTI_PURPOSE_SOCKET_ADDR & 0xfff00000) + (FLASH_BS << 17) + (FLASH_BU << 15) + (FLASH_BW << 13))

/* MPS CS1 or CS0 */
/* Boot CS: */
#define MPS_BME_B	1	/* Burst enable */
#define MPS_FWT_B	0x6/* 6 * 30ns 210ns First Wait Access */
#define MPS_BWT_B	0x6	/* 6 * 30ns 210ns Burst Wait Access */
#define MPS_BME		0	/* Burst disable */
#define MPS_TWE		0xb/* 11 * 30ns 330ns Waitstates (access=TWT+1+TH) */
#define MPS_CSN		0	/* Chipselect is driven inactive for 1 Cycle BTW transfers */
#define MPS_OEN		1	/* Cycles from CS low to OE low   */
#define MPS_WBN		1	/* Cycles from CS low to WE low   */
#define MPS_WBF		1	/* Cycles from WE high to CS high */
#define MPS_TH		2	/* Number of hold cycles after transfer */
#define MPS_RE		0	/* Ready disabled */
#define MPS_SOR		1	/* Sample on Ready disabled */
#define MPS_BEM		0	/* Byte Write only active on Write cycles */
#define MPS_PEN		0	/* Parity disable */
/* Access Parameter Register for non Boot */
#define MPS_AP		((MPS_BME << 31) + (MPS_TWE << 23) + (MPS_CSN << 18) + (MPS_OEN << 16) + (MPS_WBN << 14) + \
					(MPS_WBF << 12) + (MPS_TH << 9) + (MPS_RE << 8) + (MPS_SOR << 7) + (MPS_BEM << 6) + (MPS_PEN << 5))
/* Access Parameter Register for Boot */
#define MPS_AP_B		((MPS_BME_B << 31) + (MPS_FWT_B << 26) + (MPS_BWT_B << 23) + (MPS_CSN << 18) + (MPS_OEN << 16) + (MPS_WBN << 14) + \
					(MPS_WBF << 12) + (MPS_TH << 9) + (MPS_RE << 8) + (MPS_SOR << 7) + (MPS_BEM << 6) + (MPS_PEN << 5))

/* Size: 0=1MB, 1=2MB, 2=4MB, 3=8MB, 4=16MB, 5=32MB, 6=64MB, 7=128MB */
#define MPS_BS		2	/* 4 MByte */
#define MPS_BS_B		FLASH_SIZE_PRELIM	/* 1 MByte */
/* Usage: 0=disabled, 1=Read only, 2=Write Only, 3=R/W */
#define MPS_BU		3	/* R/W */
/* Bus width: 0=8Bit, 1=16Bit, 2=32Bit, 3=Reserved */
#define MPS_BW		0	/* 8Bit */
/* CR register for Boot */
#define MPS_CR_B	((FLASH_BASE_PRELIM & 0xfff00000) + (MPS_BS_B << 17) + (MPS_BU << 15) + (MPS_BW << 13))
/* CR register for non Boot */
#define MPS_CR		((MULTI_PURPOSE_SOCKET_ADDR & 0xfff00000) + (MPS_BS << 17) + (MPS_BU << 15) + (MPS_BW << 13))

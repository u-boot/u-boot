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
 * Global routines used for PIP405
 *****************************************************************************/

#ifndef __ASSEMBLY__

extern int  mem_test(unsigned long start, unsigned long ramsize,int mode);

void print_pip405_info(void);

void user_led0(unsigned char on);
void user_led1(unsigned char on);


#define PLD_BASE_ADDRESS		CFG_ISA_IO_BASE_ADDRESS + 0x800
#define PLD_PART_REG			PLD_BASE_ADDRESS + 0
#define PLD_VERS_REG			PLD_BASE_ADDRESS + 1
#define PLD_BOARD_CFG_REG		PLD_BASE_ADDRESS + 2
#define PLD_LED_USER_REG		PLD_BASE_ADDRESS + 3
#define PLD_SYS_MAN_REG			PLD_BASE_ADDRESS + 4
#define PLD_FLASH_COM_REG		PLD_BASE_ADDRESS + 5
#define PLD_CAN_REG			PLD_BASE_ADDRESS + 6
#define PLD_SER_PWR_REG			PLD_BASE_ADDRESS + 7
#define PLD_COM_PWR_REG			PLD_BASE_ADDRESS + 8
#define PLD_NIC_VGA_REG			PLD_BASE_ADDRESS + 9
#define PLD_SCSI_RST_REG		PLD_BASE_ADDRESS + 0xA

#define PIIX4_VENDOR_ID			0x8086
#define PIIX4_IDE_DEV_ID		0x7111

#endif

/* timings */

/* CS Config register (CS7) */
#define CONFIG_PORT_BME	0 	/* Burst disable */
#define CONFIG_PORT_TWE	255	/* 255 * 30ns 120ns Waitstates (access=TWT+1+TH) */
#define CONFIG_PORT_CSN	1	/* Chipselect is driven inactive for 1 Cycle BTW transfers */
#define CONFIG_PORT_OEN	1	/* Cycles from CS low to OE low   */
#define CONFIG_PORT_WBN	1	/* Cycles from CS low to WE low   */
#define CONFIG_PORT_WBF	1	/* Cycles from WE high to CS high */
#define CONFIG_PORT_TH	2	/* Number of hold cycles after transfer */
#define CONFIG_PORT_RE	0	/* Ready disabled */
#define CONFIG_PORT_SOR	1	/* Sample on Ready disabled */
#define CONFIG_PORT_BEM	0	/* Byte Write only active on Write cycles */
#define CONFIG_PORT_PEN	0	/* Parity disable */
#define CONFIG_PORT_AP 	((CONFIG_PORT_BME << 31) + (CONFIG_PORT_TWE << 23) + (CONFIG_PORT_CSN << 18) + (CONFIG_PORT_OEN << 16) + (CONFIG_PORT_WBN << 14) + \
				(CONFIG_PORT_WBF << 12) + (CONFIG_PORT_TH << 9) + (CONFIG_PORT_RE << 8) + (CONFIG_PORT_SOR << 7) + (CONFIG_PORT_BEM << 6) + (CONFIG_PORT_PEN << 5))

/* Size: 0=1MB, 1=2MB, 2=4MB, 3=8MB, 4=16MB, 5=32MB, 6=64MB, 7=128MB */
#define CONFIG_PORT_BS	0	/* 1 MByte */
/* Usage: 0=disabled, 1=Read only, 2=Write Only, 3=R/W */
#define CONFIG_PORT_BU	3	/* R/W */
/* Bus width: 0=8Bit, 1=16Bit, 2=32Bit, 3=Reserved */
#define CONFIG_PORT_BW	0	/* 16Bit */
#define CONFIG_PORT_CR	((CONFIG_PORT_ADDR & 0xfff00000) + (CONFIG_PORT_BS << 17) + (CONFIG_PORT_BU << 15) + (CONFIG_PORT_BW << 13))

/* Flash CS0 or CS 1 */
/* 0x7F8FFE80 slowest timing at all... */
#define FLASH_BME_B	1 	/* Burst enable */
#define FLASH_FWT_B	0x6	/* 6 * 30ns 210ns First Wait Access */
#define FLASH_BWT_B	0x6	/* 6 * 30ns 210ns Burst Wait Access */
#define FLASH_BME	0 	/* Burst disable */
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
#define FLASH_AP 	((FLASH_BME << 31) + (FLASH_TWE << 23) + (FLASH_CSN << 18) + (FLASH_OEN << 16) + (FLASH_WBN << 14) + \
				(FLASH_WBF << 12) + (FLASH_TH << 9) + (FLASH_RE << 8) + (FLASH_SOR << 7) + (FLASH_BEM << 6) + (FLASH_PEN << 5))
/* Access Parameter Register for Boot */
#define FLASH_AP_B 	((FLASH_BME_B << 31) + (FLASH_FWT_B << 26) + (FLASH_BWT_B << 23) + (FLASH_CSN << 18) + (FLASH_OEN << 16) + (FLASH_WBN << 14) + \
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
#define MPS_BME_B	1 	/* Burst enable */
#define MPS_FWT_B	0x6/* 6 * 30ns 210ns First Wait Access */
#define MPS_BWT_B	0x6	/* 6 * 30ns 210ns Burst Wait Access */
#define MPS_BME		0 	/* Burst disable */
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
#define MPS_AP 		((MPS_BME << 31) + (MPS_TWE << 23) + (MPS_CSN << 18) + (MPS_OEN << 16) + (MPS_WBN << 14) + \
				(MPS_WBF << 12) + (MPS_TH << 9) + (MPS_RE << 8) + (MPS_SOR << 7) + (MPS_BEM << 6) + (MPS_PEN << 5))
/* Access Parameter Register for Boot */
#define MPS_AP_B 	((MPS_BME_B << 31) + (MPS_FWT_B << 26) + (MPS_BWT_B << 23) + (MPS_CSN << 18) + (MPS_OEN << 16) + (MPS_WBN << 14) + \
				(MPS_WBF << 12) + (MPS_TH << 9) + (MPS_RE << 8) + (MPS_SOR << 7) + (MPS_BEM << 6) + (MPS_PEN << 5))

/* Size: 0=1MB, 1=2MB, 2=4MB, 3=8MB, 4=16MB, 5=32MB, 6=64MB, 7=128MB */
#define MPS_BS		2	/* 4 MByte */
#define MPS_BS_B		FLASH_SIZE_PRELIM	/* 1 MByte */
/* Usage: 0=disabled, 1=Read only, 2=Write Only, 3=R/W */
#define MPS_BU		3	/* R/W */
/* Bus width: 0=8Bit, 1=16Bit, 2=32Bit, 3=Reserved */
#define MPS_BW		0	/* 8Bit */
/* CR register for Boot */
#define MPS_CR_B	((FLASH_BASE_PRELIM & 0xfff00000) + (MPS_BS << 17) + (MPS_BU << 15) + (MPS_BW << 13))
/* CR register for non Boot */
#define MPS_CR		((MULTI_PURPOSE_SOCKET_ADDR & 0xfff00000) + (MPS_BS << 17) + (MPS_BU << 15) + (MPS_BW << 13))

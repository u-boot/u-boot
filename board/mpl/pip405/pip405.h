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


extern int  mem_test(unsigned long start, unsigned long ramsize,int mode);

void print_pip405_info(void);

void user_led0(unsigned char on);
void user_led1(unsigned char on);


#define PLD_BASE_ADDRESS		CFG_ISA_IO_BASE_ADDRESS + 0x800
#define PLD_PART_REG				PLD_BASE_ADDRESS + 0
#define PLD_VERS_REG				PLD_BASE_ADDRESS + 1
#define PLD_BOARD_CFG_REG		PLD_BASE_ADDRESS + 2
#define PLD_LED_USER_REG		PLD_BASE_ADDRESS + 3
#define PLD_SYS_MAN_REG			PLD_BASE_ADDRESS + 4
#define PLD_FLASH_COM_REG		PLD_BASE_ADDRESS + 5
#define PLD_CAN_REG					PLD_BASE_ADDRESS + 6
#define PLD_SER_PWR_REG			PLD_BASE_ADDRESS + 7
#define PLD_COM_PWR_REG			PLD_BASE_ADDRESS + 8
#define PLD_NIC_VGA_REG			PLD_BASE_ADDRESS + 9
#define PLD_SCSI_RST_REG		PLD_BASE_ADDRESS + 0xA

#define PIIX4_VENDOR_ID			0x8086
#define PIIX4_IDE_DEV_ID		0x7111



/*
 * (C) Copyright 2009
 * Werner Pfister <Pfister_Werner@intercontrol.de>
 *
 * (C) Copyright 2009 Semihalf, Grzegorz Bernacki
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

#ifndef CMD_MTC_H
#define CMD_MTC_H

#define	CMD_WD_PARA		0x02
#define	CMD_WD_WDSTATE		0x04
#define	CMD_FW_VERSION		0x10
#define	CMD_GET_VIM		0x30
#define	CMD_SET_LED		0x40

typedef struct {
	u8 cmd;
	u8 sys_in;
	u8 cmd_val0;
	u8 cmd_val1;
	u8 cmd_val2;
	u8 user_out;
	u8 cks;
	u8 dummy1;
	u8 dummy2;
} tx_msp_cmd;

typedef struct {
	u8 input;
	u8 state;
	u8 ack2;
	u8 ack3;
	u8 ack0;
	u8 ack1;
	u8 ack;
	u8 dummy;
	u8 cks;
} rx_msp_cmd;

#define MTC_TRANSFER_SIZE (sizeof(tx_msp_cmd) * 8)

#endif

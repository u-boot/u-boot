/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#ifndef __SCM_H
#define __SCM_H

/*----------------*/
/* CAN Structures */
/*----------------*/

/* Message */
typedef struct can_msg {
    uchar	ctrl_0;
    uchar	ctrl_1;
    uchar	arbit_0;
    uchar	arbit_1;
    uchar	arbit_2;
    uchar	arbit_3;
    uchar	config;
    uchar	data[8];
} can_msg_t;

/* CAN Register */
typedef struct can_reg {
    uchar	ctrl;
    uchar	status;
    uchar	cpu_interface;
    uchar	resv0;
    ushort	high_speed_rd;
    ushort	gbl_mask_std;
    uint	gbl_mask_extd;
    uint	msg15_mask;
    can_msg_t	msg1 __attribute__ ((packed));
    uchar	clkout;
    can_msg_t	msg2 __attribute__ ((packed));
    uchar	bus_config;
    can_msg_t	msg3 __attribute__ ((packed));
    uchar	bit_timing_0;
    can_msg_t	msg4 __attribute__ ((packed));
    uchar	bit_timing_1;
    can_msg_t	msg5 __attribute__ ((packed));
    uchar	interrupt;
    can_msg_t	msg6 __attribute__ ((packed));
    uchar	resv1;
    can_msg_t	msg7 __attribute__ ((packed));
    uchar	resv2;
    can_msg_t	msg8 __attribute__ ((packed));
    uchar	resv3;
    can_msg_t	msg9 __attribute__ ((packed));
    uchar	p1conf;
    can_msg_t	msg10 __attribute__ ((packed));
    uchar	p2conf;
    can_msg_t	msg11 __attribute__ ((packed));
    uchar	p1in;
    can_msg_t	msg12 __attribute__ ((packed));
    uchar	p2in;
    can_msg_t	msg13 __attribute__ ((packed));
    uchar	p1out;
    can_msg_t	msg14 __attribute__ ((packed));
    uchar	p2out;
    can_msg_t	msg15 __attribute__ ((packed));
    uchar	ser_res_addr;
    uchar	resv_cs[0x8000-0x100];	/* 0x8000 is the min size for CS */
} can_reg_t;


#endif /* __SCM_H */

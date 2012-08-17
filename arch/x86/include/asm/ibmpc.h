/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
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

#ifndef __ASM_IBMPC_H_
#define __ASM_IBMPC_H_ 1

/* misc ports in an ibm compatible pc */

#define MASTER_PIC      0x20
#define PIT_BASE	0x40
#define KBDDATA         0x60
#define SYSCTLB         0x62
#define KBDCMD          0x64
#define SYSCTLA         0x92
#define SLAVE_PIC       0xa0

#if 1
#define UART0_BASE     0x3f8
#define UART1_BASE     0x2f8
#else
/* FixMe: uarts swapped */
#define UART0_BASE     0x2f8
#define UART1_BASE     0x3f8
#endif


#endif

/*
 * (C) Copyright 2002
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _NS16550_H_
#define _NS16550_H_

#define NS16550_RBR	0x00
#define NS16550_IER	0x01
#define NS16550_FCR	0x02
#define NS16550_LCR	0x03
#define NS16550_MCR	0x04
#define NS16550_LSR	0x05
#define NS16550_MSR	0x06
#define NS16550_SCR	0x07

#define NS16550_THR	NS16550_RBR
#define NS16550_IIR	NS16550_FCR
#define NS16550_DLL	NS16550_RBR
#define NS16550_DLM	NS16550_IER

#endif

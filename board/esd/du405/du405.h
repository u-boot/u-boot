/*
 * (C) Copyright 2000, 2001
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

/****************************************************************************
 * FLASH Memory Map as used by TQ Monitor:
 *
 *                          Start Address    Length
 * +-----------------------+ 0x4000_0000     Start of Flash -----------------
 * | MON8xx code           | 0x4000_0100     Reset Vector
 * +-----------------------+ 0x400?_????
 * | (unused)              |
 * +-----------------------+ 0x4001_FF00
 * | Ethernet Addresses    |                 0x78
 * +-----------------------+ 0x4001_FF78
 * | (Reserved for MON8xx) |                 0x44
 * +-----------------------+ 0x4001_FFBC
 * | Lock Address          |                 0x04
 * +-----------------------+ 0x4001_FFC0                     ^
 * | Hardware Information  |                 0x40            | MON8xx
 * +=======================+ 0x4002_0000 (sector border)    -----------------
 * | Autostart Header      |                                 | Applications
 * | ...                   |                                 v
 *
 *****************************************************************************/

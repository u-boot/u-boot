/*
 * (C) Copyright 2000
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
 * FLASH Memory Map as used by FADS Monitor:
 *
 *                          Start Address    Length
 * +-----------------------+ 0xFE00_0000     Start of Flash -----------------
 * | MON8xx code           | 0xFE00_0100     Reset Vector
 * +-----------------------+ 0xFE0?_????
 * | (unused)              |
 * +-----------------------+ 0xFE01_FF00
 * | Ethernet Addresses    |                 0x78
 * +-----------------------+ 0xFE01_FF78
 * | (Reserved for MON8xx) |                 0x44
 * +-----------------------+ 0xFE01_FFBC
 * | Lock Address          |                 0x04
 * +-----------------------+ 0xFE01_FFC0                     ^
 * | Hardware Information  |                 0x40            | MON8xx
 * +=======================+ 0xFE02_0000 (sector border)    -----------------
 * | Autostart Header      |                                 | Applications
 * | ...                   |                                 v
 *
 *****************************************************************************/

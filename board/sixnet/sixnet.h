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

/*
 * Memory map:
 *
 *   ff100000 -> ff13ffff : FPGA        CS1
 *   ff030000 -> ff03ffff : EXPANSION   CS7
 *   ff020000 -> ff02ffff : DATA FLASH  CS4
 *   ff018000 -> ff01ffff : UART B      CS6/UPMB
 *   ff010000 -> ff017fff : UART A      CS5/UPMB
 *   ff000000 -> ff00ffff : IMAP                   internal to the MPC855T
 *   f8000000 -> fbffffff : FLASH       CS0        up to 64MB
 *   f4000000 -> f7ffffff : NVSRAM      CS2        up to 64MB
 *   00000000 -> 0fffffff : SDRAM       CS3/UPMA   up to 256MB
 */

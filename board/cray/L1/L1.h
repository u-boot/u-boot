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
 * FLASH Memory Map as used by CRAY L1, 4MB AMD29F032B flash chip
 *
 *                          Start Address    Length
 * +++++++++++++++++++++++++ 0xFFC0_0000     Start of Flash -----------------
 * | Failsafe Linux Image  | 	(1M)
 * +=======================+ 0xFFD0_0000
 * | (Reserved FlashFiles) |	(1M)
 * +=======================+ 0xFFE0_0000
 * | Failsafe RootFS       |	(1M)
 * +=======================+ 0xFFF0_0000
 * |                       |
 * | U N U S E D           |
 * |                       |
 * +-----------------------+ 0xFFFD_0000	U-Boot image header (64 bytes)
 * | environment settings  | 	(64k)
 * +-----------------------+ 0xFFFE_0000	U-Boot image header (64 bytes)
 * | U-Boot                | 0xFFFE_0040    _start of U-Boot
 * |                       | 0xFFFE_FFFC    reset vector - branch to _start
 * +++++++++++++++++++++++++ 0xFFFF_FFFF     End of Flash -----------------
 *****************************************************************************/

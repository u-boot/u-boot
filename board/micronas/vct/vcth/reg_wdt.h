/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
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

#define WDT_BASE		0x000b0000
#define WDT_CR_OFFS		0x00000000
#define WDT_CR(base)		((base) + WDT_CR_OFFS)
#define WDT_TORR_OFFS		0x00000004
#define WDT_TORR(base)		((base) + WDT_TORR_OFFS)

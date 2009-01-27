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

#define DCGU_BASE		0x0004c000
#define DCGU_EN_WDT_RESET_OFFS	0x000000FC
#define DCGU_EN_WDT_RESET(base)	((base) + DCGU_EN_WDT_RESET_OFFS)

/* The magic value to write in order to activate the WDT */
#define DCGU_MAGIC_WDT		0x1909

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

#define GPIO1_BASE		0x00044000
#define GPIO2_BASE		0x00048000

/* Instances */
#define GPIO_INSTANCES		2

/*  Relative offsets of the register adresses */
#define GPIO_SWPORTA_DR_OFFS	0x00000000
#define GPIO_SWPORTA_DR(base)	((base) + GPIO_SWPORTA_DR_OFFS)
#define GPIO_SWPORTA_DDR_OFFS	0x00000004
#define GPIO_SWPORTA_DDR(base)	((base) + GPIO_SWPORTA_DDR_OFFS)
#define GPIO_EXT_PORTA_OFFS	0x00000050
#define GPIO_EXT_PORTA(base)	((base) + GPIO_EXT_PORTA_OFFS)

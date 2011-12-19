/*
 * (C) Copyright 2011
 * Logic Product Development <www.logicpd.com>
 *
 * Author:
 * Peter Barada <peter.barada@logicpd.com>
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
#ifndef _OMAP3LOGIC_H_
#define _OMAP3LOGIC_H_

/*
 * OMAP3 GPMC register settings for CS1 LAN922x
 */
#define NET_LAN92XX_GPMC_CONFIG1	0x00001000
#define NET_LAN92XX_GPMC_CONFIG2	0x00080801
#define NET_LAN92XX_GPMC_CONFIG3	0x00000000
#define NET_LAN92XX_GPMC_CONFIG4	0x08010801
#define NET_LAN92XX_GPMC_CONFIG5	0x00080a0a
#define NET_LAN92XX_GPMC_CONFIG6	0x03000280


const omap3_sysinfo sysinfo = {
	DDR_DISCRETE,
	"Logic DM37x/OMAP35x reference board",
	"NAND",
};


#endif

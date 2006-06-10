/*
 * (C) Copyright 2005
 * BuS Elektronik GmbH & Co.KG <esw@bus-elektonik.de>
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

#ifndef __VCXK_H_
#define __VCXK_H_

extern int init_vcxk(void);
void 	vcxk_loadimage(ulong source);

#define VIDEO_ACKNOWLEDGE_PORT	MCFGPTB_GPTPORT
#define VIDEO_ACKNOWLEDGE_DDR 	MCFGPTB_GPTDDR
#define VIDEO_ACKNOWLEDGE_PIN	0x0001

#define VIDEO_ENABLE_PORT    	MCFGPTB_GPTPORT
#define VIDEO_ENABLE_DDR 	MCFGPTB_GPTDDR
#define VIDEO_ENABLE_PIN	0x0002

#define VIDEO_REQUEST_PORT   	MCFGPTB_GPTPORT
#define VIDEO_REQUEST_DDR 	MCFGPTB_GPTDDR
#define VIDEO_REQUEST_PIN	0x0004

#define VIDEO_Invert_CFG	MCFGPIO_PEPAR
#define VIDEO_Invert_IO		MCFGPIO_PEPAR_PEPA2
#define VIDEO_INVERT_PORT   	MCFGPIO_PORTE
#define VIDEO_INVERT_DDR 	MCFGPIO_DDRE
#define VIDEO_INVERT_PIN	MCFGPIO_PORT2

#endif

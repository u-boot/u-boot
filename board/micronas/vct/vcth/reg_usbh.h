/*
 * (C) Copyright 2009 Stefan Roese <sr@denx.de>, DENX Software Engineering
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

#define USBH_BASE		0x00080000

/* Relative offsets of the register adresses */

#define USBH_CAPLENGTH_OFFS	0x00000100
#define USBH_CAPLENGTH(base)	((base) + USBH_CAPLENGTH_OFFS)
#define USBH_USBCMD_OFFS	0x00000140
#define USBH_USBCMD(base)	((base) + USBH_USBCMD_OFFS)
#define USBH_BURSTSIZE_OFFS	0x00000160
#define USBH_BURSTSIZE(base)	((base) + USBH_BURSTSIZE_OFFS)
#define USBH_USBMODE_OFFS	0x000001A8
#define USBH_USBMODE(base)	((base) + USBH_USBMODE_OFFS)
#define USBH_USBHMISC_OFFS	0x00000200
#define USBH_USBHMISC(base)	((base) + USBH_USBHMISC_OFFS)

/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
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
 *
 */
#ifndef	_CMD_USB_H
#define	_CMD_USB_H

#include <common.h>
#include <command.h>


#if (CONFIG_COMMANDS & CFG_CMD_USB)

#ifdef CONFIG_USB_STORAGE
#define	CMD_TBL_USB	MK_CMD_TBL_ENTRY(					\
	"usb",	4,	5,	1,	do_usb,				\
	"usb     - USB sub-system\n",						\
	"reset - reset (rescan) USB controller\n"					\
	"usb  stop [f]  - stop USB [f]=force stop\n"				\
	"usb  tree  - show USB device tree\n"				\
	"usb  info [dev] - show available USB devices\n"				\
	"usb  scan  - (re-)scan USB bus for storage devices\n"					\
	"usb  device [dev] - show or set current USB storage device\n"			\
	"usb  part [dev] - print partition table of one or all USB storage devices\n"	\
	"usb  read addr blk# cnt - read `cnt' blocks starting at block `blk#'\n"\
	"     to memory address `addr'\n"					\
),


#define	CMD_TBL_USBBOOT	MK_CMD_TBL_ENTRY(					\
	"usbboot",	5,	3,	1,	do_usbboot,					\
	"usbboot - boot from USB device\n",						\
	"loadAddr dev:part\n"			\
),

#else
#define	CMD_TBL_USB	MK_CMD_TBL_ENTRY(					\
	"usb",	4,	5,	1,	do_usb,				\
	"usb     - USB sub-system\n",						\
	"reset - reset (rescan) USB controller\n"					\
	"usb  tree  - show USB device tree\n"				\
	"usb  info [dev] - show available USB devices\n"				\
),

#define CMD_TBL_USBBOOT
#endif

int do_usb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_usbboot (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);


#else
#define CMD_TBL_USB
#define CMD_TBL_USBBOOT
#endif

#endif	/* _CMD_USB_H */


/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
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
 * MII Functions
 */
#ifndef	_CMD_PCI_H
#define _CMD_PCI_H

#if (CONFIG_COMMANDS & CFG_CMD_PCI)
#define CMD_TBL_PCI	MK_CMD_TBL_ENTRY(				\
	"pci",		3,	5,	1,	do_pci,			\
	"pci     - list and access PCI Configuraton Space\n",		\
	"[bus] [long]\n"						\
	"    - short or long list of PCI devices on bus 'bus'\n"	\
	"pci header b.d.f\n"						\
	"    - show header of PCI device 'bus.device.function'\n"	\
	"pci display[.b, .w, .l] b.d.f [address] [# of objects]\n"	\
	"    - display PCI configuration space (CFG)\n"			\
	"pci next[.b, .w, .l] b.d.f address\n"				\
	"    - modify, read and keep CFG address\n"			\
	"pci modify[.b, .w, .l] b.d.f address\n"			\
	"    -  modify, auto increment CFG address\n"			\
	"pci write[.b, .w, .l] b.d.f address value\n"			\
	"    - write to CFG address\n"					\
),

int do_pci (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_PCI
#endif	/* CFG_CMD_PCI */

#endif	/* _CMD_PCI_H */

/*
 * (C) Copyright 2001
 * Kyle Harris, kharris@nexus-tech.net
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

#ifndef	_CMD_AUTOSCRIPT_H_
#define	_CMD_AUTOSCRIPT_H_

#define AUTOSCRIPT_MAGIC	0x09011962

#if (CONFIG_COMMANDS & CFG_CMD_AUTOSCRIPT)

#define	CMD_TBL_AUTOSCRIPT	MK_CMD_TBL_ENTRY(					\
	"autoscr",	5,	2,	0,	do_autoscript,			\
	"autoscr - run script from memory\n",				\
	"[addr] - run script starting at addr. " \
	"A valid autoscr header must be present\n" \
),

int autoscript (ulong addr);
int do_autoscript (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else

#define	CMD_TBL_AUTOSCRIPT

#endif

#endif	/* _CMD_AUTOSCRIPT_H_ */

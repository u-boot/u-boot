/*
 * (C) Copyright 2003
 * Marc Singer, elf@buici.com
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
 * Memory Functions
 */
#ifndef	_CMD_PORTIO_H
#define _CMD_PORTIO_H

#if (CONFIG_COMMANDS & CFG_CMD_PORTIO)

#define CMD_TBL_PORTIO_OUT	MK_CMD_TBL_ENTRY(		      \
	"out",		3,	3,	1,	do_portio_out,	      \
	"out     - write datum to IO port\n",			      \
	"[.b, .w, .l] port value\n    - output to IO port\n"   	      \
),
#define CMD_TBL_PORTIO_IN	MK_CMD_TBL_ENTRY(		      \
 	"in",		2,	2,	1,	do_portio_in,	      \
	"in      - read data from an IO port\n",		      \
	"[.b, .w, .l] port\n"					      \
	"    - read datum from IO port\n"			      \
),

int do_portio_out (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_portio_in  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_PORTIO_OUT
#define CMD_TBL_PORTIO_IN
#endif	/* CFG_CMD_PORTIO */

#endif	/* _CMD_PORTIO_H */

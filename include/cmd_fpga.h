/*
 * (C) Copyright 2001
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
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

/*
 * FPGA support
 */
#ifndef	_CMD_FPGA_H
#define	_CMD_FPGA_H

#include <common.h>
#include <command.h>

#if defined(CONFIG_FPGA) && (CONFIG_COMMANDS & CFG_CMD_FPGA)

#define	CMD_TBL_FPGA	MK_CMD_TBL_ENTRY(				\
	"fpga",      4,    6,     1,     do_fpga,			\
	"fpga	- loadable FPGA image support\n",			\
	"fpga [operation type] [device number] [image address] [image size]\n" \
	"fpga operations:\n" \
	"\tinfo\tlist known device information.\n" \
	"\tload\tLoad device from memory buffer.\n" \
	"\tdump\tLoad device to memory buffer.\n"  \
),

extern int do_fpga (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_FPGA
#endif  /* CONFIG_FPGA && CONFIG_COMMANDS & CFG_CMD_FPGA */

#endif	/* _CMD_FPGA_H */

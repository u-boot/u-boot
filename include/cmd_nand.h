/*
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
 * NAND support
 */
#ifndef	_CMD_NAND_H
#define	_CMD_NAND_H

#include <common.h>
#include <command.h>


#if (CONFIG_COMMANDS & CFG_CMD_NAND)
#define	CMD_TBL_NAND	MK_CMD_TBL_ENTRY(					\
	"nand",	3,	5,	1,	do_nand,				\
	"nand    - NAND sub-system\n",						\
	"info  - show available NAND devices\n"					\
	"nand device [dev] - show or set current device\n"			\
	"nand read[.jffs2]  addr off size\n"					\
	"nand write[.jffs2] addr off size - read/write `size' bytes starting\n"	\
	"    at offset `off' to/from memory address `addr'\n"			\
	"nand erase [clean] [off size] - erase `size' bytes from\n"		\
	"    offset `off' (entire device if not specified)\n"			\
	"nand bad - show bad blocks\n"						\
	"nand read.oob addr off size - read out-of-band data\n"			\
	"nand write.oob addr off size - read out-of-band data\n"		\
),

#define CMD_TBL_NANDBOOT	MK_CMD_TBL_ENTRY(				\
	"nboot", 4,	4,	1,	do_nandboot,				\
	"nboot   - boot from NAND device\n",					\
	"loadAddr dev\n"							\
),

int do_nand (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_nandboot (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_NAND
#define CMD_TBL_NANDBOOT
#endif

#endif	/* _CMD_NAND_H */

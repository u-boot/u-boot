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
 * Disk-On-Chip support
 */
#ifndef	_CMD_DOC_H
#define	_CMD_DOC_H

#include <common.h>
#include <command.h>


#if (CONFIG_COMMANDS & CFG_CMD_DOC)
#define	CMD_TBL_DOC	MK_CMD_TBL_ENTRY(					\
	"doc",	3,	5,	1,	do_doc,					\
	"doc     - Disk-On-Chip sub-system\n",					\
	"info  - show available DOC devices\n"					\
	"doc device [dev] - show or set current device\n"			\
	"doc read  addr off size\n"						\
	"doc write addr off size - read/write `size'"				\
	" bytes starting at offset `off'\n"					\
	"    to/from memory address `addr'\n"					\
	"doc erase off size - erase `size' bytes of DOC from offset `off'\n"	\
),

#define CMD_TBL_DOCBOOT	MK_CMD_TBL_ENTRY(					\
	"docboot", 4,	4,	1,	do_docboot,				\
	"docboot - boot from DOC device\n",					\
	"loadAddr dev\n"							\
),

int do_doc (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_docboot (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_DOC
#define CMD_TBL_DOCBOOT
#endif

#endif	/* _CMD_DOC_H */

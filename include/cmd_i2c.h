/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
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
 * I2C Functions
 */
#ifndef	_CMD_I2C_H
#define _CMD_I2C_H

#if (CONFIG_COMMANDS & CFG_CMD_I2C)
#define CMD_TBL_IMD	MK_CMD_TBL_ENTRY(				\
	"imd",		3,	4,	1,	do_i2c_md,		\
	"imd     - i2c memory display\n",				\
	"chip address[.0, .1, .2] [# of objects]\n    - i2c memory display\n" \
),
#define CMD_TBL_IMM	MK_CMD_TBL_ENTRY(				\
 	"imm",		3,	3,	1,	do_i2c_mm,		\
	"imm     - i2c memory modify (auto-incrementing)\n",		\
	"chip address[.0, .1, .2]\n"					\
	"    - memory modify, auto increment address\n"			\
),
#define CMD_TBL_INM	MK_CMD_TBL_ENTRY(				\
	"inm",		3,	3,	1,	do_i2c_nm,		\
	"inm     - memory modify (constant address)\n",			\
	"chip address[.0, .1, .2]\n    - memory modify, read and keep address\n" \
),
#define CMD_TBL_IMW	MK_CMD_TBL_ENTRY(				\
	"imw",		3,	5,	1,	do_i2c_mw,		\
	"imw     - memory write (fill)\n",				\
	"chip address[.0, .1, .2] value [count]\n    - memory write (fill)\n" \
),
#define	CMD_TBL_ICRC	MK_CMD_TBL_ENTRY(				\
	"icrc32",	4,	5,	1,	do_i2c_crc,		\
	"icrc32  - checksum calculation\n",				\
	"chip address[.0, .1, .2] count\n    - compute CRC32 checksum\n" \
),
#define CMD_TBL_IPROBE	MK_CMD_TBL_ENTRY(				\
	"iprobe",	3,	1,	1,	do_i2c_probe,		\
	"iprobe  - probe to discover valid I2C chip addresses\n",	\
	"\n    -discover valid I2C chip addresses\n"			\
),
/*
 * Require full name for "iloop" because it is an infinite loop!
 */
#define CMD_TBL_ILOOP	MK_CMD_TBL_ENTRY(				\
	"iloop",	5,	5,	1,	do_i2c_loop,		\
	"iloop   - infinite loop on address range\n",			\
	"chip address[.0, .1, .2] [# of objects]\n"			\
	"    - loop, reading a set of addresses\n"			\
),
#if (CONFIG_COMMANDS & CFG_CMD_SDRAM)
#define CMD_TBL_ISDRAM	MK_CMD_TBL_ENTRY(				\
	"isdram",	6,	2,	1,	do_sdram,		\
	"isdram  - print SDRAM configuration information\n",		\
	"chip\n    - print SDRAM configuration information\n"		\
	"      (valid chip values 50..57)\n"				\
),
#else
#define CMD_TBL_ISDRAM
#endif


int do_i2c_md(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_i2c_mm(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_i2c_nm(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_i2c_mw(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_i2c_crc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_i2c_probe(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_i2c_loop(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_sdram(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else
#define CMD_TBL_IMD
#define CMD_TBL_IMM
#define CMD_TBL_INM
#define CMD_TBL_IMW
#define CMD_TBL_ICRC
#define CMD_TBL_IPROBE
#define CMD_TBL_ILOOP
#define CMD_TBL_ISDRAM
#endif	/* CFG_CMD_MEMORY */

#endif	/* _CMD_I2C_H */

/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * PowerPC 8xx/8260 Internal Memory Map commands
 */
#ifndef	_CMD_IMMAP_H
#define	_CMD_IMMAP_H

#if (CONFIG_COMMANDS & CFG_CMD_IMMAP) && \
    (defined(CONFIG_8xx) || defined(CONFIG_8260))

#define	CMD_TBL_SIUINFO		MK_CMD_TBL_ENTRY(			\
	"siuinfo",	3,	1,	1,	do_siuinfo,		\
	"siuinfo - print System Interface Unit (SIU) registers\n",	\
	NULL								\
),

#define CMD_TBL_MEMCINFO	MK_CMD_TBL_ENTRY(			\
	"memcinfo",	4,	1,	1,	do_memcinfo,		\
	"memcinfo- print Memory Controller registers\n",		\
	NULL								\
),

#define CMD_TBL_SITINFO		MK_CMD_TBL_ENTRY(			\
	"sitinfo",	3,	1,	1,	do_sitinfo,		\
	"sitinfo - print System Integration Timers (SIT) registers\n",	\
	NULL								\
),

#ifdef CONFIG_8260
#define	CMD_TBL_ICINFO		MK_CMD_TBL_ENTRY(			\
	"icinfo",	3,	1,	1,	do_icinfo,		\
	"icinfo  - print Interrupt Controller registers\n",		\
	NULL								\
),
#endif

#define	CMD_TBL_CARINFO		MK_CMD_TBL_ENTRY(			\
	"carinfo",	3,	1,	1,	do_carinfo,		\
	"carinfo - print Clocks and Reset registers\n",			\
	NULL								\
),

#define	CMD_TBL_IOPINFO		MK_CMD_TBL_ENTRY(			\
	"iopinfo",	4,	1,	1,	do_iopinfo,		\
	"iopinfo - print I/O Port registers\n",				\
	NULL								\
),

#define	CMD_TBL_IOPSET 		MK_CMD_TBL_ENTRY(			\
	"iopset",	4,	5,	0,	do_iopset,		\
	"iopset - set I/O Port registers\n",				\
	"PORT PIN CMD VALUE\nPORT: A-D, PIN: 0-31, CMD: [dat|dir|odr|sor], VALUE: 0|1" \
),

#define	CMD_TBL_DMAINFO		MK_CMD_TBL_ENTRY(			\
	"dmainfo",	3,	1,	1,	do_dmainfo,		\
	"dmainfo - print SDMA/IDMA registers\n",			\
	NULL								\
),

#define	CMD_TBL_FCCINFO		MK_CMD_TBL_ENTRY(			\
	"fccinfo",	3,	1,	1,	do_fccinfo,		\
	"fccinfo - print FCC registers\n",				\
	NULL								\
),

#define	CMD_TBL_BRGINFO		MK_CMD_TBL_ENTRY(			\
	"brginfo",	3,	1,	1,	do_brginfo,		\
	"brginfo - print Baud Rate Generator (BRG) registers\n",	\
	NULL								\
),

#define	CMD_TBL_I2CINFO		MK_CMD_TBL_ENTRY(			\
	"i2cinfo",	4,	1,	1,	do_i2cinfo,		\
	"i2cinfo - print I2C registers\n",				\
	NULL								\
),

#define	CMD_TBL_SCCINFO		MK_CMD_TBL_ENTRY(			\
	"sccinfo",	3,	1,	1,	do_sccinfo,		\
	"sccinfo - print SCC registers\n",				\
	NULL								\
),

#define	CMD_TBL_SMCINFO		MK_CMD_TBL_ENTRY(			\
	"smcinfo",	3,	1,	1,	do_smcinfo,		\
	"smcinfo - print SMC registers\n",				\
	NULL								\
),

#define	CMD_TBL_SPIINFO		MK_CMD_TBL_ENTRY(			\
	"spiinfo",	3,	1,	1,	do_spiinfo,		\
	"spiinfo - print Serial Peripheral Interface (SPI) registers\n",\
	NULL								\
),

#define	CMD_TBL_MUXINFO		MK_CMD_TBL_ENTRY(			\
	"muxinfo",	3,	1,	1,	do_muxinfo,		\
	"muxinfo - print CPM Multiplexing registers\n",			\
	NULL								\
),

#define	CMD_TBL_SIINFO		MK_CMD_TBL_ENTRY(			\
	"siinfo",	3,	1,	1,	do_siinfo,		\
	"siinfo  - print Serial Interface (SI) registers\n",		\
	NULL								\
),

#define	CMD_TBL_MCCINFO		MK_CMD_TBL_ENTRY(			\
	"mccinfo",	3,	1,	1,	do_mccinfo,		\
	"mccinfo - print MCC registers\n",				\
	NULL								\
),

int do_siuinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_memcinfo(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_sitinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#ifdef CONFIG_8260
int do_icinfo  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#endif
int do_carinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_iopinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_iopset  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_dmainfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_fccinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_brginfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_i2cinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_sccinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_smcinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_spiinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_muxinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_siinfo  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_mccinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#else

#define CMD_TBL_SIUINFO
#define CMD_TBL_MEMCINFO
#define CMD_TBL_SITINFO
#ifdef CONFIG_8260
#define	CMD_TBL_ICINFO
#endif
#define	CMD_TBL_CARINFO
#define	CMD_TBL_IOPINFO
#define	CMD_TBL_IOPSET
#define	CMD_TBL_DMAINFO
#define	CMD_TBL_FCCINFO
#define	CMD_TBL_BRGINFO
#define	CMD_TBL_I2CINFO
#define	CMD_TBL_SCCINFO
#define	CMD_TBL_SMCINFO
#define	CMD_TBL_SPIINFO
#define	CMD_TBL_MUXINFO
#define	CMD_TBL_SIINFO
#define	CMD_TBL_MCCINFO

#endif	/* CFG_CMD_IMMAP && (CONFIG_8xx || CONFIG_8260) */

#endif	/* _CMD_IMMAP_H */

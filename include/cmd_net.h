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
 * Network boot support
 */
#ifndef	_CMD_NET_H
#define	_CMD_NET_H

#if (CONFIG_COMMANDS & CFG_CMD_NET)
#define	CMD_TBL_BOOTP	MK_CMD_TBL_ENTRY(					\
	"bootp",	5,	3,	1,	do_bootp,			\
	"bootp   - boot image via network using BootP/TFTP protocol\n",		\
	"[loadAddress] [bootfilename]\n" 		 			\
),
int do_bootp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#define	CMD_TBL_TFTPB	MK_CMD_TBL_ENTRY(					\
	"tftpboot",	4,	3,	1,	do_tftpb,			\
	"tftpboot- boot image via network using TFTP protocol\n"		\
	"               and env variables ipaddr and serverip\n",		\
	"[loadAddress] [bootfilename]\n" 		 			\
),

int do_tftpb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);


#define	CMD_TBL_RARPB	MK_CMD_TBL_ENTRY(					\
	"rarpboot",	4,	3,	1,	do_rarpb,			\
	"rarpboot- boot image via network using RARP/TFTP protocol\n",		\
	"[loadAddress] [bootfilename]\n" 		 			\
),

int do_rarpb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#if (CONFIG_COMMANDS & CFG_CMD_DHCP)
#define	CMD_TBL_DHCP	MK_CMD_TBL_ENTRY(					\
	"dhcp",		4,	3,	1,	do_dhcp,			\
	"dhcp    - invoke DHCP client to obtain IP/boot params\n",		\
	"[loadAddress] [bootfilename]\n" 		 			\
),

int do_dhcp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#else
#define CMD_TBL_DHCP
#endif	/* CFG_CMD_DHCP */

#if (CONFIG_COMMANDS & CFG_CMD_PING)
#define	CMD_TBL_PING	MK_CMD_TBL_ENTRY(					\
	"ping",		4,	2,	1,	do_ping,			\
	"ping    - check if host is reachable\n",				\
	"host\n" 					 			\
),

int do_ping (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#else
#define CMD_TBL_PING
#endif	/* CFG_CMD_PING */

#else
#define CMD_TBL_BOOTP
#define CMD_TBL_TFTPB
#define CMD_TBL_RARPB
#define CMD_TBL_DHCP
#define CMD_TBL_PING
#endif	/* CFG_CMD_NET */

#endif

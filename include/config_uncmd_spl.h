/*
 * (C) Copyright 2012
 * Ilya Yanok, ilya.yanok@gmail.com
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 *
 * We don't use any commands in SPL, but generic networking code
 * has some features enabled/disabled based on CONFIG_CMD_*
 * options. As we want a minimal set of features included
 * into network SPL image, we undefine some config options here.
 */

#ifndef __CONFIG_UNCMD_SPL_H__
#define __CONFIG_UNCMD_SPL_H__

#ifdef CONFIG_SPL_BUILD
/* SPL needs only BOOTP + TFTP so undefine other stuff to save space */
#undef CONFIG_CMD_CDP
#undef CONFIG_CMD_DHCP
#undef CONFIG_CMD_DNS
#undef CONFIG_CMD_LINK_LOCAL
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_PING
#undef CONFIG_CMD_RARP
#undef CONFIG_CMD_SNTP
#undef CONFIG_CMD_TFTPPUT
#undef CONFIG_CMD_TFTPSRV
#endif /* CONFIG_SPL_BUILD */
#endif /* __CONFIG_UNCMD_SPL_H__ */

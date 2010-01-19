/*
 * File:         board/bf561-acvilon/bf561-acvilon.c
 * Based on:     board/bf561-ezkit/bf561-ezkit.c
 * Author:
 *
 * Created:      2009-06-23
 * Description:  Acvilon System On Module board file
 *
 * Modified:
 *               Copyright 2009 CJSC "NII STT", http://www.niistt.ru/
 *               Copyright (c) 2005-2008 Analog Devices Inc.
 *
 *               (C) Copyright 2000-2004
 *               Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Bugs:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see the file COPYING, or write
 * to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <common.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board:  CJSC \"NII STT\"-=Acvilon Platform=- [U-Boot]\n");
	printf("       Support: http://www.niistt.ru/\n");
	return 0;
}

#ifdef CONFIG_SMC911X
int board_eth_init(bd_t *bis)
{
	return smc911x_initialize(0, CONFIG_SMC911X_BASE);
}
#endif

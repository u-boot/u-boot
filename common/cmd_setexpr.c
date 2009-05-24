/*
 * Copyright 2008 Freescale Semiconductor, Inc.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * This file provides a shell like 'expr' function to return.
 */

#include <common.h>
#include <config.h>
#include <command.h>

int do_setexpr(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong a, b;
	char buf[16];

	/* Validate arguments */
	if ((argc != 5) || (strlen(argv[3]) != 1)) {
		cmd_usage(cmdtp);
		return 1;
	}

	a = simple_strtoul(argv[2], NULL, 16);
	b = simple_strtoul(argv[4], NULL, 16);

	switch (argv[3][0]) {
	case '|': sprintf(buf, "%lx", (a | b)); break;
	case '&': sprintf(buf, "%lx", (a & b)); break;
	case '+': sprintf(buf, "%lx", (a + b)); break;
	case '^': sprintf(buf, "%lx", (a ^ b)); break;
	case '-': sprintf(buf, "%lx", (a - b)); break;
	case '*': sprintf(buf, "%lx", (a * b)); break;
	case '/': sprintf(buf, "%lx", (a / b)); break;
	case '%': sprintf(buf, "%lx", (a % b)); break;
	default:
		printf("invalid op\n");
		return 1;
	}

	setenv(argv[1], buf);

	return 0;
}

U_BOOT_CMD(
	setexpr, 5, 0, do_setexpr,
	"set environment variable as the result of eval expression",
	"name value1 <op> value2\n"
	"    - set environment variable 'name' to the result of the evaluated\n"
	"      express specified by <op>.  <op> can be &, |, ^, +, -, *, /, %"
);

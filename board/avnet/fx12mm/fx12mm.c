/*
 * (C) Copyright 2008
 *
 * Author: Xilinx Inc.
 *
 * Modified by:
 *  Georg Schardt <schardt@team-ctech.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <config.h>
#include <common.h>
#include <asm/processor.h>

int checkboard(void)
{
	char buf[64];
	int i;
	int l = getenv_f("serial#", buf, sizeof(buf));

	if (l < 0) {
		printf("Avnet Virtex4 FX12 with no serial #");
	} else {
		printf("Avnet Virtex4 FX12 Minimodul # ");
		for (i = 0; i < l; ++i) {
			if (buf[i] == ' ')
				break;
			putc(buf[i]);
		}
	}
	putc('\n');
	return 0;
}

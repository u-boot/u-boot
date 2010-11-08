/*
 * (C) Copyright 2010
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

#include <common.h>
#include <asm/ppc4xx_config.h>

struct ppc4xx_config ppc4xx_config_val[] = {
	{
		"600-67", "CPU: 600 PLB: 200 OPB:  67 EBC:  67",
		{
			0x86, 0x80, 0xce, 0x1f, 0x7d, 0x80, 0x00, 0xe0,
			0x40, 0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
		}
	},
	{
		"600-100", "CPU: 600 PLB: 200 OPB: 100 EBC: 100",
		{
			0x86, 0x80, 0xce, 0x1f, 0x79, 0x80, 0x00, 0xa0,
			0x40, 0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
		}
	},
	{
		"667", "CPU: 667 PLB: 166 OPB:  83 EBC:  83",
		{
			0x06, 0x80, 0xbb, 0x14, 0x99, 0x82, 0x00, 0xa0,
			0x40, 0x88, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
		}
	},
	{
		"800", "CPU: 800 PLB: 200 OPB: 100 EBC: 100",
		{
			0x86, 0x80, 0xba, 0x14, 0x99, 0x80, 0x00, 0xa0,
			0x40, 0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
		}
	},
	{
		"1000", "CPU:1000 PLB: 200 OPB: 100 EBC: 100",
		{
			0x86, 0x82, 0x96, 0x19, 0xb9, 0x80, 0x00, 0xa0,
			0x40, 0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
		}
	},
	{
		"1066", "CPU:1066 PLB: 266 OPB:  88 EBC:  88",
		{
			0x86, 0x80, 0xb3, 0x01, 0x9d, 0x80, 0x00, 0xa0,
			0x40, 0x08, 0x23, 0x50, 0x0d, 0x05, 0x00, 0x00
		}
	},
};

int ppc4xx_config_count = ARRAY_SIZE(ppc4xx_config_val);

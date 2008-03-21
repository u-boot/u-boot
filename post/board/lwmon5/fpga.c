/*
 * (C) Copyright 2008 Dmitry Rakhchev, EmCraft Systems, rda@emcraft.com
 *
 * Developed for DENX Software Engineering GmbH
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
#include <common.h>

#ifdef CONFIG_POST

/* This test performs testing of FPGA SCRATCH register,
 * gets FPGA version and run get_ram_size() on FPGA memory
 */

#include <post.h>

#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define FPGA_SCRATCH_REG	0xC4000050
#define FPGA_VERSION_REG	0xC4000040
#define FPGA_RAM_START		0xC4200000
#define FPGA_RAM_END		0xC4203FFF
#define FPGA_STAT		0xC400000C

#if CONFIG_POST & CFG_POST_BSPEC3

static int one_scratch_test(uint value)
{
	uint read_value;
	int ret = 0;

	out_be32((void *)FPGA_SCRATCH_REG, value);
	/* read other location (protect against data lines capacity) */
	ret = in_be16((void *)FPGA_VERSION_REG);
	/* verify test pattern */
	read_value = in_be32((void *)FPGA_SCRATCH_REG);
	if (read_value != value) {
		post_log("FPGA SCRATCH test failed write %08X, read %08X\n",
			value, read_value);
		ret = 1;
	}

	return ret;
}

/* Verify FPGA, get version & memory size */
int fpga_post_test(int flags)
{
	uint   old_value;
	ushort version;
	uint   read_value;
	int    ret = 0;

	post_log("\n");
	old_value = in_be32((void *)FPGA_SCRATCH_REG);

	if (one_scratch_test(0x55555555))
		ret = 1;
	if (one_scratch_test(0xAAAAAAAA))
		ret = 1;

	out_be32((void *)FPGA_SCRATCH_REG, old_value);

	version = in_be16((void *)FPGA_VERSION_REG);
	post_log("FPGA : version %u.%u\n",
		(version >> 8) & 0xFF, version & 0xFF);

	/* Enable write to FPGA RAM */
	out_be32((void *)FPGA_STAT, in_be32((void *)FPGA_STAT) | 0x1000);

	read_value = get_ram_size((void *)CFG_FPGA_BASE_1, 0x4000);
	post_log("FPGA RAM size: %d bytes\n", read_value);

	return ret;
}

#endif /* CONFIG_POST & CFG_POST_BSPEC3 */
#endif /* CONFIG_POST */

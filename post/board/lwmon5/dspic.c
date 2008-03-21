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

/* There are two tests for dsPIC currently implemented:
 * 1. dsPIC ready test. Done in board_early_init_f(). Only result verified here.
 * 2. dsPIC POST result test.  This test gets dsPIC POST codes and version.
 */

#include <post.h>

#include <i2c.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define DSPIC_POST_ERROR_REG	0x800
#define DSPIC_SYS_ERROR_REG	0x802
#define DSPIC_VERSION_REG	0x804

#if CONFIG_POST & CFG_POST_BSPEC1

/* Verify that dsPIC ready test done early at hw init passed ok */
int dspic_init_post_test(int flags)
{
	if (in_be32((void *)CFG_DSPIC_TEST_ADDR) & CFG_DSPIC_TEST_MASK) {
		post_log("dsPIC init test failed\n");
		return 1;
	}

	return 0;
}

#endif /* CONFIG_POST & CFG_POST_BSPEC1 */

#if CONFIG_POST & CFG_POST_BSPEC2
/* Read a register from the dsPIC. */
int dspic_read(ushort reg)
{
	uchar buf[2];

	if (i2c_read(CFG_I2C_DSPIC_IO_ADDR, reg, 2, buf, 2))
		return -1;

	return (uint)((buf[0] << 8) | buf[1]);
}

/* Verify error codes regs, display version */
int dspic_post_test(int flags)
{
	int data;
	int ret = 0;

	post_log("\n");
	data = dspic_read(DSPIC_VERSION_REG);
	if (data == -1) {
		post_log("dsPIC : failed read version\n");
		ret = 1;
	} else {
		post_log("dsPIC version: %u.%u\n",
			(data >> 8) & 0xFF, data & 0xFF);
	}

	data = dspic_read(DSPIC_POST_ERROR_REG);
	if (data != 0) ret = 1;
	if (data == -1) {
		post_log("dsPIC : failed read POST code\n");
	} else {
		post_log("dsPIC POST code 0x%04X\n", data);
	}

	data = dspic_read(DSPIC_SYS_ERROR_REG);
	if (data == -1) {
		post_log("dsPIC : failed read system error\n");
		ret = 1;
	} else {
		post_log("dsPIC SYS-ERROR code: 0x%04X\n", data);
	}

	return ret;
}

#endif /* CONFIG_POST & CFG_POST_BSPEC2 */
#endif /* CONFIG_POST */

/*
 * (C) Copyright 2008 Dmitry Rakhchev, EmCraft Systems, rda@emcraft.com
 *
 * Developed for DENX Software Engineering GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

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
#define DSPIC_SYS_VERSION_REG	0x804
#define DSPIC_FW_VERSION_REG	0x808

#if CONFIG_POST & CONFIG_SYS_POST_BSPEC1

/* Verify that dsPIC ready test done early at hw init passed ok */
int dspic_init_post_test(int flags)
{
	if (in_be32((void *)CONFIG_SYS_DSPIC_TEST_ADDR) &
	    CONFIG_SYS_DSPIC_TEST_MASK) {
		post_log("dsPIC init test failed\n");
		return 1;
	}

	return 0;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_BSPEC1 */

#if CONFIG_POST & CONFIG_SYS_POST_BSPEC2
/* Read a register from the dsPIC. */
int dspic_read(ushort reg, ushort *data)
{
	uchar buf[sizeof(*data)];
	int rval;

	if (i2c_read(CONFIG_SYS_I2C_DSPIC_IO_ADDR, reg, 2, buf, 2))
		return -1;
	rval = i2c_read(CONFIG_SYS_I2C_DSPIC_IO_ADDR, reg, sizeof(reg),
			buf, sizeof(*data));
	*data = (buf[0] << 8) | buf[1];

	return rval;
}

/* Verify error codes regs, display version */
int dspic_post_test(int flags)
{
	ushort data;
	int ret = 0;

	post_log("\n");

	/* read dspic FW-Version */
	if (dspic_read(DSPIC_FW_VERSION_REG, &data)) {
		post_log("dsPIC: failed read FW-Version\n");
		ret = 1;
	} else {
		post_log("dsPIC FW-Version:  %u.%u\n",
			 (data >> 8) & 0xFF, data & 0xFF);
	}

	/* read dspic SYS-Version */
	if (dspic_read(DSPIC_SYS_VERSION_REG, &data)) {
		post_log("dsPIC: failed read version\n");
		ret = 1;
	} else {
		post_log("dsPIC SYS-Version: %u.%u\n",
			 (data >> 8) & 0xFF, data & 0xFF);
	}

	/* read dspic POST error code */
	if (dspic_read(DSPIC_POST_ERROR_REG, &data)) {
		post_log("dsPIC: failed read POST code\n");
		ret = 1;
	} else {
		post_log("dsPIC POST-ERROR   code:  0x%04X\n", data);
	}

	/* read dspic SYS error code */
	if ((data = dspic_read(DSPIC_SYS_ERROR_REG, &data))) {
		post_log("dsPIC: failed read system error\n");
		ret = 1;
	} else {
		post_log("dsPIC SYS-ERROR    code:  0x%04X\n", data);
	}

	return ret;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_BSPEC2 */

/*
 * (C) Copyright 2002
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

#include <common.h>

#ifdef CONFIG_POST

/*
 * I2C test
 *
 * For verifying the I2C bus, a full I2C bus scanning is performed.
 * If any I2C device is found, the test is considered as passed,
 * otherwise failed.
 */

#include <post.h>
#include <i2c.h>

#if CONFIG_POST & CFG_POST_I2C

int i2c_post_test (int flags)
{
	unsigned int i;
	unsigned int chips = 0;

	for (i = 0; i < 128; i++) {
		if (i2c_probe (i) == 0)
			chips++;
	}

	return chips > 0 ? 0 : -1;
}

#endif /* CONFIG_POST & CFG_POST_I2C */
#endif /* CONFIG_POST */

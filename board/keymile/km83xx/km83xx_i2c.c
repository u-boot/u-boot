/*
 * (C) Copyright 2011
 * Holger Brunck, Keymile GmbH Hannover, holger.brunck@keymile.com
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
#include <i2c.h>
#include <asm/io.h>
#include <linux/ctype.h>
#include "../common/common.h"

static void i2c_write_start_seq(void)
{
	struct fsl_i2c *dev;
	dev = (struct fsl_i2c *) (CONFIG_SYS_IMMR + CONFIG_SYS_I2C_OFFSET);
	udelay(DELAY_ABORT_SEQ);
	out_8(&dev->cr, (I2C_CR_MEN | I2C_CR_MSTA));
	udelay(DELAY_ABORT_SEQ);
	out_8(&dev->cr, (I2C_CR_MEN));
}

int i2c_make_abort(void)
{
	struct fsl_i2c *dev;
	dev = (struct fsl_i2c *) (CONFIG_SYS_IMMR + CONFIG_SYS_I2C_OFFSET);
	uchar   last;
	int     nbr_read = 0;
	int     i = 0;
	int	    ret = 0;

	/* wait after each operation to finsh with a delay */
	out_8(&dev->cr, (I2C_CR_MSTA));
	udelay(DELAY_ABORT_SEQ);
	out_8(&dev->cr, (I2C_CR_MEN | I2C_CR_MSTA));
	udelay(DELAY_ABORT_SEQ);
	in_8(&dev->dr);
	udelay(DELAY_ABORT_SEQ);
	last = in_8(&dev->dr);
	nbr_read++;

	/*
	 * do read until the last bit is 1, but stop if the full eeprom is
	 * read.
	 */
	while (((last & 0x01) != 0x01) &&
		(nbr_read < CONFIG_SYS_IVM_EEPROM_MAX_LEN)) {
		udelay(DELAY_ABORT_SEQ);
		last = in_8(&dev->dr);
		nbr_read++;
	}
	if ((last & 0x01) != 0x01)
		ret = -2;
	if ((last != 0xff) || (nbr_read > 1))
		printf("[INFO] i2c abort after %d bytes (0x%02x)\n",
			nbr_read, last);
	udelay(DELAY_ABORT_SEQ);
	out_8(&dev->cr, (I2C_CR_MEN));
	udelay(DELAY_ABORT_SEQ);
	/* clear status reg */
	out_8(&dev->sr, 0);

	for (i = 0; i < 5; i++)
		i2c_write_start_seq();
	if (ret != 0)
		printf("[ERROR] i2c abort failed after %d bytes (0x%02x)\n",
			nbr_read, last);

	return ret;
}

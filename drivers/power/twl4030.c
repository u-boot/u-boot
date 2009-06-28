/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix at windriver.com>
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
 * Derived from code on omapzoom, git://git.omapzoom.com/repo/u-boot.git
 *
 * Copyright (C) 2007-2009 Texas Instruments, Inc.
 */

#include <twl4030.h>

/*
 * Power Reset
 */
void twl4030_power_reset_init(void)
{
	u8 val = 0;
	if (twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER, &val,
				TWL4030_PM_MASTER_P1_SW_EVENTS)) {
		printf("Error:TWL4030: failed to read the power register\n");
		printf("Could not initialize hardware reset\n");
	} else {
		val |= TWL4030_PM_MASTER_SW_EVENTS_STOPON_PWRON;
		if (twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, val,
					 TWL4030_PM_MASTER_P1_SW_EVENTS)) {
			printf("Error:TWL4030: failed to write the power register\n");
			printf("Could not initialize hardware reset\n");
		}
	}
}



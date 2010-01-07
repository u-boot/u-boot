/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>

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

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#if defined(CONFIG_I2C_ENV_EEPROM_BUS)
#include <i2c.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

env_t *env_ptr = NULL;

char * env_name_spec = "EEPROM";
int env_eeprom_bus = -1;

static int eeprom_bus_read (unsigned dev_addr, unsigned offset, uchar *buffer,
				unsigned cnt)
{
	int rcode;
#if defined(CONFIG_I2C_ENV_EEPROM_BUS)
	int old_bus = i2c_get_bus_num();

	if (gd->flags & GD_FLG_RELOC) {
		if (env_eeprom_bus == -1) {
			I2C_MUX_DEVICE *dev = NULL;
			dev = i2c_mux_ident_muxstring(
				(uchar *)CONFIG_I2C_ENV_EEPROM_BUS);
			if (dev != NULL) {
				env_eeprom_bus = dev->busid;
			} else
				printf ("error adding env eeprom bus.\n");
		}
		if (old_bus != env_eeprom_bus) {
			i2c_set_bus_num(env_eeprom_bus);
			old_bus = env_eeprom_bus;
		}
	} else {
		rcode = i2c_mux_ident_muxstring_f(
				(uchar *)CONFIG_I2C_ENV_EEPROM_BUS);
	}
#endif

	rcode = eeprom_read (dev_addr, offset, buffer, cnt);
#if defined(CONFIG_I2C_ENV_EEPROM_BUS)
	if (old_bus != env_eeprom_bus)
		i2c_set_bus_num(old_bus);
#endif
	return rcode;
}

static int eeprom_bus_write (unsigned dev_addr, unsigned offset, uchar *buffer,
				unsigned cnt)
{
	int rcode;
#if defined(CONFIG_I2C_ENV_EEPROM_BUS)
	int old_bus = i2c_get_bus_num();

	rcode = i2c_mux_ident_muxstring_f((uchar *)CONFIG_I2C_ENV_EEPROM_BUS);
#endif
	rcode = eeprom_write (dev_addr, offset, buffer, cnt);
#if defined(CONFIG_I2C_ENV_EEPROM_BUS)
	i2c_set_bus_num(old_bus);
#endif
	return rcode;
}

uchar env_get_char_spec (int index)
{
	uchar c;

	eeprom_bus_read (CONFIG_SYS_DEF_EEPROM_ADDR,
		     CONFIG_ENV_OFFSET+index+offsetof(env_t,data),
		     &c, 1);

	return (c);
}

void env_relocate_spec (void)
{
	eeprom_bus_read (CONFIG_SYS_DEF_EEPROM_ADDR,
		     CONFIG_ENV_OFFSET,
		     (uchar*)env_ptr,
		     CONFIG_ENV_SIZE);
}

int saveenv(void)
{
	return eeprom_bus_write (CONFIG_SYS_DEF_EEPROM_ADDR,
			     CONFIG_ENV_OFFSET,
			     (uchar *)env_ptr,
			     CONFIG_ENV_SIZE);
}

/************************************************************************
 * Initialize Environment use
 *
 * We are still running from ROM, so data use is limited
 * Use a (moderately small) buffer on the stack
 */
int env_init(void)
{
	ulong crc, len, new;
	unsigned off;
	uchar buf[64];

	eeprom_init ();	/* prepare for EEPROM read/write */

	/* read old CRC */
	eeprom_bus_read (CONFIG_SYS_DEF_EEPROM_ADDR,
		     CONFIG_ENV_OFFSET+offsetof(env_t,crc),
		     (uchar *)&crc, sizeof(ulong));

	new = 0;
	len = ENV_SIZE;
	off = offsetof(env_t,data);
	while (len > 0) {
		int n = (len > sizeof(buf)) ? sizeof(buf) : len;

		eeprom_bus_read (CONFIG_SYS_DEF_EEPROM_ADDR,
				CONFIG_ENV_OFFSET + off, buf, n);
		new = crc32 (new, buf, n);
		len -= n;
		off += n;
	}

	if (crc == new) {
		gd->env_addr  = offsetof(env_t,data);
		gd->env_valid = 1;
	} else {
		gd->env_addr  = 0;
		gd->env_valid = 0;
	}

	return (0);
}

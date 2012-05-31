/*
 * (C) Copyright 2009 mGine co.
 * unsik Kim <donari75@gmail.com>
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
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <mg_disk.h>

/* references to names in env_common.c */
extern uchar default_environment[];

char *env_name_spec = "MG_DISK";

env_t *env_ptr = 0;

DECLARE_GLOBAL_DATA_PTR;

uchar env_get_char_spec(int index)
{
	return (*((uchar *)(gd->env_addr + index)));
}

void env_relocate_spec(void)
{
	char buf[CONFIG_ENV_SIZE];
	unsigned int err, rc;

	err = mg_disk_init();
	if (err) {
		set_default_env("!mg_disk_init error");
		return;
	}

	err = mg_disk_read(CONFIG_ENV_ADDR, buf, CONFIG_ENV_SIZE);
	if (err) {
		set_default_env("!mg_disk_read error");
		return;
	}

	env_import(buf, 1);
}

int saveenv(void)
{
	unsigned int err;

	env_ptr->crc = crc32(0, env_ptr->data, ENV_SIZE);
	err = mg_disk_write(CONFIG_ENV_ADDR, (u_char *)env_ptr,
			CONFIG_ENV_SIZE);
	if (err)
		puts("*** Warning - mg_disk_write error\n\n");

	return err;
}

int env_init(void)
{
	/* use default */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}

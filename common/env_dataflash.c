/*
 * LowLevel function for DataFlash environment support
 * Author : Gilles Gastaldi (Atmel)
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
#include <dataflash.h>
#include <search.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

env_t *env_ptr;

char *env_name_spec = "dataflash";

uchar env_get_char_spec(int index)
{
	uchar c;

	read_dataflash(CONFIG_ENV_ADDR + index + offsetof(env_t, data),
			1, (char *)&c);
	return c;
}

void env_relocate_spec(void)
{
	char buf[CONFIG_ENV_SIZE];

	read_dataflash(CONFIG_ENV_ADDR, CONFIG_ENV_SIZE, buf);

	env_import(buf, 1);
}

#ifdef CONFIG_ENV_OFFSET_REDUND
#error No support for redundant environment on dataflash yet!
#endif

int saveenv(void)
{
	env_t	env_new;
	ssize_t	len;
	char	*res;

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}
	env_new.crc = crc32(0, env_new.data, ENV_SIZE);

	return write_dataflash(CONFIG_ENV_ADDR,
				(unsigned long)&env_new,
				CONFIG_ENV_SIZE);
}

/*
 * Initialize environment use
 *
 * We are still running from ROM, so data use is limited.
 * Use a (moderately small) buffer on the stack
 */
int env_init(void)
{
	ulong crc, len = ENV_SIZE, new = 0;
	unsigned off;
	uchar buf[64];

	if (gd->env_valid)
		return 0;

	AT91F_DataflashInit();	/* prepare for DATAFLASH read/write */

	/* read old CRC */
	read_dataflash(CONFIG_ENV_ADDR + offsetof(env_t, crc),
		sizeof(ulong), (char *)&crc);

	off = offsetof(env_t, data);
	while (len > 0) {
		int n = (len > sizeof(buf)) ? sizeof(buf) : len;

		read_dataflash(CONFIG_ENV_ADDR + off, n, (char *)buf);

		new = crc32(new, buf, n);
		len -= n;
		off += n;
	}

	if (crc == new) {
		gd->env_addr	= offsetof(env_t, data);
		gd->env_valid	= 1;
	} else {
		gd->env_addr	= (ulong)&default_environment[0];
		gd->env_valid	= 0;
	}

	return 0;
}

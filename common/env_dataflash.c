/* LowLevel function for DataFlash environment support
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
 *
 */
#include <common.h>

#if defined(CFG_ENV_IS_IN_DATAFLASH) /* Environment is in DataFlash */

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <dataflash.h>

DECLARE_GLOBAL_DATA_PTR;

env_t *env_ptr = NULL;

char * env_name_spec = "dataflash";

extern int read_dataflash (unsigned long addr, unsigned long size, char
*result);
extern int write_dataflash (unsigned long addr_dest, unsigned long addr_src,
		     unsigned long size);
extern int AT91F_DataflashInit (void);
extern uchar default_environment[];
/* extern int default_environment_size; */


uchar env_get_char_spec (int index)
{
	uchar c;
	read_dataflash(CFG_ENV_ADDR + index + offsetof(env_t,data),
	1, (char *)&c);
	return (c);
}

void env_relocate_spec (void)
{
	read_dataflash(CFG_ENV_ADDR, CFG_ENV_SIZE, (char *)env_ptr);
}

int saveenv(void)
{
	/* env must be copied to do not alter env structure in memory*/
	unsigned char temp[CFG_ENV_SIZE];
	memcpy(temp, env_ptr, CFG_ENV_SIZE);
	return write_dataflash(CFG_ENV_ADDR, (unsigned long)temp, CFG_ENV_SIZE);
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
	if (gd->env_valid == 0){
		AT91F_DataflashInit();	/* prepare for DATAFLASH read/write */

		/* read old CRC */
		read_dataflash(CFG_ENV_ADDR + offsetof(env_t, crc),
			sizeof(ulong), (char *)&crc);
		new = 0;
		len = ENV_SIZE;
		off = offsetof(env_t,data);
		while (len > 0) {
			int n = (len > sizeof(buf)) ? sizeof(buf) : len;
			read_dataflash(CFG_ENV_ADDR + off, n, (char *)buf);
			new = crc32 (new, buf, n);
			len -= n;
			off += n;
		}
		if (crc == new) {
			gd->env_addr  = offsetof(env_t,data);
			gd->env_valid = 1;
		} else {
			gd->env_addr  = (ulong)&default_environment[0];
			gd->env_valid = 0;
		}
	}

 	return (0);
}

#endif /* CFG_ENV_IS_IN_DATAFLASH */

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

/*
 * 09-18-2001 Andreas Heppel, Sysgo RTS GmbH <aheppel@sysgo.de>
 *
 * It might not be possible in all cases to use 'memcpy()' to copy
 * the environment to NVRAM, as the NVRAM might not be mapped into
 * the memory space. (I.e. this is the case for the BAB750). In those
 * cases it might be possible to access the NVRAM using a different
 * method. For example, the RTC on the BAB750 is accessible in IO
 * space using its address and data registers. To enable usage of
 * NVRAM in those cases I invented the functions 'nvram_read()' and
 * 'nvram_write()', which will be activated upon the configuration
 * #define CFG_NVRAM_ACCESS_ROUTINE. Note, that those functions are
 * strongly dependent on the used HW, and must be redefined for each
 * board that wants to use them.
 */

#include <common.h>

#ifdef CFG_ENV_IS_IN_NVRAM /* Environment is in NVRAM */

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>

#ifdef CFG_NVRAM_ACCESS_ROUTINE
extern void *nvram_read(void *dest, const long src, size_t count);
extern void nvram_write(long dest, const void *src, size_t count);
env_t *env_ptr = NULL;
#else
env_t *env_ptr = (env_t *)CFG_ENV_ADDR;
#endif

char * env_name_spec = "NVRAM";

extern uchar default_environment[];
extern int default_environment_size;

extern uchar (*env_get_char)(int);
extern uchar env_get_char_memory (int index);

#ifdef CONFIG_AMIGAONEG3SE
uchar env_get_char_spec (int index)
{
#ifdef CFG_NVRAM_ACCESS_ROUTINE
	uchar c;

	nvram_read(&c, CFG_ENV_ADDR+index, 1);

	return c;
#else
	DECLARE_GLOBAL_DATA_PTR;
	uchar retval;
	enable_nvram();
	retval = *((uchar *)(gd->env_addr + index));
	disable_nvram();
	return retval;
#endif
}
#else
uchar env_get_char_spec (int index)
{
#ifdef CFG_NVRAM_ACCESS_ROUTINE
	uchar c;

	nvram_read(&c, CFG_ENV_ADDR+index, 1);

	return c;
#else
	DECLARE_GLOBAL_DATA_PTR;

	return *((uchar *)(gd->env_addr + index));
#endif
}
#endif

void env_relocate_spec (void)
{
#if defined(CFG_NVRAM_ACCESS_ROUTINE)
	nvram_read(env_ptr, CFG_ENV_ADDR, CFG_ENV_SIZE);
#else
	memcpy (env_ptr, (void*)CFG_ENV_ADDR, CFG_ENV_SIZE);
#endif
}

int saveenv (void)
{
	int rcode = 0;
#ifdef CONFIG_AMIGAONEG3SE
	enable_nvram();
#endif
#ifdef CFG_NVRAM_ACCESS_ROUTINE
	nvram_write(CFG_ENV_ADDR, env_ptr, CFG_ENV_SIZE);
#else
	if (memcpy ((char *)CFG_ENV_ADDR, env_ptr, CFG_ENV_SIZE) == NULL)
		    rcode = 1 ;
#endif
#ifdef CONFIG_AMIGAONEG3SE
	udelay(10000);
	disable_nvram();
#endif
	return rcode;
}


/************************************************************************
 * Initialize Environment use
 *
 * We are still running from ROM, so data use is limited
 */
int env_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
#ifdef CONFIG_AMIGAONEG3SE
	enable_nvram();
#endif
#if defined(CFG_NVRAM_ACCESS_ROUTINE)
	ulong crc;
	uchar data[ENV_SIZE];
	nvram_read (&crc, CFG_ENV_ADDR, sizeof(ulong));
	nvram_read (data, CFG_ENV_ADDR+sizeof(ulong), ENV_SIZE);

	if (crc32(0, data, ENV_SIZE) == crc) {
		gd->env_addr  = (ulong)CFG_ENV_ADDR + sizeof(long);
#else
	if (crc32(0, env_ptr->data, ENV_SIZE) == env_ptr->crc) {
		gd->env_addr  = (ulong)&(env_ptr->data);
#endif
		gd->env_valid = 1;
	} else {
		gd->env_addr  = (ulong)&default_environment[0];
		gd->env_valid = 0;
	}
#ifdef CONFIG_AMIGAONEG3SE
	disable_nvram();
#endif
	return (0);
}

#endif /* CFG_ENV_IS_IN_NVRAM */

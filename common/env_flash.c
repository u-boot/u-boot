/*
 * (C) Copyright 2000-2010
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

/* #define DEBUG */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <search.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CMD_SAVEENV) && defined(CONFIG_CMD_FLASH)
#define CMD_SAVEENV
#elif defined(CONFIG_ENV_ADDR_REDUND)
#error Cannot use CONFIG_ENV_ADDR_REDUND without CONFIG_CMD_SAVEENV & CONFIG_CMD_FLASH
#endif

#if defined(CONFIG_ENV_SIZE_REDUND) && (CONFIG_ENV_SIZE_REDUND < CONFIG_ENV_SIZE)
#error CONFIG_ENV_SIZE_REDUND should not be less then CONFIG_ENV_SIZE
#endif

char * env_name_spec = "Flash";

#ifdef ENV_IS_EMBEDDED

extern uchar environment[];
env_t *env_ptr = (env_t *)(&environment[0]);

static env_t *flash_addr = (env_t *)CONFIG_ENV_ADDR;

#else /* ! ENV_IS_EMBEDDED */

env_t *env_ptr = (env_t *)CONFIG_ENV_ADDR;
static env_t *flash_addr = (env_t *)CONFIG_ENV_ADDR;

#endif /* ENV_IS_EMBEDDED */

#if defined(CMD_SAVEENV) || defined(CONFIG_ENV_ADDR_REDUND)
/* CONFIG_ENV_ADDR is supposed to be on sector boundary */
static ulong end_addr = CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1;
#endif

#ifdef CONFIG_ENV_ADDR_REDUND
static env_t *flash_addr_new = (env_t *)CONFIG_ENV_ADDR_REDUND;

/* CONFIG_ENV_ADDR_REDUND is supposed to be on sector boundary */
static ulong end_addr_new = CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - 1;
#endif /* CONFIG_ENV_ADDR_REDUND */

extern const uchar default_environment[];


uchar env_get_char_spec(int index)
{
	return (*((uchar *)(gd->env_addr + index)));
}

#ifdef CONFIG_ENV_ADDR_REDUND

int  env_init(void)
{
	int crc1_ok = 0, crc2_ok = 0;

	uchar flag1 = flash_addr->flags;
	uchar flag2 = flash_addr_new->flags;

	ulong addr_default = (ulong)&default_environment[0];
	ulong addr1 = (ulong)&(flash_addr->data);
	ulong addr2 = (ulong)&(flash_addr_new->data);

	crc1_ok = (crc32(0, flash_addr->data, ENV_SIZE) == flash_addr->crc);
	crc2_ok = (crc32(0, flash_addr_new->data, ENV_SIZE) == flash_addr_new->crc);

	if (crc1_ok && ! crc2_ok) {
		gd->env_addr  = addr1;
		gd->env_valid = 1;
	} else if (! crc1_ok && crc2_ok) {
		gd->env_addr  = addr2;
		gd->env_valid = 1;
	} else if (! crc1_ok && ! crc2_ok) {
		gd->env_addr  = addr_default;
		gd->env_valid = 0;
	} else if (flag1 == ACTIVE_FLAG && flag2 == OBSOLETE_FLAG) {
		gd->env_addr  = addr1;
		gd->env_valid = 1;
	} else if (flag1 == OBSOLETE_FLAG && flag2 == ACTIVE_FLAG) {
		gd->env_addr  = addr2;
		gd->env_valid = 1;
	} else if (flag1 == flag2) {
		gd->env_addr  = addr1;
		gd->env_valid = 2;
	} else if (flag1 == 0xFF) {
		gd->env_addr  = addr1;
		gd->env_valid = 2;
	} else if (flag2 == 0xFF) {
		gd->env_addr  = addr2;
		gd->env_valid = 2;
	}

	return 0;
}

#ifdef CMD_SAVEENV
int saveenv(void)
{
	env_t	env_new;
	ssize_t	len;
	char	*saved_data = NULL;
	char	*res;
	int	rc = 1;
	char	flag = OBSOLETE_FLAG, new_flag = ACTIVE_FLAG;
#if CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE
	ulong	up_data = 0;
#endif

	debug("Protect off %08lX ... %08lX\n",
		(ulong)flash_addr, end_addr);

	if (flash_sect_protect(0, (ulong)flash_addr, end_addr)) {
		goto done;
	}

	debug("Protect off %08lX ... %08lX\n",
		(ulong)flash_addr_new, end_addr_new);

	if (flash_sect_protect(0, (ulong)flash_addr_new, end_addr_new)) {
		goto done;
	}

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', &res, ENV_SIZE);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		goto done;
	}
	env_new.crc   = crc32(0, env_new.data, ENV_SIZE);
	env_new.flags = new_flag;

#if CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE
	up_data = (end_addr_new + 1 - ((long)flash_addr_new + CONFIG_ENV_SIZE));
	debug("Data to save 0x%lX\n", up_data);
	if (up_data) {
		if ((saved_data = malloc(up_data)) == NULL) {
			printf("Unable to save the rest of sector (%ld)\n",
				up_data);
			goto done;
		}
		memcpy(saved_data,
			(void *)((long)flash_addr_new + CONFIG_ENV_SIZE), up_data);
		debug("Data (start 0x%lX, len 0x%lX) saved at 0x%p\n",
			(long)flash_addr_new + CONFIG_ENV_SIZE,
			up_data, saved_data);
	}
#endif
	puts("Erasing Flash...");
	debug(" %08lX ... %08lX ...",
		(ulong)flash_addr_new, end_addr_new);

	if (flash_sect_erase((ulong)flash_addr_new, end_addr_new)) {
		goto done;
	}

	puts("Writing to Flash... ");
	debug(" %08lX ... %08lX ...",
		(ulong)&(flash_addr_new->data),
		sizeof(env_ptr->data)+(ulong)&(flash_addr_new->data));
	if ((rc = flash_write((char *)&env_new,
			(ulong)flash_addr_new,
			sizeof(env_new))) ||
	    (rc = flash_write(&flag,
			(ulong)&(flash_addr->flags),
			sizeof(flash_addr->flags))) ) {
		flash_perror(rc);
		goto done;
	}

#if CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE
	if (up_data) { /* restore the rest of sector */
		debug("Restoring the rest of data to 0x%lX len 0x%lX\n",
			(long)flash_addr_new + CONFIG_ENV_SIZE, up_data);
		if (flash_write(saved_data,
				(long)flash_addr_new + CONFIG_ENV_SIZE,
				up_data)) {
			flash_perror(rc);
			goto done;
		}
	}
#endif
	puts("done\n");

	{
		env_t * etmp = flash_addr;
		ulong ltmp = end_addr;

		flash_addr = flash_addr_new;
		flash_addr_new = etmp;

		end_addr = end_addr_new;
		end_addr_new = ltmp;
	}

	rc = 0;
done:
	if (saved_data)
		free(saved_data);
	/* try to re-protect */
	(void) flash_sect_protect(1, (ulong)flash_addr, end_addr);
	(void) flash_sect_protect(1, (ulong)flash_addr_new, end_addr_new);

	return rc;
}
#endif /* CMD_SAVEENV */

#else /* ! CONFIG_ENV_ADDR_REDUND */

int  env_init(void)
{
	if (crc32(0, env_ptr->data, ENV_SIZE) == env_ptr->crc) {
		gd->env_addr  = (ulong)&(env_ptr->data);
		gd->env_valid = 1;
		return(0);
	}

	gd->env_addr  = (ulong)&default_environment[0];
	gd->env_valid = 0;
	return 0;
}

#ifdef CMD_SAVEENV

int saveenv(void)
{
	env_t	env_new;
	ssize_t	len;
	int	rc = 1;
	char	*res;
	char	*saved_data = NULL;
#if CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE
	ulong	up_data = 0;

	up_data = (end_addr + 1 - ((long)flash_addr + CONFIG_ENV_SIZE));
	debug("Data to save 0x%lx\n", up_data);
	if (up_data) {
		if ((saved_data = malloc(up_data)) == NULL) {
			printf("Unable to save the rest of sector (%ld)\n",
				up_data);
			goto done;
		}
		memcpy(saved_data,
			(void *)((long)flash_addr + CONFIG_ENV_SIZE), up_data);
		debug("Data (start 0x%lx, len 0x%lx) saved at 0x%lx\n",
			(ulong)flash_addr + CONFIG_ENV_SIZE,
			up_data,
			(ulong)saved_data);
	}
#endif	/* CONFIG_ENV_SECT_SIZE */

	debug("Protect off %08lX ... %08lX\n",
		(ulong)flash_addr, end_addr);

	if (flash_sect_protect(0, (long)flash_addr, end_addr))
		goto done;

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', &res, ENV_SIZE);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		goto done;
	}
	env_new.crc = crc32(0, env_new.data, ENV_SIZE);

	puts("Erasing Flash...");
	if (flash_sect_erase((long)flash_addr, end_addr))
		goto done;

	puts("Writing to Flash... ");
	rc = flash_write((char *)&env_new, (long)flash_addr, CONFIG_ENV_SIZE);
	if (rc != 0) {
		flash_perror(rc);
		goto done;
	}
#if CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE
	if (up_data) {	/* restore the rest of sector */
		debug("Restoring the rest of data to 0x%lx len 0x%lx\n",
			(ulong)flash_addr + CONFIG_ENV_SIZE, up_data);
		if (flash_write(saved_data,
				(long)flash_addr + CONFIG_ENV_SIZE,
				up_data)) {
			flash_perror(rc);
			goto done;
		}
	}
#endif
	puts("done\n");
	rc = 0;
done:
	if (saved_data)
		free(saved_data);
	/* try to re-protect */
	(void) flash_sect_protect(1, (long)flash_addr, end_addr);
	return rc;
}

#endif /* CMD_SAVEENV */

#endif /* CONFIG_ENV_ADDR_REDUND */

void env_relocate_spec(void)
{
#ifdef CONFIG_ENV_ADDR_REDUND
	if (gd->env_addr != (ulong)&(flash_addr->data)) {
		env_t *etmp = flash_addr;
		ulong ltmp = end_addr;

		flash_addr = flash_addr_new;
		flash_addr_new = etmp;

		end_addr = end_addr_new;
		end_addr_new = ltmp;
	}

	if (flash_addr_new->flags != OBSOLETE_FLAG &&
	    crc32(0, flash_addr_new->data, ENV_SIZE) ==
	    flash_addr_new->crc) {
		char flag = OBSOLETE_FLAG;

		gd->env_valid = 2;
		flash_sect_protect(0, (ulong)flash_addr_new, end_addr_new);
		flash_write(&flag,
			    (ulong)&(flash_addr_new->flags),
			    sizeof(flash_addr_new->flags));
		flash_sect_protect(1, (ulong)flash_addr_new, end_addr_new);
	}

	if (flash_addr->flags != ACTIVE_FLAG &&
	    (flash_addr->flags & ACTIVE_FLAG) == ACTIVE_FLAG) {
		char flag = ACTIVE_FLAG;

		gd->env_valid = 2;
		flash_sect_protect(0, (ulong)flash_addr, end_addr);
		flash_write(&flag,
			    (ulong)&(flash_addr->flags),
			    sizeof(flash_addr->flags));
		flash_sect_protect(1, (ulong)flash_addr, end_addr);
	}

	if (gd->env_valid == 2)
		puts ("*** Warning - some problems detected "
		      "reading environment; recovered successfully\n\n");
#endif /* CONFIG_ENV_ADDR_REDUND */

	env_import((char *)flash_addr, 1);
}

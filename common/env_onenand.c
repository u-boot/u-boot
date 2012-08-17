/*
 * (C) Copyright 2010 DENX Software Engineering
 * Wolfgang Denk <wd@denx.de>
 *
 * (C) Copyright 2005-2009 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
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
#include <malloc.h>
#include <search.h>
#include <errno.h>
#include <onenand_uboot.h>

#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

char *env_name_spec = "OneNAND";

#define ONENAND_MAX_ENV_SIZE	4096
#define ONENAND_ENV_SIZE(mtd)	(ONENAND_MAX_ENV_SIZE - ENV_HEADER_SIZE)

DECLARE_GLOBAL_DATA_PTR;

void env_relocate_spec(void)
{
	struct mtd_info *mtd = &onenand_mtd;
#ifdef CONFIG_ENV_ADDR_FLEX
	struct onenand_chip *this = &onenand_chip;
#endif
	int rc;
	size_t retlen;
#ifdef ENV_IS_EMBEDDED
	char *buf = (char *)&environment;
#else
	loff_t env_addr = CONFIG_ENV_ADDR;
	char onenand_env[ONENAND_MAX_ENV_SIZE];
	char *buf = (char *)&onenand_env[0];
#endif /* ENV_IS_EMBEDDED */

#ifndef ENV_IS_EMBEDDED
# ifdef CONFIG_ENV_ADDR_FLEX
	if (FLEXONENAND(this))
		env_addr = CONFIG_ENV_ADDR_FLEX;
# endif
	/* Check OneNAND exist */
	if (mtd->writesize)
		/* Ignore read fail */
		mtd->read(mtd, env_addr, ONENAND_MAX_ENV_SIZE,
				&retlen, (u_char *)buf);
	else
		mtd->writesize = MAX_ONENAND_PAGESIZE;
#endif /* !ENV_IS_EMBEDDED */

	rc = env_import(buf, 1);
	if (rc)
		gd->env_valid = 1;
}

int saveenv(void)
{
	env_t	env_new;
	ssize_t	len;
	char	*res;
	struct mtd_info *mtd = &onenand_mtd;
#ifdef CONFIG_ENV_ADDR_FLEX
	struct onenand_chip *this = &onenand_chip;
#endif
	loff_t	env_addr = CONFIG_ENV_ADDR;
	size_t	retlen;
	struct erase_info instr = {
		.callback	= NULL,
	};

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}
	env_new.crc = crc32(0, env_new.data, ENV_SIZE);

	instr.len = CONFIG_ENV_SIZE;
#ifdef CONFIG_ENV_ADDR_FLEX
	if (FLEXONENAND(this)) {
		env_addr = CONFIG_ENV_ADDR_FLEX;
		instr.len = CONFIG_ENV_SIZE_FLEX;
		instr.len <<= onenand_mtd.eraseregions[0].numblocks == 1 ?
				1 : 0;
	}
#endif
	instr.addr = env_addr;
	instr.mtd = mtd;
	if (mtd->erase(mtd, &instr)) {
		printf("OneNAND: erase failed at 0x%08llx\n", env_addr);
		return 1;
	}

	if (mtd->write(mtd, env_addr, ONENAND_MAX_ENV_SIZE, &retlen,
			(u_char *)&env_new)) {
		printf("OneNAND: write failed at 0x%llx\n", instr.addr);
		return 2;
	}

	return 0;
}

int env_init(void)
{
	/* use default */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}

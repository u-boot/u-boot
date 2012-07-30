/*
 * (C) Copyright 2008-2011 Freescale Semiconductor, Inc.
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
#include <mmc.h>
#include <search.h>
#include <errno.h>

char *env_name_spec = "MMC";

#ifdef ENV_IS_EMBEDDED
env_t *env_ptr = &environment;
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr;
#endif /* ENV_IS_EMBEDDED */

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_OFFSET 0
#endif

static int __mmc_get_env_addr(struct mmc *mmc, u32 *env_addr)
{
	*env_addr = CONFIG_ENV_OFFSET;
	return 0;
}
int mmc_get_env_addr(struct mmc *mmc, u32 *env_addr)
	__attribute__((weak, alias("__mmc_get_env_addr")));

int env_init(void)
{
	/* use default */
	gd->env_addr	= (ulong)&default_environment[0];
	gd->env_valid	= 1;

	return 0;
}

static int init_mmc_for_env(struct mmc *mmc)
{
	if (!mmc) {
		puts("No MMC card found\n");
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -1;
	}

#ifdef CONFIG_SYS_MMC_ENV_PART
	if (CONFIG_SYS_MMC_ENV_PART != mmc->part_num) {
		if (mmc_switch_part(CONFIG_SYS_MMC_ENV_DEV,
				    CONFIG_SYS_MMC_ENV_PART)) {
			puts("MMC partition switch failed\n");
			return -1;
		}
	}
#endif

	return 0;
}

static void fini_mmc_for_env(struct mmc *mmc)
{
#ifdef CONFIG_SYS_MMC_ENV_PART
	if (CONFIG_SYS_MMC_ENV_PART != mmc->part_num)
		mmc_switch_part(CONFIG_SYS_MMC_ENV_DEV,
				mmc->part_num);
#endif
}

#ifdef CONFIG_CMD_SAVEENV
static inline int write_env(struct mmc *mmc, unsigned long size,
			    unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start	= ALIGN(offset, mmc->write_bl_len) / mmc->write_bl_len;
	blk_cnt		= ALIGN(size, mmc->write_bl_len) / mmc->write_bl_len;

	n = mmc->block_dev.block_write(CONFIG_SYS_MMC_ENV_DEV, blk_start,
					blk_cnt, (u_char *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

int saveenv(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	ssize_t	len;
	char	*res;
	struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);
	u32	offset;
	int	ret;

	if (init_mmc_for_env(mmc))
		return 1;

	if (mmc_get_env_addr(mmc, &offset)) {
		ret = 1;
		goto fini;
	}

	res = (char *)&env_new->data;
	len = hexport_r(&env_htab, '\0', &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		ret = 1;
		goto fini;
	}

	env_new->crc = crc32(0, &env_new->data[0], ENV_SIZE);
	printf("Writing to MMC(%d)... ", CONFIG_SYS_MMC_ENV_DEV);
	if (write_env(mmc, CONFIG_ENV_SIZE, offset, (u_char *)env_new)) {
		puts("failed\n");
		ret = 1;
		goto fini;
	}

	puts("done\n");
	ret = 0;

fini:
	fini_mmc_for_env(mmc);
	return ret;
}
#endif /* CONFIG_CMD_SAVEENV */

static inline int read_env(struct mmc *mmc, unsigned long size,
			   unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start	= ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt		= ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;

	n = mmc->block_dev.block_read(CONFIG_SYS_MMC_ENV_DEV, blk_start,
					blk_cnt, (uchar *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

void env_relocate_spec(void)
{
#if !defined(ENV_IS_EMBEDDED)
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);
	u32 offset;
	int ret;

	if (init_mmc_for_env(mmc)) {
		ret = 1;
		goto err;
	}

	if (mmc_get_env_addr(mmc, &offset)) {
		ret = 1;
		goto fini;
	}

	if (read_env(mmc, CONFIG_ENV_SIZE, offset, buf)) {
		ret = 1;
		goto fini;
	}

	env_import(buf, 1);
	ret = 0;

fini:
	fini_mmc_for_env(mmc);
err:
	if (ret)
		set_default_env(NULL);
#endif
}

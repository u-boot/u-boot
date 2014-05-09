/*
 * (C) Copyright 2008-2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

#if defined(CONFIG_ENV_SIZE_REDUND) &&  \
	(CONFIG_ENV_SIZE_REDUND != CONFIG_ENV_SIZE)
#error CONFIG_ENV_SIZE_REDUND should be the same as CONFIG_ENV_SIZE
#endif

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

__weak int mmc_get_env_addr(struct mmc *mmc, int copy, u32 *env_addr)
{
	s64 offset;

	offset = CONFIG_ENV_OFFSET;
#ifdef CONFIG_ENV_OFFSET_REDUND
	if (copy)
		offset = CONFIG_ENV_OFFSET_REDUND;
#endif

	if (offset < 0)
		offset += mmc->capacity;

	*env_addr = offset;

	return 0;
}

int env_init(void)
{
	/* use default */
	gd->env_addr	= (ulong)&default_environment[0];
	gd->env_valid	= 1;

	return 0;
}

static int init_mmc_for_env(struct mmc *mmc)
{
#ifdef CONFIG_SYS_MMC_ENV_PART
	int dev = CONFIG_SYS_MMC_ENV_DEV;

#ifdef CONFIG_SPL_BUILD
	dev = 0;
#endif
#endif

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
		if (mmc_switch_part(dev, CONFIG_SYS_MMC_ENV_PART)) {
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
	int dev = CONFIG_SYS_MMC_ENV_DEV;

#ifdef CONFIG_SPL_BUILD
	dev = 0;
#endif
	if (CONFIG_SYS_MMC_ENV_PART != mmc->part_num)
		mmc_switch_part(dev, mmc->part_num);
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

#ifdef CONFIG_ENV_OFFSET_REDUND
static unsigned char env_flags;
#endif

int saveenv(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);
	u32	offset;
	int	ret, copy = 0;

	if (init_mmc_for_env(mmc))
		return 1;

	ret = env_export(env_new);
	if (ret)
		goto fini;

#ifdef CONFIG_ENV_OFFSET_REDUND
	env_new->flags	= ++env_flags; /* increase the serial */

	if (gd->env_valid == 1)
		copy = 1;
#endif

	if (mmc_get_env_addr(mmc, copy, &offset)) {
		ret = 1;
		goto fini;
	}

	printf("Writing to %sMMC(%d)... ", copy ? "redundant " : "",
	       CONFIG_SYS_MMC_ENV_DEV);
	if (write_env(mmc, CONFIG_ENV_SIZE, offset, (u_char *)env_new)) {
		puts("failed\n");
		ret = 1;
		goto fini;
	}

	puts("done\n");
	ret = 0;

#ifdef CONFIG_ENV_OFFSET_REDUND
	gd->env_valid = gd->env_valid == 2 ? 1 : 2;
#endif

fini:
	fini_mmc_for_env(mmc);
	return ret;
}
#endif /* CONFIG_CMD_SAVEENV */

static inline int read_env(struct mmc *mmc, unsigned long size,
			   unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;
	int dev = CONFIG_SYS_MMC_ENV_DEV;

#ifdef CONFIG_SPL_BUILD
	dev = 0;
#endif

	blk_start	= ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt		= ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;

	n = mmc->block_dev.block_read(dev, blk_start, blk_cnt, (uchar *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

#ifdef CONFIG_ENV_OFFSET_REDUND
void env_relocate_spec(void)
{
#if !defined(ENV_IS_EMBEDDED)
	struct mmc *mmc;
	u32 offset1, offset2;
	int read1_fail = 0, read2_fail = 0;
	int crc1_ok = 0, crc2_ok = 0;
	env_t *ep;
	int ret;
	int dev = CONFIG_SYS_MMC_ENV_DEV;

	ALLOC_CACHE_ALIGN_BUFFER(env_t, tmp_env1, 1);
	ALLOC_CACHE_ALIGN_BUFFER(env_t, tmp_env2, 1);

#ifdef CONFIG_SPL_BUILD
	dev = 0;
#endif

	mmc = find_mmc_device(dev);

	if (init_mmc_for_env(mmc)) {
		ret = 1;
		goto err;
	}

	if (mmc_get_env_addr(mmc, 0, &offset1) ||
	    mmc_get_env_addr(mmc, 1, &offset2)) {
		ret = 1;
		goto fini;
	}

	read1_fail = read_env(mmc, CONFIG_ENV_SIZE, offset1, tmp_env1);
	read2_fail = read_env(mmc, CONFIG_ENV_SIZE, offset2, tmp_env2);

	if (read1_fail && read2_fail)
		puts("*** Error - No Valid Environment Area found\n");
	else if (read1_fail || read2_fail)
		puts("*** Warning - some problems detected "
		     "reading environment; recovered successfully\n");

	crc1_ok = !read1_fail &&
		(crc32(0, tmp_env1->data, ENV_SIZE) == tmp_env1->crc);
	crc2_ok = !read2_fail &&
		(crc32(0, tmp_env2->data, ENV_SIZE) == tmp_env2->crc);

	if (!crc1_ok && !crc2_ok) {
		ret = 1;
		goto fini;
	} else if (crc1_ok && !crc2_ok) {
		gd->env_valid = 1;
	} else if (!crc1_ok && crc2_ok) {
		gd->env_valid = 2;
	} else {
		/* both ok - check serial */
		if (tmp_env1->flags == 255 && tmp_env2->flags == 0)
			gd->env_valid = 2;
		else if (tmp_env2->flags == 255 && tmp_env1->flags == 0)
			gd->env_valid = 1;
		else if (tmp_env1->flags > tmp_env2->flags)
			gd->env_valid = 1;
		else if (tmp_env2->flags > tmp_env1->flags)
			gd->env_valid = 2;
		else /* flags are equal - almost impossible */
			gd->env_valid = 1;
	}

	free(env_ptr);

	if (gd->env_valid == 1)
		ep = tmp_env1;
	else
		ep = tmp_env2;

	env_flags = ep->flags;
	env_import((char *)ep, 0);
	ret = 0;

fini:
	fini_mmc_for_env(mmc);
err:
	if (ret)
		set_default_env(NULL);

#endif
}
#else /* ! CONFIG_ENV_OFFSET_REDUND */
void env_relocate_spec(void)
{
#if !defined(ENV_IS_EMBEDDED)
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	struct mmc *mmc;
	u32 offset;
	int ret;
	int dev = CONFIG_SYS_MMC_ENV_DEV;

#ifdef CONFIG_SPL_BUILD
	dev = 0;
#endif

	mmc = find_mmc_device(dev);

	if (init_mmc_for_env(mmc)) {
		ret = 1;
		goto err;
	}

	if (mmc_get_env_addr(mmc, 0, &offset)) {
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
#endif /* CONFIG_ENV_OFFSET_REDUND */

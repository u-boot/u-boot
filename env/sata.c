/*
 * (C) Copyright 2010-2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* #define DEBUG */

#include <common.h>

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <errno.h>
#include <memalign.h>
#include <sata.h>
#include <search.h>

#if defined(CONFIG_ENV_SIZE_REDUND) || defined(CONFIG_ENV_OFFSET_REDUND)
#error ENV REDUND not supported
#endif

#if !defined(CONFIG_ENV_OFFSET) || !defined(CONFIG_ENV_SIZE)
#error CONFIG_ENV_OFFSET or CONFIG_ENV_SIZE not defined
#endif

char *env_name_spec = "SATA";

DECLARE_GLOBAL_DATA_PTR;

__weak int sata_get_env_dev(void)
{
	return CONFIG_SYS_SATA_ENV_DEV;
}

int env_init(void)
{
	/* use default */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}

#ifdef CONFIG_CMD_SAVEENV
static inline int write_env(struct blk_desc *sata, unsigned long size,
			    unsigned long offset, void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start = ALIGN(offset, sata->blksz) / sata->blksz;
	blk_cnt   = ALIGN(size, sata->blksz) / sata->blksz;

	n = blk_dwrite(sata, blk_start, blk_cnt, buffer);

	return (n == blk_cnt) ? 0 : -1;
}

int saveenv(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	struct blk_desc *sata = NULL;
	int env_sata, ret;

	if (sata_initialize())
		return 1;

	env_sata = sata_get_env_dev();

	sata = sata_get_dev(env_sata);
	if (sata == NULL) {
		printf("Unknown SATA(%d) device for environment!\n",
		       env_sata);
		return 1;
	}

	ret = env_export(env_new);
	if (ret)
		return 1;

	printf("Writing to SATA(%d)...", env_sata);
	if (write_env(sata, CONFIG_ENV_SIZE, CONFIG_ENV_OFFSET, &env_new)) {
		puts("failed\n");
		return 1;
	}

	puts("done\n");
	return 0;
}
#endif /* CONFIG_CMD_SAVEENV */

static inline int read_env(struct blk_desc *sata, unsigned long size,
			   unsigned long offset, void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start = ALIGN(offset, sata->blksz) / sata->blksz;
	blk_cnt   = ALIGN(size, sata->blksz) / sata->blksz;

	n = blk_dread(sata, blk_start, blk_cnt, buffer);

	return (n == blk_cnt) ? 0 : -1;
}

void env_relocate_spec(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	struct blk_desc *sata = NULL;
	int env_sata;

	if (sata_initialize())
		return;

	env_sata = sata_get_env_dev();

	sata = sata_get_dev(env_sata);
	if (sata == NULL) {
		printf("Unknown SATA(%d) device for environment!\n",
		       env_sata);
		return;
	}

	if (read_env(sata, CONFIG_ENV_SIZE, CONFIG_ENV_OFFSET, buf))
		return set_default_env(NULL);

	env_import(buf, 1);
}

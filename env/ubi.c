/*
 * (c) Copyright 2012 by National Instruments,
 *        Joe Hershberger <joe.hershberger@ni.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <command.h>
#include <environment.h>
#include <errno.h>
#include <malloc.h>
#include <memalign.h>
#include <search.h>
#include <ubi_uboot.h>
#undef crc32

char *env_name_spec = "UBI";

env_t *env_ptr;

DECLARE_GLOBAL_DATA_PTR;

int env_init(void)
{
	/* use default */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}

#ifdef CONFIG_CMD_SAVEENV
#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
int saveenv(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	int ret;

	ret = env_export(env_new);
	if (ret)
		return ret;

	if (ubi_part(CONFIG_ENV_UBI_PART, NULL)) {
		printf("\n** Cannot find mtd partition \"%s\"\n",
		       CONFIG_ENV_UBI_PART);
		return 1;
	}

	if (gd->env_valid == 1) {
		puts("Writing to redundant UBI... ");
		if (ubi_volume_write(CONFIG_ENV_UBI_VOLUME_REDUND,
				     (void *)env_new, CONFIG_ENV_SIZE)) {
			printf("\n** Unable to write env to %s:%s **\n",
			       CONFIG_ENV_UBI_PART,
			       CONFIG_ENV_UBI_VOLUME_REDUND);
			return 1;
		}
	} else {
		puts("Writing to UBI... ");
		if (ubi_volume_write(CONFIG_ENV_UBI_VOLUME,
				     (void *)env_new, CONFIG_ENV_SIZE)) {
			printf("\n** Unable to write env to %s:%s **\n",
			       CONFIG_ENV_UBI_PART,
			       CONFIG_ENV_UBI_VOLUME);
			return 1;
		}
	}

	puts("done\n");

	gd->env_valid = gd->env_valid == 2 ? 1 : 2;

	return 0;
}
#else /* ! CONFIG_SYS_REDUNDAND_ENVIRONMENT */
int saveenv(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	int ret;

	ret = env_export(env_new);
	if (ret)
		return ret;

	if (ubi_part(CONFIG_ENV_UBI_PART, NULL)) {
		printf("\n** Cannot find mtd partition \"%s\"\n",
		       CONFIG_ENV_UBI_PART);
		return 1;
	}

	if (ubi_volume_write(CONFIG_ENV_UBI_VOLUME, (void *)env_new,
			     CONFIG_ENV_SIZE)) {
		printf("\n** Unable to write env to %s:%s **\n",
		       CONFIG_ENV_UBI_PART, CONFIG_ENV_UBI_VOLUME);
		return 1;
	}

	puts("done\n");
	return 0;
}
#endif /* CONFIG_SYS_REDUNDAND_ENVIRONMENT */
#endif /* CONFIG_CMD_SAVEENV */

#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
void env_relocate_spec(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, env1_buf, CONFIG_ENV_SIZE);
	ALLOC_CACHE_ALIGN_BUFFER(char, env2_buf, CONFIG_ENV_SIZE);
	env_t *tmp_env1, *tmp_env2;

	/*
	 * In case we have restarted u-boot there is a chance that buffer
	 * contains old environment (from the previous boot).
	 * If UBI volume is zero size, ubi_volume_read() doesn't modify the
	 * buffer.
	 * We need to clear buffer manually here, so the invalid CRC will
	 * cause setting default environment as expected.
	 */
	memset(env1_buf, 0x0, CONFIG_ENV_SIZE);
	memset(env2_buf, 0x0, CONFIG_ENV_SIZE);

	tmp_env1 = (env_t *)env1_buf;
	tmp_env2 = (env_t *)env2_buf;

	if (ubi_part(CONFIG_ENV_UBI_PART, NULL)) {
		printf("\n** Cannot find mtd partition \"%s\"\n",
		       CONFIG_ENV_UBI_PART);
		set_default_env(NULL);
		return;
	}

	if (ubi_volume_read(CONFIG_ENV_UBI_VOLUME, (void *)tmp_env1,
			    CONFIG_ENV_SIZE)) {
		printf("\n** Unable to read env from %s:%s **\n",
		       CONFIG_ENV_UBI_PART, CONFIG_ENV_UBI_VOLUME);
	}

	if (ubi_volume_read(CONFIG_ENV_UBI_VOLUME_REDUND, (void *)tmp_env2,
			    CONFIG_ENV_SIZE)) {
		printf("\n** Unable to read redundant env from %s:%s **\n",
		       CONFIG_ENV_UBI_PART, CONFIG_ENV_UBI_VOLUME_REDUND);
	}

	env_import_redund((char *)tmp_env1, (char *)tmp_env2);
}
#else /* ! CONFIG_SYS_REDUNDAND_ENVIRONMENT */
void env_relocate_spec(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);

	/*
	 * In case we have restarted u-boot there is a chance that buffer
	 * contains old environment (from the previous boot).
	 * If UBI volume is zero size, ubi_volume_read() doesn't modify the
	 * buffer.
	 * We need to clear buffer manually here, so the invalid CRC will
	 * cause setting default environment as expected.
	 */
	memset(buf, 0x0, CONFIG_ENV_SIZE);

	if (ubi_part(CONFIG_ENV_UBI_PART, NULL)) {
		printf("\n** Cannot find mtd partition \"%s\"\n",
		       CONFIG_ENV_UBI_PART);
		set_default_env(NULL);
		return;
	}

	if (ubi_volume_read(CONFIG_ENV_UBI_VOLUME, buf, CONFIG_ENV_SIZE)) {
		printf("\n** Unable to read env from %s:%s **\n",
		       CONFIG_ENV_UBI_PART, CONFIG_ENV_UBI_VOLUME);
		set_default_env(NULL);
		return;
	}

	env_import(buf, 1);
}
#endif /* CONFIG_SYS_REDUNDAND_ENVIRONMENT */

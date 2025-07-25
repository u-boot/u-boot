// SPDX-License-Identifier: GPL-2.0+
/*
 *  Author: Christian Marangi <ansuelsmth@gmail.com>
 */
#include <env_internal.h>
#include <errno.h>
#include <malloc.h>
#include <mtd.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <linux/mtd/mtd.h>
#include <u-boot/crc.h>

DECLARE_GLOBAL_DATA_PTR;

static int setup_mtd_device(struct mtd_info **mtd_env)
{
	struct mtd_info *mtd;

	mtd_probe_devices();

	mtd = get_mtd_device_nm(CONFIG_ENV_MTD_DEV);
	if (IS_ERR_OR_NULL(mtd)) {
		env_set_default("get_mtd_device_nm() failed", 0);
		return mtd ? PTR_ERR(mtd) : -EINVAL;
	}

	*mtd_env = mtd;

	return 0;
}

static int env_mtd_save(void)
{
	char *saved_buf = NULL, *write_buf, *tmp;
	struct erase_info ei = { };
	struct mtd_info *mtd_env;
	u32 sect_size, sect_num;
	size_t ret_len = 0;
	u32 write_size;
	env_t env_new;
	int remaining;
	u32 offset;
	int ret;

	ret = setup_mtd_device(&mtd_env);
	if (ret)
		return ret;

	sect_size = mtd_env->erasesize;

	/* Is the sector larger than the env (i.e. embedded) */
	if (sect_size > CONFIG_ENV_SIZE) {
		saved_buf = malloc(sect_size);
		if (!saved_buf) {
			ret = -ENOMEM;
			goto done;
		}

		offset = CONFIG_ENV_OFFSET;
		remaining = sect_size;
		tmp = saved_buf;

		while (remaining) {
			/* Skip the block if it is bad */
			if (!(offset % sect_size) &&
			    mtd_block_isbad(mtd_env, offset)) {
				offset += sect_size;
				continue;
			}

			ret = mtd_read(mtd_env, offset, mtd_env->writesize,
				       &ret_len, tmp);
			if (ret)
				goto done;

			tmp += ret_len;
			offset += ret_len;
			remaining -= ret_len;
		}
	}

	ret = env_export(&env_new);
	if (ret)
		goto done;

	sect_num = DIV_ROUND_UP(CONFIG_ENV_SIZE, sect_size);

	ei.mtd = mtd_env;
	ei.addr = CONFIG_ENV_OFFSET;
	ei.len = sect_num * sect_size;

	puts("Erasing MTD...");
	ret = mtd_erase(mtd_env, &ei);
	if (ret)
		goto done;

	if (sect_size > CONFIG_ENV_SIZE) {
		memcpy(saved_buf, &env_new, CONFIG_ENV_SIZE);
		write_size = sect_size;
		write_buf = saved_buf;
	} else {
		write_size = sect_num * sect_size;
		write_buf = (char *)&env_new;
	}

	offset = CONFIG_ENV_OFFSET;
	remaining = write_size;
	tmp = write_buf;

	puts("Writing to MTD...");
	while (remaining) {
		/* Skip the block if it is bad */
		if (!(offset % sect_size) &&
		    mtd_block_isbad(mtd_env, offset)) {
			offset += sect_size;
			continue;
		}

		ret = mtd_write(mtd_env, offset, mtd_env->writesize,
				&ret_len, tmp);
		if (ret)
			goto done;

		offset += mtd_env->writesize;
		remaining -= ret_len;
		tmp += ret_len;
	}

	ret = 0;
	puts("done\n");

done:
	put_mtd_device(mtd_env);

	if (saved_buf)
		free(saved_buf);

	return ret;
}

static int env_mtd_load(void)
{
	struct mtd_info *mtd_env;
	char *buf, *tmp;
	size_t ret_len;
	int remaining;
	u32 sect_size;
	u32 offset;
	int ret;

	buf = (char *)memalign(ARCH_DMA_MINALIGN, CONFIG_ENV_SIZE);
	if (!buf) {
		env_set_default("memalign() failed", 0);
		return -EIO;
	}

	ret = setup_mtd_device(&mtd_env);
	if (ret)
		goto out;

	sect_size = mtd_env->erasesize;

	offset = CONFIG_ENV_OFFSET;
	remaining = CONFIG_ENV_SIZE;
	tmp = buf;

	while (remaining) {
		/* Skip the block if it is bad */
		if (!(offset % sect_size) &&
		    mtd_block_isbad(mtd_env, offset)) {
			offset += sect_size;
			continue;
		}

		ret = mtd_read(mtd_env, offset, mtd_env->writesize,
			       &ret_len, tmp);
		if (ret) {
			env_set_default("mtd_read() failed", 1);
			goto out;
		}

		tmp += ret_len;
		offset += ret_len;
		remaining -= ret_len;
	}

	ret = env_import(buf, 1, H_EXTERNAL);
	if (!ret)
		gd->env_valid = ENV_VALID;

out:
	put_mtd_device(mtd_env);

	free(buf);

	return ret;
}

static int env_mtd_erase(void)
{
	struct mtd_info *mtd_env;
	u32 sect_size, sect_num;
	char *saved_buf = NULL, *tmp;
	struct erase_info ei;
	size_t ret_len;
	int remaining;
	u32 offset;
	int ret;

	ret = setup_mtd_device(&mtd_env);
	if (ret)
		return ret;

	sect_size = mtd_env->erasesize;

	/* Is the sector larger than the env (i.e. embedded) */
	if (sect_size > CONFIG_ENV_SIZE) {
		saved_buf = malloc(sect_size);
		if (!saved_buf) {
			ret = -ENOMEM;
			goto done;
		}

		offset = CONFIG_ENV_OFFSET;
		remaining = sect_size;
		tmp = saved_buf;

		while (remaining) {
			/* Skip the block if it is bad */
			if (!(offset % sect_size) &&
			    mtd_block_isbad(mtd_env, offset)) {
				offset += sect_size;
				continue;
			}

			ret = mtd_read(mtd_env, offset, mtd_env->writesize,
				       &ret_len, tmp);
			if (ret)
				goto done;

			tmp += ret_len;
			offset += ret_len;
			remaining -= ret_len;
		}
	}

	sect_num = DIV_ROUND_UP(CONFIG_ENV_SIZE, sect_size);

	ei.mtd = mtd_env;
	ei.addr = CONFIG_ENV_OFFSET;
	ei.len = sect_num * sect_size;

	ret = mtd_erase(mtd_env, &ei);
	if (ret)
		goto done;

	if (sect_size > CONFIG_ENV_SIZE) {
		memset(saved_buf, 0, CONFIG_ENV_SIZE);

		offset = CONFIG_ENV_OFFSET;
		remaining = sect_size;
		tmp = saved_buf;

		while (remaining) {
			/* Skip the block if it is bad */
			if (!(offset % sect_size) &&
			    mtd_block_isbad(mtd_env, offset)) {
				offset += sect_size;
				continue;
			}

			ret = mtd_write(mtd_env, offset, mtd_env->writesize,
					&ret_len, tmp);
			if (ret)
				goto done;

			offset += mtd_env->writesize;
			remaining -= ret_len;
			tmp += ret_len;
		}
	}

	ret = 0;

done:
	put_mtd_device(mtd_env);

	if (saved_buf)
		free(saved_buf);

	return ret;
}

__weak void *env_mtd_get_env_addr(void)
{
	return (void *)CONFIG_ENV_ADDR;
}

/*
 * Check if Environment on CONFIG_ENV_ADDR is valid.
 */
static int env_mtd_init_addr(void)
{
	env_t *env_ptr = (env_t *)env_mtd_get_env_addr();

	if (!env_ptr)
		return -ENOENT;

	if (crc32(0, env_ptr->data, ENV_SIZE) == env_ptr->crc) {
		gd->env_addr = (ulong)&env_ptr->data;
		gd->env_valid = ENV_VALID;
	} else {
		gd->env_valid = ENV_INVALID;
	}

	return 0;
}

static int env_mtd_init(void)
{
	int ret;

	ret = env_mtd_init_addr();
	if (ret != -ENOENT)
		return ret;

	/*
	 * return here -ENOENT, so env_init()
	 * can set the init bit and later if no
	 * other Environment storage is defined
	 * can set the default environment
	 */
	return -ENOENT;
}

U_BOOT_ENV_LOCATION(mtd) = {
	.location	= ENVL_MTD,
	ENV_NAME("MTD")
	.load		= env_mtd_load,
	.save		= ENV_SAVE_PTR(env_mtd_save),
	.erase		= ENV_ERASE_PTR(env_mtd_erase),
	.init		= env_mtd_init,
};

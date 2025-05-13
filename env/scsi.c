// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008-2011 Freescale Semiconductor, Inc.
 */

/* #define DEBUG */

#include <asm/global_data.h>

#include <command.h>
#include <env.h>
#include <env_internal.h>
#include <fdtdec.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <memalign.h>
#include <part.h>
#include <search.h>
#include <scsi.h>
#include <errno.h>
#include <dm/ofnode.h>

DECLARE_GLOBAL_DATA_PTR;
static env_t envbuf;

struct env_scsi_info {
	struct blk_desc *blk;
	struct disk_partition part;
	int count;
};

static struct env_scsi_info env_part;

static inline struct env_scsi_info *env_scsi_get_part(void)
{
	struct env_scsi_info *ep = &env_part;

	if (scsi_get_blk_by_uuid(CONFIG_SCSI_ENV_PART_UUID, &ep->blk, &ep->part))
		return NULL;

	ep->count = CONFIG_ENV_SIZE / ep->part.blksz;

	return ep;
}

static int env_scsi_save(void)
{
	struct env_scsi_info *ep = env_scsi_get_part();
	int ret;

	if (!ep)
		return -ENOENT;

	ret = env_export(&envbuf);
	if (ret)
		return ret;

	if (blk_dwrite(ep->blk, ep->part.start, ep->count, &envbuf) != ep->count)
		return -EIO;

	return 0;
}

static int env_scsi_erase(void)
{
	struct env_scsi_info *ep = env_scsi_get_part();

	if (!ep)
		return -ENOENT;

	return (int)blk_derase(ep->blk, ep->part.start, ep->count);
}

#if defined(ENV_IS_EMBEDDED)
static int env_scsi_load(void)
{
	return 0;
}
#else
static int env_scsi_load(void)
{
	struct env_scsi_info *ep = env_scsi_get_part();
	int ret;

	if (!ep) {
		env_set_default(CONFIG_SCSI_ENV_PART_UUID " partition not found", 0);
		return -ENOENT;
	}

	if (blk_dread(ep->blk, ep->part.start, ep->count, &envbuf) != ep->count) {
		env_set_default(CONFIG_SCSI_ENV_PART_UUID " partition read failed", 0);
		return -EIO;
	}

	ret = env_import((char *)&envbuf, 1, H_EXTERNAL);
	if (ret) {
		debug("ENV import failed\n");
		env_set_default("Cannot load environment", 0);
	} else {
		gd->env_addr = (ulong)envbuf.data;
	}

	return ret;
}
#endif

U_BOOT_ENV_LOCATION(scsi) = {
	.location	= ENVL_SCSI,
	ENV_NAME("SCSI")
	.load		= env_scsi_load,
#if defined(CONFIG_CMD_SAVEENV) && !defined(CONFIG_XPL_BUILD)
	.save		= env_save_ptr(env_scsi_save),
	.erase		= ENV_ERASE_PTR(env_scsi_erase),
#endif
};

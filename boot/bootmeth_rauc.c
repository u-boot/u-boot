// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmethod for distro boot with RAUC
 *
 * Copyright 2025 PHYTEC Messtechnik GmbH
 * Written by Martin Schwan <m.schwan@phytec.de>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <blk.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <bootstd.h>
#include <dm.h>
#include <env.h>
#include <fs.h>
#include <malloc.h>
#include <mapmem.h>
#include <string.h>
#include <asm/cache.h>

/* Length of env var "BOOT_*_LEFT" */
#define BOOT_LEFT_LEN	(5 + 32 + 5)

static const char * const script_names[] = { "boot.scr", "boot.scr.uimg", NULL };

/**
 * struct distro_rauc_slot - Slot information
 *
 * A slot describes the unit of a bootable system consisting of one or multiple
 * partitions. This usually includes a root filesystem, kernel and potentially other
 * files, like device trees and boot scripts for that particular distribution.
 *
 * @name	The slot name
 * @boot_part	The boot partition number on disk
 * @root_part	The root partition number on disk
 */
struct distro_rauc_slot {
	char *name;
	int boot_part;
	int root_part;
};

/**
 * struct distro_rauc_priv - Private data
 *
 * @slots	All slots of the device in default order
 * @boot_order	String of the current boot order containing the active slot names
 */
struct distro_rauc_priv {
	struct distro_rauc_slot **slots;
};

static void distro_rauc_priv_free(struct distro_rauc_priv *priv)
{
	int i;

	for (i = 0; priv->slots[i]; i++) {
		free(priv->slots[i]->name);
		free(priv->slots[i]);
	}
	free(priv->slots);
	free(priv);
}

static struct distro_rauc_slot *get_slot(struct distro_rauc_priv *priv,
					 const char *slot_name)
{
	int i;

	for (i = 0; priv->slots[i]->name; i++) {
		if (!strcmp(priv->slots[i]->name, slot_name))
			return priv->slots[i];
	}

	return NULL;
}

static int distro_rauc_check(struct udevice *dev, struct bootflow_iter *iter)
{
	/*
	 * This distro only works on whole MMC devices, as multiple partitions
	 * are needed for an A/B system.
	 */
	if (bootflow_iter_check_mmc(iter))
		return log_msg_ret("mmc", -EOPNOTSUPP);
	if (iter->part)
		return log_msg_ret("part", -EOPNOTSUPP);

	return 0;
}

static int distro_rauc_scan_parts(struct bootflow *bflow)
{
	struct blk_desc *desc;
	struct distro_rauc_priv *priv;
	char *boot_order;
	const char **boot_order_list;
	int ret;
	int i;

	if (bflow->blk)
		desc = dev_get_uclass_plat(bflow->blk);

	priv = bflow->bootmeth_priv;
	if (!priv || !priv->slots)
		return log_msg_ret("priv", -EINVAL);

	boot_order = env_get("BOOT_ORDER");
	boot_order_list = str_to_list(boot_order);
	for (i = 0; boot_order_list[i]; i++) {
		const struct distro_rauc_slot *slot;

		slot = get_slot(priv, boot_order_list[i]);
		if (!slot)
			return log_msg_ret("slot", -EINVAL);
		if (desc) {
			ret = fs_set_blk_dev_with_part(desc, slot->boot_part);
			if (ret)
				return log_msg_ret("part", ret);
			fs_close();
			ret = fs_set_blk_dev_with_part(desc, slot->root_part);
			if (ret)
				return log_msg_ret("part", ret);
			fs_close();
		}
	}
	str_free_list(boot_order_list);

	return 0;
}

static int distro_rauc_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	struct distro_rauc_priv *priv;
	int ret;
	char *slot;
	int i;
	char *partitions;
	char *boot_order;
	const char *default_boot_order;
	const char **default_boot_order_list;
	char *boot_order_copy;
	char boot_left[BOOT_LEFT_LEN];
	char *parts;

	/* Get RAUC variables or set their default values */
	boot_order = env_get("BOOT_ORDER");
	if (!boot_order) {
		log_debug("BOOT_ORDER did not exist yet, setting default value\n");
		if (env_set("BOOT_ORDER", CONFIG_BOOTMETH_RAUC_BOOT_ORDER))
			return log_msg_ret("env", -EPERM);
		boot_order = CONFIG_BOOTMETH_RAUC_BOOT_ORDER;
	}
	default_boot_order = CONFIG_BOOTMETH_RAUC_BOOT_ORDER;
	default_boot_order_list = str_to_list(default_boot_order);
	for (i = 0; default_boot_order_list[i]; i++) {
		sprintf(boot_left, "BOOT_%s_LEFT", default_boot_order_list[i]);
		if (!env_get(boot_left)) {
			log_debug("%s did not exist yet, setting default value\n",
				  boot_left);
			if (env_set_ulong(boot_left, CONFIG_BOOTMETH_RAUC_DEFAULT_TRIES))
				return log_msg_ret("env", -EPERM);
		}
	}
	str_free_list(default_boot_order_list);

	priv = calloc(1, sizeof(struct distro_rauc_priv));
	if (!priv)
		return log_msg_ret("buf", -ENOMEM);
	priv->slots = calloc(1, sizeof(struct distro_rauc_slot));

	/* Copy default boot_order, so we can leave the original unmodified */
	boot_order_copy = strdup(default_boot_order);
	partitions = strdup(CONFIG_BOOTMETH_RAUC_PARTITIONS);

	for (i = 1;
	     (parts = strsep(&partitions, " ")) &&
	     (slot = strsep(&boot_order_copy, " "));
	     i++) {
		struct distro_rauc_slot *s;
		struct distro_rauc_slot **new_slots;

		s = calloc(1, sizeof(struct distro_rauc_slot));
		s->name = strdup(slot);
		s->boot_part = simple_strtoul(strsep(&parts, ","), NULL, 10);
		s->root_part = simple_strtoul(strsep(&parts, ","), NULL, 10);
		new_slots = realloc(priv->slots, (i + 1) *
				    sizeof(struct distro_rauc_slot));
		if (!new_slots)
			return log_msg_ret("buf", -ENOMEM);
		priv->slots = new_slots;
		priv->slots[i - 1] = s;
		priv->slots[i] = NULL;
	}

	bflow->bootmeth_priv = priv;

	ret = distro_rauc_scan_parts(bflow);
	if (ret < 0) {
		distro_rauc_priv_free(priv);
		free(boot_order_copy);
		return ret;
	}

	bflow->state = BOOTFLOWST_READY;

	return 0;
}

static int distro_rauc_read_file(struct udevice *dev, struct bootflow *bflow,
				 const char *file_path, ulong addr,
				 enum bootflow_img_t type, ulong *sizep)
{
	/*
	 * Reading individual files is not supported since we only operate on
	 * whole MMC devices (because we require multiple partitions).
	 */
	return log_msg_ret("Unsupported", -ENOSYS);
}

static int distro_rauc_load_boot_script(struct bootflow *bflow,
					const struct distro_rauc_slot *slot)
{
	struct blk_desc *desc;
	struct distro_rauc_priv *priv;
	struct udevice *bootstd;
	const char *const *prefixes;
	int ret;
	int i;
	int j;

	ret = uclass_first_device_err(UCLASS_BOOTSTD, &bootstd);
	if (ret)
		return log_msg_ret("std", ret);
	prefixes = bootstd_get_prefixes(bootstd);

	desc = dev_get_uclass_plat(bflow->blk);
	priv = bflow->bootmeth_priv;
	if (!priv || !priv->slots)
		return log_msg_ret("priv", -EINVAL);

	bflow->part = slot->boot_part;
	if (!bflow->part)
		return log_msg_ret("part", -ENOENT);

	ret = bootmeth_setup_fs(bflow, desc);
	if (ret)
		return log_msg_ret("set", ret);

	for (i = 0; prefixes[i] && bflow->state != BOOTFLOWST_FILE; i++) {
		for (j = 0; script_names[j] && bflow->state != BOOTFLOWST_FILE; j++) {
			if (!bootmeth_try_file(bflow, desc, prefixes[i], script_names[j])) {
				log_debug("Found file '%s%s' in %s.part_%x\n",
					  prefixes[i], script_names[j],
					  bflow->dev->name, bflow->part);
				bflow->subdir = strdup(prefixes[i]);
			}
		}
	}
	if (bflow->state != BOOTFLOWST_FILE)
		return log_msg_ret("file", -ENOENT);

	ret = bootmeth_alloc_file(bflow, 0x10000, ARCH_DMA_MINALIGN,
				  (enum bootflow_img_t)IH_TYPE_SCRIPT);
	if (ret)
		return log_msg_ret("read", ret);

	return 0;
}

static int find_active_slot(char **slot_name, ulong *slot_tries)
{
	ulong tries;
	char boot_left[BOOT_LEFT_LEN];
	char *boot_order;
	const char **boot_order_list;
	bool slot_found = false;
	int ret;
	int i;

	boot_order = env_get("BOOT_ORDER");
	if (!boot_order)
		return log_msg_ret("env", -ENOENT);
	boot_order_list = str_to_list(boot_order);
	for (i = 0; boot_order_list[i] && !slot_found; i++) {
		sprintf(boot_left, "BOOT_%s_LEFT", boot_order_list[i]);
		tries = env_get_ulong(boot_left, 10, ULONG_MAX);
		if (tries == ULONG_MAX)
			return log_msg_ret("env", -ENOENT);

		if (tries) {
			ret = env_set_ulong(boot_left, tries - 1);
			if (ret)
				return log_msg_ret("env", ret);
			*slot_name = strdup(boot_order_list[i]);
			*slot_tries = tries;
			slot_found = true;
		}
	}
	str_free_list(boot_order_list);

	if (!slot_found) {
		if (IS_ENABLED(CONFIG_BOOTMETH_RAUC_RESET_ALL_ZERO_TRIES)) {
			log_warning("WARNING: No valid slot found\n");
			log_info("INFO: Resetting boot order and all slot tries\n");
			boot_order_list = str_to_list(CONFIG_BOOTMETH_RAUC_BOOT_ORDER);
			for (i = 0; boot_order_list[i]; i++) {
				sprintf(boot_left, "BOOT_%s_LEFT", boot_order_list[i]);
				ret = env_set_ulong(boot_left, CONFIG_BOOTMETH_RAUC_DEFAULT_TRIES);
				if (ret)
					return log_msg_ret("env", ret);
			}
			str_free_list(boot_order_list);
			ret = env_save();
			if (ret)
				return log_msg_ret("env", ret);
			do_reset(NULL, 0, 0, NULL);
		}
		log_err("ERROR: No valid slot found\n");
		return -EINVAL;
	}

	return 0;
}

static int distro_rauc_boot(struct udevice *dev, struct bootflow *bflow)
{
	struct blk_desc *desc;
	struct distro_rauc_priv *priv;
	const struct distro_rauc_slot *slot;
	char *boot_order;
	const char **boot_order_list;
	char *active_slot;
	ulong active_slot_tries;
	char raucargs[64];
	char boot_left[BOOT_LEFT_LEN];
	ulong addr;
	int ret = 0;
	int i;

	desc = dev_get_uclass_plat(bflow->blk);
	if (desc->uclass_id != UCLASS_MMC)
		return log_msg_ret("blk", -EINVAL);
	priv = bflow->bootmeth_priv;

	/* Device info variables */
	ret = env_set("devtype", blk_get_devtype(bflow->blk));
	if (ret)
		return log_msg_ret("env", ret);

	ret = env_set_hex("devnum", desc->devnum);
	if (ret)
		return log_msg_ret("env", ret);

	/* Find active, valid slot */
	ret = find_active_slot(&active_slot, &active_slot_tries);
	if (ret)
		return log_msg_ret("env", ret);

	/* Kernel command line arguments */
	sprintf(raucargs, "rauc.slot=%s", active_slot);
	ret = env_set("raucargs", raucargs);
	if (ret)
		return log_msg_ret("env", ret);

	/* Active slot info */
	slot = get_slot(priv, active_slot);
	if (!slot)
		return log_msg_ret("env", -ENOENT);
	ret = env_set_hex("distro_bootpart", slot->boot_part);
	if (ret)
		return log_msg_ret("env", ret);
	ret = env_set_hex("distro_rootpart", slot->root_part);
	if (ret)
		return log_msg_ret("env", ret);
	ret = env_save();
	if (ret)
		return log_msg_ret("env", ret);

	/* Load distro boot script */
	ret = distro_rauc_load_boot_script(bflow, slot);
	if (ret)
		return log_msg_ret("load", ret);

	log_info("INFO: Booting slot %s, %lu of %d tries left\n",
		 active_slot, active_slot_tries, CONFIG_BOOTMETH_RAUC_DEFAULT_TRIES);

	log_debug("devtype: %s\n", env_get("devtype"));
	log_debug("devnum: %s\n", env_get("devnum"));
	log_debug("distro_bootpart: %s\n", env_get("distro_bootpart"));
	log_debug("distro_rootpart: %s\n", env_get("distro_rootpart"));
	log_debug("raucargs: %s\n", env_get("raucargs"));
	boot_order = env_get("BOOT_ORDER");
	if (!boot_order)
		return log_msg_ret("env", -EPERM);
	log_debug("BOOT_ORDER: %s\n", boot_order);
	boot_order_list = str_to_list(boot_order);
	for (i = 0; boot_order_list[i]; i++) {
		sprintf(boot_left, "BOOT_%s_LEFT", boot_order_list[i]);
		log_debug("%s: %s\n", boot_left, env_get(boot_left));
	}
	str_free_list(boot_order_list);

	/* Run distro boot script */
	addr = map_to_sysmem(bflow->buf);
	ret = cmd_source_script(addr, NULL, NULL);
	if (ret)
		return log_msg_ret("boot", ret);

	distro_rauc_priv_free(priv);

	return 0;
}

static int distro_rauc_bootmeth_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = "RAUC distro boot from MMC";
	plat->flags = BOOTMETHF_ANY_PART;

	return 0;
}

static struct bootmeth_ops distro_rauc_bootmeth_ops = {
	.check		= distro_rauc_check,
	.read_bootflow	= distro_rauc_read_bootflow,
	.read_file	= distro_rauc_read_file,
	.boot		= distro_rauc_boot,
};

static const struct udevice_id distro_rauc_bootmeth_ids[] = {
	{ .compatible = "u-boot,distro-rauc" },
	{ }
};

U_BOOT_DRIVER(bootmeth_rauc) = {
	.name		= "bootmeth_rauc",
	.id		= UCLASS_BOOTMETH,
	.of_match	= distro_rauc_bootmeth_ids,
	.ops		= &distro_rauc_bootmeth_ops,
	.bind		= distro_rauc_bootmeth_bind,
};

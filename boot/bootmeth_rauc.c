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
#include <bootmeth_rauc.h>
#include <bootstd.h>
#include <dm.h>
#include <env.h>
#include <fs.h>
#include <malloc.h>
#include <mapmem.h>
#include <part.h>
#include <string.h>
#include <linux/err.h>
#include <linux/stringify.h>
#include <asm/cache.h>

/* Length of env var "BOOT_*_LEFT", including the terminating NUL */
#define BOOT_LEFT_LEN	(5 + 32 + 5 + 1)

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
 * @boot_part_name	The name of the boot partition, or NULL if it is given by
 *			its number
 * @root_part_name	The name of the root partition, or NULL if it is given by
 *			its number
 */
struct distro_rauc_slot {
	char *name;
	int boot_part;
	int root_part;
	char *boot_part_name;
	char *root_part_name;
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

static void distro_rauc_slot_free(struct distro_rauc_slot *slot)
{
	free(slot->name);
	free(slot->boot_part_name);
	free(slot->root_part_name);
	free(slot);
}

static void distro_rauc_priv_free(struct distro_rauc_priv *priv)
{
	int i;

	for (i = 0; priv->slots[i]; i++)
		distro_rauc_slot_free(priv->slots[i]);
	free(priv->slots);
	free(priv);
}

static int boot_left_name(char *buf, const char *slot_name)
{
	if (snprintf(buf, BOOT_LEFT_LEN, "BOOT_%s_LEFT", slot_name) >=
	    BOOT_LEFT_LEN)
		return log_msg_ret("name", -E2BIG);

	return 0;
}

static struct distro_rauc_slot *get_slot(struct distro_rauc_priv *priv,
					 const char *slot_name)
{
	int i;

	for (i = 0; priv->slots[i]; i++) {
		if (!strcmp(priv->slots[i]->name, slot_name))
			return priv->slots[i];
	}

	return NULL;
}

/**
 * distro_rauc_parse_part() - Parse a partition of a slot
 *
 * @str: Partition number, or name of a GPT partition
 * @part: Returns the partition number, if @str is a number
 * @name: Returns a copy of @str, if it is not a number
 * Return: 0 if OK, -EINVAL if @str is empty, -ENOMEM if out of memory
 */
static int distro_rauc_parse_part(const char *str, int *part, char **name)
{
	if (!str || !*str)
		return log_msg_ret("part", -EINVAL);

	if (strspn(str, "0123456789") == strlen(str)) {
		*part = simple_strtoul(str, NULL, 10);
		return 0;
	}

	*name = strdup(str);
	if (!*name)
		return log_msg_ret("buf", -ENOMEM);

	return 0;
}

/**
 * distro_rauc_find_parts() - Find the partitions of a slot by their names
 *
 * @desc: Block device to search
 * @slot: Slot to update, if its partitions are given by name
 * Return: 0 if OK, -ENOENT if a partition was not found
 */
static int distro_rauc_find_parts(struct blk_desc *desc,
				  struct distro_rauc_slot *slot)
{
	struct disk_partition info;
	int part;

	if (slot->boot_part_name) {
		part = part_get_info_by_name(desc, slot->boot_part_name, &info);
		if (part < 0)
			return log_msg_ret("boot", -ENOENT);
		slot->boot_part = part;
	}
	if (slot->root_part_name) {
		part = part_get_info_by_name(desc, slot->root_part_name, &info);
		if (part < 0)
			return log_msg_ret("root", -ENOENT);
		slot->root_part = part;
	}

	return 0;
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
	struct blk_desc *desc = NULL;
	struct distro_rauc_priv *priv;
	struct disk_partition fs_info;
	char *boot_order;
	const char **boot_order_list;
	bool slot_found = false;
	int ret;
	int i;

	if (bflow->blk)
		desc = dev_get_uclass_plat(bflow->blk);

	priv = bflow->bootmeth_priv;
	if (!priv || !priv->slots)
		return log_msg_ret("priv", -EINVAL);

	/* With partition names, the partitions of all slots must exist */
	if (desc) {
		for (i = 0; priv->slots[i]; i++) {
			ret = distro_rauc_find_parts(desc, priv->slots[i]);
			if (ret)
				return ret;
		}
	}

	/* BOOT_ORDER is only set once a device has been found */
	boot_order = env_get("BOOT_ORDER");
	if (!boot_order)
		boot_order = CONFIG_BOOTMETH_RAUC_BOOT_ORDER;
	boot_order_list = str_to_list(boot_order);
	for (i = 0; boot_order_list[i]; i++) {
		const struct distro_rauc_slot *slot;

		slot = get_slot(priv, boot_order_list[i]);
		if (!slot) {
			str_free_list(boot_order_list);
			return log_msg_ret("slot", -EINVAL);
		}
		if (desc) {
			ret = fs_set_blk_dev_with_part(desc, slot->boot_part);
			if (ret)
				continue;
			fs_close();
			ret = part_get_info(desc, slot->root_part, &fs_info);
			if (ret)
				continue;
			slot_found = true;
		}
	}
	str_free_list(boot_order_list);

	if (slot_found)
		return 0;

	return -1;
}

/**
 * distro_rauc_set_defaults() - Set the RAUC variables that do not exist yet
 *
 * This sets BOOT_ORDER and the BOOT_x_LEFT variable of each slot to their
 * default values, unless they exist already.
 *
 * Return: 0 if OK, -ve on error
 */
static int distro_rauc_set_defaults(void)
{
	const char **default_boot_order_list;
	char boot_left[BOOT_LEFT_LEN];
	int ret;
	int i;

	if (!env_get("BOOT_ORDER")) {
		log_debug("BOOT_ORDER did not exist yet, setting default value\n");
		if (env_set("BOOT_ORDER", CONFIG_BOOTMETH_RAUC_BOOT_ORDER))
			return log_msg_ret("env", -EPERM);
	}
	default_boot_order_list = str_to_list(CONFIG_BOOTMETH_RAUC_BOOT_ORDER);
	for (i = 0; default_boot_order_list[i]; i++) {
		ret = boot_left_name(boot_left, default_boot_order_list[i]);
		if (ret) {
			log_err("ERROR: Slot name %s is too long\n",
				default_boot_order_list[i]);
			str_free_list(default_boot_order_list);
			return ret;
		}
		if (!env_get(boot_left)) {
			log_debug("%s did not exist yet, setting default value\n",
				  boot_left);
			if (env_set_ulong(boot_left, CONFIG_BOOTMETH_RAUC_DEFAULT_TRIES)) {
				str_free_list(default_boot_order_list);
				return log_msg_ret("env", -EPERM);
			}
		}
	}
	str_free_list(default_boot_order_list);

	return 0;
}

static int distro_rauc_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	const struct distro_rauc_plat *rplat = dev_get_plat(dev);
	struct distro_rauc_priv *priv = NULL;
	int ret = 0;
	char *slot;
	int i;
	char *partitions = NULL;
	char *partitions_cursor;
	const char *default_boot_order;
	char *boot_order_copy;
	char *boot_order_cursor;
	char *parts;

	default_boot_order = CONFIG_BOOTMETH_RAUC_BOOT_ORDER;

	priv = calloc(1, sizeof(struct distro_rauc_priv));
	if (!priv)
		return log_msg_ret("buf", -ENOMEM);
	priv->slots = calloc(1, sizeof(struct distro_rauc_slot));
	if (!priv->slots) {
		free(priv);
		return log_msg_ret("buf", -ENOMEM);
	}

	/* Copy default boot_order, so we can leave the original unmodified */
	boot_order_copy = strdup(default_boot_order);
	if (!boot_order_copy) {
		ret = log_msg_ret("buf", -ENOMEM);
		goto rauc_read_bootflow_err;
	}
	partitions = strdup(rplat->partitions);
	if (!partitions) {
		ret = log_msg_ret("buf", -ENOMEM);
		goto rauc_read_bootflow_err;
	}

	partitions_cursor = partitions;
	boot_order_cursor = boot_order_copy;
	for (i = 1;
	     (parts = strsep(&partitions_cursor, " ")) &&
	     (slot = strsep(&boot_order_cursor, " "));
	     i++) {
		struct distro_rauc_slot *s;
		struct distro_rauc_slot **new_slots;

		s = calloc(1, sizeof(struct distro_rauc_slot));
		if (!s) {
			ret = log_msg_ret("buf", -ENOMEM);
			goto rauc_read_bootflow_err;
		}
		s->name = strdup(slot);
		if (!s->name) {
			free(s);
			ret = log_msg_ret("buf", -ENOMEM);
			goto rauc_read_bootflow_err;
		}
		ret = distro_rauc_parse_part(strsep(&parts, ","), &s->boot_part,
					     &s->boot_part_name);
		if (!ret)
			ret = distro_rauc_parse_part(strsep(&parts, ","),
						     &s->root_part,
						     &s->root_part_name);
		if (ret) {
			if (ret == -EINVAL)
				log_err("ERROR: Invalid partitions for slot %s\n",
					slot);
			distro_rauc_slot_free(s);
			goto rauc_read_bootflow_err;
		}
		new_slots = realloc(priv->slots, (i + 1) *
				    sizeof(struct distro_rauc_slot));
		if (!new_slots) {
			distro_rauc_slot_free(s);
			ret = log_msg_ret("buf", -ENOMEM);
			goto rauc_read_bootflow_err;
		}
		priv->slots = new_slots;
		priv->slots[i - 1] = s;
		priv->slots[i] = NULL;
	}

	bflow->bootmeth_priv = priv;

	ret = distro_rauc_scan_parts(bflow);
	if (ret < 0)
		goto rauc_read_bootflow_err;

	/* Only touch the environment for a device with a RAUC system */
	ret = distro_rauc_set_defaults();
	if (ret)
		goto rauc_read_bootflow_err;

	bflow->state = BOOTFLOWST_READY;

	free(boot_order_copy);
	free(partitions);

	return 0;

rauc_read_bootflow_err:
	distro_rauc_priv_free(priv);
	bflow->bootmeth_priv = NULL;
	free(boot_order_copy);
	free(partitions);

	return ret;
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

	/* Drop the boot script of a previous, failed boot attempt */
	free(bflow->buf);
	bflow->buf = NULL;
	free(bflow->subdir);
	bflow->subdir = NULL;
	bootflow_free_images(bflow);
	bflow->state = BOOTFLOWST_READY;

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
				if (!bflow->subdir)
					return log_msg_ret("buf", -ENOMEM);
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

static const struct distro_rauc_slot *find_active_slot(struct distro_rauc_priv *priv,
						       ulong *slot_tries)
{
	ulong tries;
	char boot_left[BOOT_LEFT_LEN];
	char *boot_order;
	const struct distro_rauc_slot *slot = NULL;
	const char **boot_order_list;
	int ret;
	int i;

	boot_order = env_get("BOOT_ORDER");
	if (!boot_order)
		return ERR_PTR(log_msg_ret("env", -ENOENT));
	boot_order_list = str_to_list(boot_order);
	for (i = 0; boot_order_list[i] && !slot; i++) {
		ret = boot_left_name(boot_left, boot_order_list[i]);
		if (ret) {
			str_free_list(boot_order_list);
			return ERR_PTR(ret);
		}
		tries = env_get_ulong(boot_left, 10, ULONG_MAX);
		if (tries == ULONG_MAX) {
			str_free_list(boot_order_list);
			return ERR_PTR(log_msg_ret("env", -ENOENT));
		}

		if (tries) {
			slot = get_slot(priv, boot_order_list[i]);
			if (!slot) {
				str_free_list(boot_order_list);
				return ERR_PTR(log_msg_ret("slot", -ENOENT));
			}
			ret = env_set_ulong(boot_left, tries - 1);
			if (ret) {
				str_free_list(boot_order_list);
				return ERR_PTR(log_msg_ret("env", ret));
			}
			*slot_tries = tries;
		}
	}
	str_free_list(boot_order_list);

	if (!slot) {
		if (IS_ENABLED(CONFIG_BOOTMETH_RAUC_RESET_ALL_ZERO_TRIES)) {
			log_warning("WARNING: No valid slot found\n");
			log_info("INFO: Resetting all slot tries to " __stringify(CONFIG_BOOTMETH_RAUC_DEFAULT_TRIES) "\n");
			boot_order_list = str_to_list(CONFIG_BOOTMETH_RAUC_BOOT_ORDER);
			for (i = 0; boot_order_list[i]; i++) {
				ret = boot_left_name(boot_left, boot_order_list[i]);
				if (ret) {
					str_free_list(boot_order_list);
					return ERR_PTR(ret);
				}
				ret = env_set_ulong(boot_left, CONFIG_BOOTMETH_RAUC_DEFAULT_TRIES);
				if (ret) {
					str_free_list(boot_order_list);
					return ERR_PTR(log_msg_ret("env", ret));
				}
			}
			str_free_list(boot_order_list);
			ret = env_save();
			if (ret)
				return ERR_PTR(log_msg_ret("env", ret));
			do_reset(NULL, 0, 0, NULL);
		}
		log_err("ERROR: No valid slot found\n");
		return ERR_PTR(-EINVAL);
	}

	return slot;
}

static int distro_rauc_boot(struct udevice *dev, struct bootflow *bflow)
{
	const struct distro_rauc_plat *rplat = dev_get_plat(dev);
	struct blk_desc *desc;
	struct distro_rauc_priv *priv;
	const struct distro_rauc_slot *slot;
	char *boot_order;
	const char **boot_order_list;
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
	if (!priv || !priv->slots)
		return log_msg_ret("priv", -EINVAL);

	/* Device info variables */
	ret = env_set("devtype", blk_get_devtype(bflow->blk));
	if (ret)
		return log_msg_ret("env", ret);

	ret = env_set_hex("devnum", desc->devnum);
	if (ret)
		return log_msg_ret("env", ret);

	/* Find active, valid slot */
	slot = find_active_slot(priv, &active_slot_tries);
	if (IS_ERR(slot))
		return log_msg_ret("env", PTR_ERR(slot));

	/* Kernel command line arguments */
	sprintf(raucargs, "rauc.slot=%s", slot->name);
	ret = env_set("raucargs", raucargs);
	if (ret)
		return log_msg_ret("env", ret);
	ret = env_set_hex("distro_bootpart", slot->boot_part);
	if (ret)
		return log_msg_ret("env", ret);
	ret = env_set_hex("distro_rootpart", slot->root_part);
	if (ret)
		return log_msg_ret("env", ret);
	ret = env_save();
	if (ret)
		return log_msg_ret("env", ret);

	/* Load distro boot script, unless a built-in command boots the slot */
	if (!*rplat->bootcmd) {
		ret = distro_rauc_load_boot_script(bflow, slot);
		if (ret)
			return log_msg_ret("load", ret);
	}

	log_info("INFO: Booting slot %s, %lu of %d tries left\n",
		 slot->name, active_slot_tries, CONFIG_BOOTMETH_RAUC_DEFAULT_TRIES);

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
		if (!boot_left_name(boot_left, boot_order_list[i]))
			log_debug("%s: %s\n", boot_left, env_get(boot_left));
	}
	str_free_list(boot_order_list);

	if (*rplat->bootcmd) {
		/* Run built-in boot command */
		ret = run_command(rplat->bootcmd, 0);
	} else {
		/* Run distro boot script */
		addr = map_to_sysmem(bflow->buf);
		ret = cmd_source_script(addr, NULL, NULL);
	}
	if (ret)
		return log_msg_ret("boot", ret);

	distro_rauc_priv_free(priv);
	bflow->bootmeth_priv = NULL;

	return 0;
}

static int distro_rauc_bootmeth_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);
	struct distro_rauc_plat *rplat = dev_get_plat(dev);

	plat->desc = "RAUC distro boot from MMC";
	plat->flags = BOOTMETHF_ANY_PART;
	rplat->partitions = CONFIG_BOOTMETH_RAUC_PARTITIONS;
	rplat->bootcmd = CONFIG_BOOTMETH_RAUC_BOOTCMD;

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
	.plat_auto	= sizeof(struct distro_rauc_plat),
};

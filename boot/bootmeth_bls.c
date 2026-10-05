// SPDX-License-Identifier: GPL-2.0+
// SPDX-FileCopyrightText: 2026 Flipper FZCO
/*
 * Bootmethod for the Boot Loader Specification (type #1 entry files)
 *
 * Reuses the pxelinux parser/boot path: each on-disk entry is read into a
 * struct pxe_label via parse_label_keys() and booted via label_boot().
 *
 * Spec: https://uapi-group.org/specifications/specs/boot_loader_specification/
 *
 * TODO: a partition typically holds several BLS entries, but the bootstd
 * framework currently allows only one bootflow per (bootmeth, partition)
 * pair, so this bootmeth surfaces only the highest-sorting entry. Once the
 * framework grows a way for a bootmeth to emit multiple bootflows from a
 * single partition, this should expose every discovered entry so the user
 * can pick from the standard 'bootflow menu' UI rather than be limited to
 * the default pick.
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <asm/cache.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <bootstd.h>
#include <command.h>
#include <dm.h>
#include <extlinux.h>
#include <fs.h>
#include <malloc.h>
#include <mapmem.h>
#include <pxe_utils.h>
#include <linux/sizes.h>

#define BLS_DIR		"loader/entries"
#define BLS_SUFFIX	".conf"

static int bls_check(struct udevice *dev, struct bootflow_iter *iter)
{
	int ret;

	/* This only works on block devices */
	ret = bootflow_iter_check_blk(iter);
	if (ret)
		return log_msg_ret("blk", ret);

	return 0;
}

/**
 * bls_pick_entry() - Find the highest-sorting *.conf across bootstd prefixes
 *
 * Walks ``<prefix>/loader/entries/`` for each prefix in @prefixes and
 * returns the lexicographically maximum full path seen.
 *
 * The spec leaves ordering between prefixes unspecified, as it only deals with
 * a dedicated boot/XBOOTLDR/ESP partition and not a $BOOT as a directory.
 * Comparing full paths is a deterministic-and-cheap stand-in.
 *
 * The Boot Loader Specification says entries should be sorted by sort-key
 * (descending), then version (descending), then filename (descending). For
 * the time being only the filename criterion is implemented, which is
 * sufficient for most distros that encode kernel version into the filename.
 *
 * TODO: implement proper spec-compliant ordering. That requires reading
 * each candidate entry, parsing its 'sort-key' and 'version' fields (the
 * latter compared with strverscmp()-style logic), and only falling back to
 * filename order when those tie.
 *
 * The prefix the winning entry was found under is recorded in
 * @bflow->subdir, so that 'bootflow info' can report where it came from.
 *
 * @prefixes:	NULL-terminated array of bootstd prefixes to search
 * @desc:	Block descriptor (used to re-mount per prefix)
 * @bflow:	Bootflow being populated (used to re-mount per prefix)
 * @fullp:	Returns the chosen full path (allocated), or NULL if none
 * Return: 0 on success, -ENOENT if no entry was found, < 0 on other error
 */
static int bls_pick_entry(const char *const *prefixes, struct blk_desc *desc,
			  struct bootflow *bflow, char **fullp)
{
	const char *best_prefix = NULL;
	char dirpath[256];
	char *best = NULL;
	int ret;
	int i;

	for (i = 0; prefixes && prefixes[i]; i++) {
		struct fs_dir_stream *dirs;
		struct fs_dirent *dent;

		/* fs_closedir() below resets the global fs_type. */
		ret = bootmeth_setup_fs(bflow, desc);
		if (ret) {
			free(best);
			return log_msg_ret("fs", ret);
		}

		snprintf(dirpath, sizeof(dirpath), "%s%s",
			 prefixes[i], BLS_DIR);
		dirs = fs_opendir(dirpath);
		if (!dirs)
			continue;

		while ((dent = fs_readdir(dirs))) {
			size_t len = strlen(dent->name);
			char *full;

			if (dent->type != FS_DT_REG)
				continue;
			if (len <= strlen(BLS_SUFFIX))
				continue;
			if (strcmp(dent->name + len - strlen(BLS_SUFFIX),
				   BLS_SUFFIX))
				continue;

			full = malloc(strlen(dirpath) + 1 + len + 1);
			if (!full) {
				free(best);
				fs_closedir(dirs);
				return -ENOMEM;
			}
			sprintf(full, "%s/%s", dirpath, dent->name);

			if (!best || strcmp(full, best) > 0) {
				free(best);
				best = full;
				best_prefix = prefixes[i];
			} else {
				free(full);
			}
		}
		fs_closedir(dirs);
	}

	if (!best)
		return -ENOENT;

	free(bflow->subdir);
	bflow->subdir = strdup(best_prefix);
	if (!bflow->subdir) {
		free(best);
		return -ENOMEM;
	}

	*fullp = best;

	return 0;
}

/*
 * TODO: BLS entry filenames may carry a boot-counter suffix of the form
 * '+TRIES_LEFT[-TRIES_DONE]' immediately before the .conf extension (see
 * the spec section on "Boot counting"). When that is implemented, this
 * bootmeth should:
 *   - parse and strip the suffix from the displayed entry name,
 *   - skip entries whose TRIES_LEFT has reached zero,
 *   - decrement TRIES_LEFT (renaming the file) on each boot attempt.
 * For now the suffix is left intact in the entry name and ignored.
 */
static int bls_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	struct pxe_label *label = NULL;
	struct pxe_menu scratch = {};
	const char *const *prefixes;
	struct udevice *bootstd;
	struct blk_desc *desc;
	char *fpath = NULL;
	const char *base;
	char *body;
	int ret;

	ret = uclass_first_device_err(UCLASS_BOOTSTD, &bootstd);
	if (ret)
		return log_msg_ret("std", ret);

	/* We require a partitioned block device */
	if (!bflow->blk || !bflow->part)
		return -ENOENT;

	desc = dev_get_uclass_plat(bflow->blk);
	prefixes = bootstd_get_prefixes(bootstd);

	ret = bls_pick_entry(prefixes, desc, bflow, &fpath);
	if (ret)
		return log_msg_ret("scan", ret);

	base = strrchr(fpath, '/');
	base = base ? base + 1 : fpath;

	/*
	 * bls_pick_entry() finished with fs_closedir(), which resets the
	 * global fs_type. Re-mount the partition so bootmeth_try_file()'s
	 * internal fs_size() call can find the right filesystem driver.
	 */
	ret = bootmeth_setup_fs(bflow, desc);
	if (ret) {
		free(fpath);
		return log_msg_ret("fs", ret);
	}

	ret = bootmeth_try_file(bflow, desc, NULL, fpath);
	if (ret) {
		free(fpath);
		return log_msg_ret("try", ret);
	}

	ret = bootmeth_alloc_file(bflow, SZ_64K, ARCH_DMA_MINALIGN,
				  BFI_EXTLINUX_CFG);
	if (ret) {
		free(fpath);
		return log_msg_ret("read", ret);
	}

	label = label_create();
	if (!label) {
		ret = -ENOMEM;
		goto err;
	}

	/*
	 * BLS files have no 'label NAME' header so derive the label name from
	 * the basename (without the .conf suffix) so messages are useful.
	 */
	label->name = strndup(base, strlen(base) - strlen(BLS_SUFFIX));
	if (!label->name) {
		ret = -ENOMEM;
		goto err;
	}

	body = bflow->buf;
	ret = parse_label_keys(&body, &scratch, label, true);
	if (ret < 0)
		goto err;

	/*
	 * scratch is only used to give parse_label_keys() somewhere safe to
	 * stash menu-level state (e.g. a stray 'menu default' line). BLS
	 * entry files don't contain such lines but defensively free anything
	 * that did get allocated.
	 */
	free(scratch.default_label);

	/*
	 * label->menu was populated either from a BLS 'title' line (the
	 * spec-mandated human-readable name) or from a stray 'menu label'
	 * the parser may have picked up. Fall back to the filename-derived
	 * label name when neither is present.
	 */
	bflow->os_name = strdup(label->menu ? label->menu : label->name);
	if (!bflow->os_name) {
		ret = -ENOMEM;
		goto err;
	}

	/*
	 * Don't retain the parsed label. bflow->bootmeth_priv is freed by the
	 * bootstd core with a single flat free(), which would leak the label's
	 * many separately-allocated string members. Worse, a bootflow is
	 * shallow-copied into the bootflow list (bootstd_add_bootflow()), so a
	 * retained pointer is shared between the iterator's temporary and the
	 * stored copy and can be freed twice, corrupting the heap.
	 *
	 * The raw config text remains in bflow->buf (owned and freed by the
	 * core); bls_boot() re-parses it when the entry is actually booted,
	 * mirroring how bootmeth_extlinux works.
	 */
	label_destroy(label);
	free(fpath);

	return 0;

err:
	free(scratch.default_label);
	if (label)
		label_destroy(label);
	free(fpath);
	return log_msg_ret("bls", ret);
}

static int bls_boot(struct udevice *dev, struct bootflow *bflow)
{
	struct cmd_tbl cmdtp = {};	/* dummy */
	struct pxe_context ctx;
	struct extlinux_info info;
	struct pxe_menu scratch = {};
	struct pxe_label *label;
	char *body;
	int ret;

	if (!bflow->buf)
		return log_msg_ret("buf", -ENOENT);

	label = label_create();
	if (!label)
		return log_msg_ret("lbl", -ENOMEM);

	/* Give label_boot() a useful name for its progress messages. */
	if (bflow->os_name) {
		label->name = strdup(bflow->os_name);
		if (!label->name) {
			ret = -ENOMEM;
			goto out;
		}
	}

	body = bflow->buf;
	ret = parse_label_keys(&body, &scratch, label, true);
	if (ret < 0)
		goto out;

	info.dev = dev;
	info.bflow = bflow;

	/*
	 * The spec has paths inside an entry "always relative to the root
	 * directory of the partition they are referenced from", which is what
	 * the bootmeth sees regardless of the prefix the entry was found
	 * under. allow_abs_path=true honours that, and passing NULL as the
	 * bootfile keeps the prefix empty so the paths are not rebased.
	 */
	ret = pxe_setup_ctx(&ctx, &cmdtp, extlinux_getfile, &info, true,
			    NULL, false, false);
	if (ret) {
		ret = -EINVAL;
		goto out;
	}

	ret = label_boot(&ctx, label);
	pxe_destroy_ctx(&ctx);
	if (ret)
		ret = -EINVAL;

out:
	free(scratch.default_label);
	label_destroy(label);

	return ret < 0 ? log_msg_ret("boot", ret) : 0;
}

static int bls_bootmeth_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = IS_ENABLED(CONFIG_BOOTSTD_FULL) ?
		"Boot Loader Specification" : "bls";

	return 0;
}

static struct bootmeth_ops bls_bootmeth_ops = {
	.check		= bls_check,
	.read_bootflow	= bls_read_bootflow,
	.read_file	= bootmeth_common_read_file,
	.boot		= bls_boot,
};

static const struct udevice_id bls_bootmeth_ids[] = {
	{ .compatible = "u-boot,bls" },
	{ }
};

U_BOOT_DRIVER(bootmeth_bls) = {
	.name		= "bootmeth_bls",
	.id		= UCLASS_BOOTMETH,
	.of_match	= bls_bootmeth_ids,
	.ops		= &bls_bootmeth_ops,
	.bind		= bls_bootmeth_bind,
};

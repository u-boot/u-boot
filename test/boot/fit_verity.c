// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for FIT dm-verity cmdline generation
 *
 * Copyright 2026 Daniel Golle <daniel@makrotopia.org>
 */

#include <image.h>
#include <test/test.h>
#include <test/ut.h>

#define FIT_VERITY_TEST(_name, _flags)	UNIT_TEST(_name, _flags, fit_verity)

/* FIT blob buffer size — generous to avoid FDT_ERR_NOSPACE */
#define FIT_BUF_SIZE	4096

/* Test digest (32 bytes = sha256) */
static const u8 test_digest[32] = {
	0x8e, 0x67, 0x91, 0x63, 0x7f, 0x93, 0xcb, 0xb8,
	0x1f, 0xc4, 0x52, 0x99, 0xe2, 0x03, 0xcb, 0xe8,
	0x5c, 0xa2, 0xe4, 0x7a, 0x38, 0xf5, 0x05, 0x1b,
	0xdd, 0xee, 0xce, 0x92, 0xd7, 0xb1, 0xc9, 0xf9,
};

/* Test salt (32 bytes) */
static const u8 test_salt[32] = {
	0xaa, 0x7b, 0x11, 0xf8, 0xdb, 0x8f, 0xe2, 0xe5,
	0xbf, 0xd4, 0xec, 0xa1, 0xd1, 0x8a, 0x22, 0xb5,
	0xde, 0x7e, 0xa3, 0x9d, 0x2e, 0x1b, 0x93, 0xbb,
	0x72, 0x72, 0xce, 0x0c, 0x6c, 0xa3, 0xcc, 0x8e,
};

/**
 * build_verity_fit() - construct a minimal FIT blob with dm-verity metadata
 * @buf:		output buffer (at least FIT_BUF_SIZE bytes)
 * @num_loadables:	number of filesystem loadables to create (1 or 2)
 *
 * Builds a FIT blob containing:
 *  - /images/rootfsN  with type="filesystem" and a dm-verity subnode
 *  - /configurations/conf-1  referencing the loadable(s)
 *
 * Return: configuration node offset, or -ve on error
 */
static int build_verity_fit(void *buf, int num_loadables)
{
	int images_node, conf_node, confs_node, img_node, verity_node;
	fdt32_t val;
	int ret, i;
	char name[32];
	/*
	 * Build the loadables string list. FDT stringlists are concatenated
	 * NUL-terminated strings. E.g. "rootfs0\0rootfs1\0"
	 */
	char loadables[128];
	int loadables_len = 0;

	ret = fdt_create_empty_tree(buf, FIT_BUF_SIZE);
	if (ret)
		return ret;

	/* /images */
	images_node = fdt_add_subnode(buf, 0, "images");
	if (images_node < 0)
		return images_node;

	for (i = 0; i < num_loadables; i++) {
		snprintf(name, sizeof(name), "rootfs%d", i);

		img_node = fdt_add_subnode(buf, images_node, name);
		if (img_node < 0)
			return img_node;

		ret = fdt_setprop_string(buf, img_node, FIT_TYPE_PROP,
					 "filesystem");
		if (ret)
			return ret;

		verity_node = fdt_add_subnode(buf, img_node,
					      FIT_VERITY_NODENAME);
		if (verity_node < 0)
			return verity_node;

		ret = fdt_setprop_string(buf, verity_node,
					 FIT_VERITY_ALGO_PROP, "sha256");
		if (ret)
			return ret;

		val = cpu_to_fdt32(4096);
		ret = fdt_setprop(buf, verity_node, FIT_VERITY_DBS_PROP,
				  &val, sizeof(val));
		if (ret)
			return ret;

		ret = fdt_setprop(buf, verity_node, FIT_VERITY_HBS_PROP,
				  &val, sizeof(val));
		if (ret)
			return ret;

		val = cpu_to_fdt32(100);
		ret = fdt_setprop(buf, verity_node, FIT_VERITY_NBLK_PROP,
				  &val, sizeof(val));
		if (ret)
			return ret;

		val = cpu_to_fdt32(100);
		ret = fdt_setprop(buf, verity_node, FIT_VERITY_HBLK_PROP,
				  &val, sizeof(val));
		if (ret)
			return ret;

		ret = fdt_setprop(buf, verity_node, FIT_VERITY_DIGEST_PROP,
				  test_digest, sizeof(test_digest));
		if (ret)
			return ret;

		ret = fdt_setprop(buf, verity_node, FIT_VERITY_SALT_PROP,
				  test_salt, sizeof(test_salt));
		if (ret)
			return ret;

		/* Append to loadables stringlist */
		loadables_len += snprintf(loadables + loadables_len,
					  sizeof(loadables) - loadables_len,
					  "%s", name) + 1;
	}

	/* /configurations/conf-1 */
	confs_node = fdt_add_subnode(buf, 0, "configurations");
	if (confs_node < 0)
		return confs_node;

	conf_node = fdt_add_subnode(buf, confs_node, "conf-1");
	if (conf_node < 0)
		return conf_node;

	ret = fdt_setprop(buf, conf_node, FIT_LOADABLE_PROP,
			  loadables, loadables_len);
	if (ret)
		return ret;

	return conf_node;
}

/* Test: single dm-verity loadable produces correct cmdline fragments */
static int fit_verity_test_single(struct unit_test_state *uts)
{
	char buf[FIT_BUF_SIZE];
	struct bootm_headers images;
	int conf_noffset;

	conf_noffset = build_verity_fit(buf, 1);
	ut_assert(conf_noffset >= 0);

	memset(&images, 0, sizeof(images));
	ut_assertok(fit_verity_build_cmdline(buf, conf_noffset, &images));

	/* dm_mod_create should contain the target spec for rootfs0 */
	ut_assertnonnull(images.dm_mod_create);
	ut_assert(strstr(images.dm_mod_create, "rootfs0,,,"));
	ut_assert(strstr(images.dm_mod_create, "verity 1"));
	ut_assert(strstr(images.dm_mod_create, "/dev/fit0"));
	ut_assert(strstr(images.dm_mod_create, "4096 4096 100 100"));
	ut_assert(strstr(images.dm_mod_create, "sha256"));
	/* Check hex-encoded digest prefix */
	ut_assert(strstr(images.dm_mod_create, "8e6791637f93cbb8"));
	/* Check hex-encoded salt prefix */
	ut_assert(strstr(images.dm_mod_create, "aa7b11f8db8fe2e5"));

	/* dm_mod_waitfor should reference /dev/fit0 */
	ut_assertnonnull(images.dm_mod_waitfor);
	ut_asserteq_str("/dev/fit0", images.dm_mod_waitfor);

	fit_verity_free(&images);
	ut_assertnull(images.dm_mod_create);
	ut_assertnull(images.dm_mod_waitfor);

	return 0;
}
FIT_VERITY_TEST(fit_verity_test_single, 0);

/* Test: FIT with no dm-verity subnode returns 0, pointers stay NULL */
static int fit_verity_test_no_verity(struct unit_test_state *uts)
{
	char buf[FIT_BUF_SIZE];
	struct bootm_headers images;
	int conf_node, images_node, img_node, confs_node;
	int ret;

	ret = fdt_create_empty_tree(buf, FIT_BUF_SIZE);
	ut_assertok(ret);

	images_node = fdt_add_subnode(buf, 0, "images");
	ut_assert(images_node >= 0);

	img_node = fdt_add_subnode(buf, images_node, "rootfs");
	ut_assert(img_node >= 0);
	ut_assertok(fdt_setprop_string(buf, img_node, FIT_TYPE_PROP,
				       "filesystem"));
	/* No dm-verity subnode */

	confs_node = fdt_add_subnode(buf, 0, "configurations");
	ut_assert(confs_node >= 0);
	conf_node = fdt_add_subnode(buf, confs_node, "conf-1");
	ut_assert(conf_node >= 0);
	ut_assertok(fdt_setprop_string(buf, conf_node, FIT_LOADABLE_PROP,
				       "rootfs"));

	memset(&images, 0, sizeof(images));
	ut_asserteq(0, fit_verity_build_cmdline(buf, conf_node, &images));
	ut_assertnull(images.dm_mod_create);
	ut_assertnull(images.dm_mod_waitfor);

	return 0;
}
FIT_VERITY_TEST(fit_verity_test_no_verity, 0);

/* Test: two dm-verity loadables produce combined cmdline */
static int fit_verity_test_two_loadables(struct unit_test_state *uts)
{
	char buf[FIT_BUF_SIZE];
	struct bootm_headers images;
	int conf_noffset;

	conf_noffset = build_verity_fit(buf, 2);
	ut_assert(conf_noffset >= 0);

	memset(&images, 0, sizeof(images));
	ut_assertok(fit_verity_build_cmdline(buf, conf_noffset, &images));

	/* Both targets should appear, separated by ";" */
	ut_assertnonnull(images.dm_mod_create);
	ut_assert(strstr(images.dm_mod_create, "rootfs0,,,"));
	ut_assert(strstr(images.dm_mod_create, ";rootfs1,,,"));
	ut_assert(strstr(images.dm_mod_create, "/dev/fit0"));
	ut_assert(strstr(images.dm_mod_create, "/dev/fit1"));

	/* dm_mod_waitfor should list both devices */
	ut_assertnonnull(images.dm_mod_waitfor);
	ut_assert(strstr(images.dm_mod_waitfor, "/dev/fit0"));
	ut_assert(strstr(images.dm_mod_waitfor, "/dev/fit1"));

	fit_verity_free(&images);
	return 0;
}
FIT_VERITY_TEST(fit_verity_test_two_loadables, 0);

/* Test: invalid block size (not power of two) returns -EINVAL */
static int fit_verity_test_bad_blocksize(struct unit_test_state *uts)
{
	char buf[FIT_BUF_SIZE];
	struct bootm_headers images;
	int images_node, conf_node, confs_node, img_node, verity_node;
	fdt32_t val;
	int ret;

	ret = fdt_create_empty_tree(buf, FIT_BUF_SIZE);
	ut_assertok(ret);

	images_node = fdt_add_subnode(buf, 0, "images");
	ut_assert(images_node >= 0);

	img_node = fdt_add_subnode(buf, images_node, "rootfs");
	ut_assert(img_node >= 0);
	ut_assertok(fdt_setprop_string(buf, img_node, FIT_TYPE_PROP,
				       "filesystem"));

	verity_node = fdt_add_subnode(buf, img_node, FIT_VERITY_NODENAME);
	ut_assert(verity_node >= 0);

	ut_assertok(fdt_setprop_string(buf, verity_node,
				       FIT_VERITY_ALGO_PROP, "sha256"));

	/* 3000 is not a power of two */
	val = cpu_to_fdt32(3000);
	ut_assertok(fdt_setprop(buf, verity_node, FIT_VERITY_DBS_PROP,
				&val, sizeof(val)));
	val = cpu_to_fdt32(4096);
	ut_assertok(fdt_setprop(buf, verity_node, FIT_VERITY_HBS_PROP,
				&val, sizeof(val)));

	val = cpu_to_fdt32(100);
	ut_assertok(fdt_setprop(buf, verity_node, FIT_VERITY_NBLK_PROP,
				&val, sizeof(val)));
	ut_assertok(fdt_setprop(buf, verity_node, FIT_VERITY_HBLK_PROP,
				&val, sizeof(val)));

	ut_assertok(fdt_setprop(buf, verity_node, FIT_VERITY_DIGEST_PROP,
				test_digest, sizeof(test_digest)));
	ut_assertok(fdt_setprop(buf, verity_node, FIT_VERITY_SALT_PROP,
				test_salt, sizeof(test_salt)));

	confs_node = fdt_add_subnode(buf, 0, "configurations");
	ut_assert(confs_node >= 0);
	conf_node = fdt_add_subnode(buf, confs_node, "conf-1");
	ut_assert(conf_node >= 0);
	ut_assertok(fdt_setprop_string(buf, conf_node, FIT_LOADABLE_PROP,
				       "rootfs"));

	memset(&images, 0, sizeof(images));
	ut_asserteq(-EINVAL, fit_verity_build_cmdline(buf, conf_node, &images));
	ut_assertnull(images.dm_mod_create);
	ut_assertnull(images.dm_mod_waitfor);

	return 0;
}
FIT_VERITY_TEST(fit_verity_test_bad_blocksize, 0);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for driver-model hash-provider selection
 *
 * Copyright (C) 2026 James Hilliard
 */

#include <dm.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <u-boot/hash.h>
#include <test/test.h>
#include <test/ut.h>

static int unsupported_calls;
static int success_calls;
static int hard_error_calls;

static int hash_test_unsupported(struct udevice *dev, enum HASH_ALGO algo,
				 const void *ibuf, const uint32_t ilen,
				 void *obuf, uint32_t chunk_sz)
{
	unsupported_calls++;

	return -EOPNOTSUPP;
}

static int hash_test_success(struct udevice *dev, enum HASH_ALGO algo,
			     const void *ibuf, const uint32_t ilen,
			     void *obuf, uint32_t chunk_sz)
{
	ssize_t digest_size;

	success_calls++;
	digest_size = hash_algo_digest_size(algo);
	if (digest_size < 0)
		return digest_size;

	memset(obuf, 0x5a, digest_size);

	return 0;
}

static int hash_test_hard_error(struct udevice *dev, enum HASH_ALGO algo,
				const void *ibuf, const uint32_t ilen,
				void *obuf, uint32_t chunk_sz)
{
	hard_error_calls++;

	return -EINVAL;
}

static const struct hash_ops hash_test_unsupported_ops = {
	.hash_digest_wd = hash_test_unsupported,
};

static const struct hash_ops hash_test_success_ops = {
	.hash_digest_wd = hash_test_success,
};

static const struct hash_ops hash_test_hard_error_ops = {
	.hash_digest_wd = hash_test_hard_error,
};

U_BOOT_DRIVER(hash_test_unsupported_drv) = {
	.name = "hash_test_unsupported",
	.id = UCLASS_HASH,
	.ops = &hash_test_unsupported_ops,
};

U_BOOT_DRIVER(hash_test_success_drv) = {
	.name = "hash_test_success",
	.id = UCLASS_HASH,
	.ops = &hash_test_success_ops,
};

U_BOOT_DRIVER(hash_test_hard_error_drv) = {
	.name = "hash_test_hard_error",
	.id = UCLASS_HASH,
	.ops = &hash_test_hard_error_ops,
};

static int hash_test_unbind_all(void)
{
	struct udevice *dev;
	int ret;

	for (;;) {
		ret = uclass_find_first_device(UCLASS_HASH, &dev);
		if (ret || !dev)
			return ret;
		if (device_active(dev)) {
			ret = device_remove(dev, DM_REMOVE_NORMAL);
			if (ret)
				return ret;
		}
		ret = device_unbind(dev);
		if (ret)
			return ret;
	}
}

static int hash_test_bind(const struct driver *drv, const char *name)
{
	struct udevice *dev;

	return device_bind(dm_root(), drv, name, 0, ofnode_null(), &dev);
}

static int dm_test_hash_provider_selection(struct unit_test_state *uts)
{
	u8 digest[32];
	int ret;

	ut_assertok(hash_test_unbind_all());
	ut_assertok(hash_test_bind(DM_DRIVER_GET(hash_test_unsupported_drv),
				   "hash-unsupported"));
	ut_assertok(hash_test_bind(DM_DRIVER_GET(hash_test_success_drv),
				   "hash-success"));

	unsupported_calls = 0;
	success_calls = 0;
	memset(digest, 0, sizeof(digest));
	ret = hash_digest_wd_lookup(HASH_ALGO_SHA256, "test", 4, digest, 4);
	ut_assertok(ret);
	ut_asserteq(1, unsupported_calls);
	ut_asserteq(1, success_calls);
	for (int i = 0; i < sizeof(digest); i++)
		ut_asserteq(0x5a, digest[i]);

	memset(digest, 0, sizeof(digest));
	ret = hash_digest_wd_lookup(HASH_ALGO_INVALID, "test", 4, digest, 4);
	ut_asserteq(-EINVAL, ret);
	ut_asserteq(2, unsupported_calls);
	ut_asserteq(2, success_calls);
	for (int i = 0; i < sizeof(digest); i++)
		ut_asserteq(0, digest[i]);

	ut_assertok(hash_test_unbind_all());
	ut_assertok(hash_test_bind(DM_DRIVER_GET(hash_test_hard_error_drv),
				   "hash-hard-error"));
	ut_assertok(hash_test_bind(DM_DRIVER_GET(hash_test_success_drv),
				   "hash-success"));

	hard_error_calls = 0;
	success_calls = 0;
	ret = hash_digest_wd_lookup(HASH_ALGO_SHA256, "test", 4, digest, 4);
	ut_asserteq(-EINVAL, ret);
	ut_asserteq(1, hard_error_calls);
	ut_asserteq(0, success_calls);

	return 0;
}

DM_TEST(dm_test_hash_provider_selection, UTF_SCAN_FDT);

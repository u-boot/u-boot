// SPDX-License-Identifier: GPL-2.0-or-later

#include <crypto/ecdsa-uclass.h>
#include <dm.h>
#include <dm/test.h>
#include <malloc.h>
#include <test/ut.h>
#include <u-boot/ecdsa.h>

#define FDT_MAX_SIZE		512

static int set_fdt_ecdsa_point(char *fdt, const char *name, const char *data)
{
	char *value = NULL;
	size_t len;
	int ret = 0;

	if (!fdt || !name || !data) {
		ret = -EINVAL;
		goto out;
	}

	len = strlen(data) / 2;
	if (!len) {
		ret = -EINVAL;
		goto out;
	}

	value = malloc(len);
	if (!value) {
		ret = -ENOMEM;
		goto out;
	}

	ret = hex2bin(value, data, len);
	if (ret)
		goto out;

	ret = fdt_property(fdt, name, value, len);
	if (ret)
		goto out;

out:
	free(value);
	return ret;
}

static int create_fdt_with_ecdsa_key(struct unit_test_state *uts,
				     char *fdt, size_t size,
				     const char *name, const char *curve,
				     const char *x, const char *y)
{
	ut_assertok(fdt_create(fdt, size));
	ut_assertok(fdt_finish_reservemap(fdt));
	ut_assertok(fdt_begin_node(fdt, ""));
	ut_assertok(fdt_begin_node(fdt, "signature"));
	ut_assertok(fdt_begin_node(fdt, name));
	ut_assertok(fdt_property_string(fdt, "algo", "sha256,ecdsa256"));
	ut_assertok(set_fdt_ecdsa_point(fdt, "ecdsa,y-point", y));
	ut_assertok(set_fdt_ecdsa_point(fdt, "ecdsa,x-point", x));
	ut_assertok(fdt_property_string(fdt, "ecdsa,curve", curve));
	ut_assertok(fdt_property_string(fdt, "key-name-hint", name));
	ut_assertok(fdt_end_node(fdt)); /* name */
	ut_assertok(fdt_end_node(fdt)); /* "signature" */
	ut_assertok(fdt_end_node(fdt)); /* "" */
	ut_assertok(fdt_finish(fdt));
	ut_assertok(fdt_pack(fdt));

	return 0;
}

/*
 * Basic test of the ECDSA uclass and ecdsa_verify()
 *
 * ECDSA software implementation is tested in another test,
 * so we only check that the UCLASS_ECDSA uclass may be used.
 *
 * The data used in this test come from RFC6979 and use the
 * sample with curve NIST P-256, hash sha256 and text "sample".
 */
static int dm_test_ecdsa_verify(struct unit_test_state *uts)
{
	struct uclass *ucp;
	const char *full_name = "sha256,ecdsa256";
	const char *name = "key-ecdsa-256";
	const char *curve = "prime256v1";
	const char *x = "60fed4ba255a9d31c961eb74c6356d68c049b8923b61fa6ce669622e60f29fb6";
	const char *y = "7903fe1008b8bc99a41ae9e95628bc64f2f1b20c2d7e9f5177a3c294d4462299";
	const char *r = "efd48b2aacb6a8fd1140dd9cd45e81d69d2c877b56aaf991c34d0ea84eaf3716";
	const char *s = "f7cb1c942d657c41d436c7a1b6e29f65f3e900dbb9aff4064dc4ab2f843acda8";
	u8 sig[64];
	char fdt[FDT_MAX_SIZE];

	struct image_region region[] = {
		{
			.data = "sample",
			.size = strlen("sample"),
		},
	};

	struct image_sign_info info = {
		.checksum = image_get_checksum_algo(full_name),
		.crypto = image_get_crypto_algo(full_name),
		.required_keynode = -1,
		.fdt_blob = fdt,
	};

	ut_assertnonnull(info.checksum);
	ut_assertnonnull(info.crypto);

	/* create a fdt with the public key */
	ut_assertok(create_fdt_with_ecdsa_key(uts, fdt, sizeof(fdt), name, curve, x, y));

	/* prepare the signature */
	ut_assertok(hex2bin(sig + 0, r, strlen(r) / 2));
	ut_assertok(hex2bin(sig + 32, s, strlen(s) / 2));

	ut_assertok(uclass_get(UCLASS_ECDSA, &ucp));
	ut_assertnonnull(ucp);
	ut_assertok(ecdsa_verify(&info, region, 1, sig, sizeof(sig)));

	return 0;
}

DM_TEST(dm_test_ecdsa_verify, UTF_SCAN_PDATA | UTF_SCAN_FDT);

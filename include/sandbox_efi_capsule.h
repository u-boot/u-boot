// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023, Linaro Limited
 */

#if !defined(_SANDBOX_EFI_CAPSULE_H_)
#define _SANDBOX_EFI_CAPSULE_H_

#define SANDBOX_UBOOT_IMAGE_GUID	"985f2937-7c2e-5e9a-8a5e-8e063312964b"
#define SANDBOX_UBOOT_ENV_IMAGE_GUID	"9e339473-c2eb-530a-a69b-0cd6bbbed40e"
#define SANDBOX_FIT_IMAGE_GUID		"46610520-469e-59dc-a8dd-c11832b877ea"
#define SANDBOX_INCORRECT_GUID		"058b7d83-50d5-4c47-a195-60d86ad341c4"

#define UBOOT_FIT_IMAGE			"u-boot_bin_env.itb"

#define CAPSULE_PRIV_KEY		"capsule_priv_key_good.key"
#define CAPSULE_PUB_KEY			"capsule_pub_key_good.crt"
#define CAPSULE_INVAL_KEY		"capsule_priv_key_bad.key"
#define CAPSULE_INVAL_PUB_KEY		"capsule_pub_key_bad.crt"

#endif /* _SANDBOX_EFI_CAPSULE_H_ */

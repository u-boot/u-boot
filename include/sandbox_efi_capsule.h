// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023, Linaro Limited
 */

#if !defined(_SANDBOX_EFI_CAPSULE_H_)
#define _SANDBOX_EFI_CAPSULE_H_

#define SANDBOX_UBOOT_IMAGE_GUID	"09d7cf52-0720-4710-91d1-08469b7fe9c8"
#define SANDBOX_UBOOT_ENV_IMAGE_GUID	"5a7021f5-fef2-48b4-aaba-832e777418c0"
#define SANDBOX_FIT_IMAGE_GUID		"3673b45d-6a7c-46f3-9e60-adabb03f7937"
#define SANDBOX_INCORRECT_GUID		"058b7d83-50d5-4c47-a195-60d86ad341c4"

#define UBOOT_BIN_IMAGE			"capsule_input_files/u-boot.bin.new"
#define UBOOT_ENV_IMAGE			"capsule_input_files/u-boot.env.new"
#define UBOOT_FIT_IMAGE			"capsule_input_files/u-boot_bin_env.itb"

#define CAPSULE_PRIV_KEY		"capsule_input_files/SIGNER.key"
#define CAPSULE_PUB_KEY			"capsule_input_files/SIGNER.crt"
#define CAPSULE_INVAL_KEY		"capsule_input_files/SIGNER2.key"
#define CAPSULE_INVAL_PUB_KEY		"capsule_input_files/SIGNER2.crt"

#endif /* _SANDBOX_EFI_CAPSULE_H_ */

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 Linaro Limited
 */

#include <common.h>
#include <efi_api.h>
#include <efi_loader.h>
#include <env.h>
#include <fdtdec.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int efi_get_public_key_data(void **pkey, efi_uintn_t *pkey_len)
{
	const void *fdt_blob = gd->fdt_blob;
	const void *blob;
	const char *cnode_name = "capsule-key";
	const char *snode_name = "signature";
	int sig_node;
	int len;

	sig_node = fdt_subnode_offset(fdt_blob, 0, snode_name);
	if (sig_node < 0) {
		EFI_PRINT("Unable to get signature node offset\n");
		return -FDT_ERR_NOTFOUND;
	}

	blob = fdt_getprop(fdt_blob, sig_node, cnode_name, &len);

	if (!blob || len < 0) {
		EFI_PRINT("Unable to get capsule-key value\n");
		*pkey = NULL;
		*pkey_len = 0;
		return -FDT_ERR_NOTFOUND;
	}

	*pkey = (void *)blob;
	*pkey_len = len;

	return 0;
}

bool efi_capsule_auth_enabled(void)
{
	return env_get("capsule_authentication_enabled") != NULL ?
		true : false;
}

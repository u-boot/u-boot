// SPDX-License-Identifier: GPL-2.0
/**
 * ufs-uclass.c - Universal Flash Subsystem (UFS) Uclass driver
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - http://www.ti.com
 */

#include <common.h>
#include "ufs.h"
#include <dm.h>

UCLASS_DRIVER(ufs) = {
	.id	= UCLASS_UFS,
	.name	= "ufs",
	.per_device_auto	= sizeof(struct ufs_hba),
};

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015-2017 Intel Corp.
 * (Written by Lance Zhao <lijian.zhao@intel.com> for Intel Corp.)
 * Copyright Google LLC 2019
 *
 * Modified from coreboot apollolake/include/soc/nvs.h
 */

#ifndef _GLOBAL_NVS_H_
#define _GLOBAL_NVS_H_

struct __packed acpi_global_nvs {
	/* Miscellaneous */
	u8	pcnt; /* 0x00 - Processor Count */
	u8	ppcm; /* 0x01 - Max PPC State */
	u8	lids; /* 0x02 - LID State */
	u8	pwrs; /* 0x03 - AC Power State */
	u8	dpte; /* 0x04 - Enable DPTF */
	u32	cbmc; /* 0x05 - 0x08 - U-Boot Console */
	u64	pm1i; /* 0x09 - 0x10 - System Wake Source - PM1 Index */
	u64	gpei; /* 0x11 - 0x18 - GPE Wake Source */
	u64	nhla; /* 0x19 - 0x20 - NHLT Address */
	u32	nhll; /* 0x21 - 0x24 - NHLT Length */
	u32	prt0; /* 0x25 - 0x28 - PERST_0 Address */
	u8	scdp; /* 0x29 - SD_CD GPIO portid */
	u8	scdo; /* 0x2a - GPIO pad offset relative to the community */
	u8	uior; /* 0x2b - UART debug controller init on S3 resume */
	u8	ecps; /* 0x2c - SGX Enabled status */
	u64	emna; /* 0x2d - 0x34 EPC base address */
	u64	elng; /* 0x35 - 0x3c EPC Length */
	u8	unused1[0x100 - 0x3d];		/* Pad out to 256 bytes */
	u8	unused2[0x1000 - 0x100];	/* Pad out to 4096 bytes */
};

#endif /* _GLOBAL_NVS_H_ */

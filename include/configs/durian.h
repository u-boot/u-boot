/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019
 * shuyiqi  <shuyiqi@phytium.com.cn>
 * liuhao   <liuhao@phytium.com.cn>
 */

#ifndef __DURIAN_CONFIG_H__
#define __DURIAN_CONFIG_H__

/* Sdram Bank #1 Address */
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_1_SIZE		0x7B000000
#define CONFIG_SYS_SDRAM_BASE   PHYS_SDRAM_1

#define CONFIG_SYS_INIT_SP_ADDR		(0x88000000 - 0x100000)

/* PCI CONFIG */
#define CONFIG_SYS_PCI_64BIT    1
#define CONFIG_PCI_SCAN_SHOW

/* SCSI */
#define CONFIG_SYS_SCSI_MAX_SCSI_ID 4
#define CONFIG_SYS_SCSI_MAX_LUN 1
#define CONFIG_SYS_SCSI_MAX_DEVICE 128
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SATA_MAX_DEVICE 4

/* BOOT */
#define CONFIG_SYS_BOOTM_LEN	(60 * 1024 * 1024)

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"load_kernel=ext4load scsi 0:1 0x90100000 uImage-2004\0"	\
	"load_fdt=ext4load scsi 0:1 0x95000000 ft2004-pci-64.dtb\0"\
	"boot_fdt=bootm 0x90100000 -:- 0x95000000\0"	\
	"distro_bootcmd=run load_kernel; run load_fdt; run boot_fdt"

#endif

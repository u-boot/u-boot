/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 */
#ifndef __CONFIG_SOCFPGA_CYCLONE5_H__
#define __CONFIG_SOCFPGA_CYCLONE5_H__

#include <asm/arch/base_addr_ac5.h>

/* Memory configurations */
#define PHYS_SDRAM_1_SIZE		0x40000000	/* 1GiB on SoCDK */

/* QSPI boot */
#define FDT_SIZE		__stringify(0x00010000)
#define KERNEL_SIZE		__stringify(0x005d0000)
#define QSPI_FDT_ADDR		__stringify(0x00220000)
#define QSPI_KERNEL_ADDR	__stringify(0x00230000)

#define SOCFPGA_BOOT_SETTINGS \
	"fdt_size=" FDT_SIZE "\0" \
	"kernel_size=" KERNEL_SIZE "\0" \
	"qspi_fdt_addr=" QSPI_FDT_ADDR "\0" \
	"qspi_kernel_addr=" QSPI_KERNEL_ADDR "\0" \
	"qspiboot=setenv bootargs earlycon " \
		"root=/dev/mtdblock1 rw rootfstype=jffs2; " \
		"bootz ${kernel_addr_r} - ${fdt_addr_r}\0" \
	"qspiload=sf probe; " \
		"sf read ${kernel_addr_r} ${qspi_kernel_addr} ${kernel_size}; " \
		"sf read ${fdt_addr_r} ${qspi_fdt_addr} ${fdt_size}\0"

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

#endif	/* __CONFIG_SOCFPGA_CYCLONE5_H__ */

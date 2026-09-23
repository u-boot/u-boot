/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Nodebox v3 CPU Module config header.
 */

#ifndef __NBXV3_H
#define __NBXV3_H

#include "lx2160a_common.h"

#define CFG_EXTRA_ENV_SETTINGS						\
	"pci_iommu_extra=pci@0x3600000,1.0.0,hp,"			\
		"pci@0x3700000,1.0.0,hp,"				\
		"pci@0x3800000,1.0.0,hp,"				\
		"pci@0x3900000,1.0.0,hp\0"				\
	"kernel_addr_r=0xc0000000\0"					\
	"fit_nor_offset=0x1000000\0"					\
	"fit_nor_size=0x1000000\0"					\
	"mcmemsize=0x70000000\0"					\
	"mcinitcmd="							\
		"sf probe 3:0 && "					\
		"sf read ${kernel_addr_r} ${fit_nor_offset} ${fit_nor_size} && "\
		"imxtract ${kernel_addr_r} mc 0xa8000000 && "		\
		"imxtract ${kernel_addr_r} dpc-${carrier} 0xa9000000 && "\
		"imxtract ${kernel_addr_r} dpl-${carrier} 0xaa000000 && "\
		"fsl_mc start mc 0xa8000000 0xa9000000 && "		\
		"fsl_mc lazyapply dpl 0xaa000000\0"			\
	"mc_init=bootm ${kernel_addr_r}#conf-${carrier}\0"		\
	"linux_boot_fit="						\
		"sf probe 3:0 && "					\
		"sf read ${kernel_addr_r} ${fit_nor_offset} ${fit_nor_size} && "\
		"bootm ${kernel_addr_r}#conf-${carrier}\0"		\
	"host_boot="							\
		"load semihosting - ${kernel_addr_r} fit.itb && "	\
		"bootm ${kernel_addr_r}#conf-${carrier}\0"		\
	"provision_openocd_fit="					\
		"load semihosting - ${kernel_addr_r} fit.itb && "	\
		"sf probe 3:0 && "					\
		"sf erase ${fit_nor_offset} ${fit_nor_size} && "	\
		"sf write ${kernel_addr_r} ${fit_nor_offset} ${filesize} && "\
		"reset\0"						\
	"xspi_bootcmd="							\
		"cp.b 0x21000000 ${kernel_addr_r} ${fit_nor_size} && "	\
		"bootm ${kernel_addr_r}#conf-${carrier}\0"		\
	"fsl_bootcmd_mcinitcmd_set=y\0"					\
	"bootcmd=run mc_init || run provision_openocd_fit\0"		\
	"mdio_list="							\
		"echo === Nbxv3 probed PHYs === && "			\
		"mdio list\0"						\
	"PS1=nbxv3> \0"							\
	"ps_refresh="							\
		"setenv PS1 \"nbxv3 ${ethact} ${ipaddr}> \"\0"		\
	"preboot=run mdio_list ; run ps_refresh\0"

#endif /* __NBXV3_H */

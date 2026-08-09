/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/sparrowhawk.h
 *     This file is Sparrow Hawk board configuration.
 *
 * Copyright (C) 2025 Marek Vasut <marek.vasut+renesas@mailbox.org>
 */

#ifndef __SPARROWHAWK_H
#define __SPARROWHAWK_H

#include "rcar-gen4-common.h"

/* Environment setting */
#undef CFG_EXTRA_ENV_SETTINGS
#define CFG_EXTRA_ENV_SETTINGS						\
	"bootm_size=0x10000000\0"					\
	\
	"renesas_rcar_gen4_pcie_firmware_sf_offset=0x300000\0"		\
	"renesas_rcar_gen4_pcie_firmware_sd_path=lib/firmware/rcar_gen4_pcie.bin\0" \
	"renesas_rcar_gen4_load_firmware="				\
		"env set renesas_rcar_gen4_load_firmware_addr 0x54000000 && " \
		"env set renesas_rcar_gen4_load_firmware_size 0x8000 && " \
		"sf probe && "						\
		"sf read ${renesas_rcar_gen4_load_firmware_addr} "	\
			"${renesas_rcar_gen4_pcie_firmware_sf_offset} "	\
			"${renesas_rcar_gen4_load_firmware_size}\0"	\
	"flash_pcie_fw_to_qspi_from_mmc="				\
		"load mmc 0:1 ${loadaddr} "				\
			"${renesas_rcar_gen4_pcie_firmware_sd_path} && " \
		"sf probe && "						\
		"sf update ${loadaddr} "				\
			"${renesas_rcar_gen4_pcie_firmware_sf_offset} "	\
			"${filesize}\0"

#endif /* __SPARROWHAWK_H */

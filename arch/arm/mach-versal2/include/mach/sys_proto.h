/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2021 - 2022, Xilinx, Inc.
 * Copyright (C) 2022 - 2026, Advanced Micro Devices, Inc.
 */

#ifndef _ASM_ARCH_SYS_PROTO_H
#define _ASM_ARCH_SYS_PROTO_H

#include <linux/build_bug.h>
#include <asm/armv8/mmu.h>

void mem_map_fill(struct mm_region *bank_info, u32 num_banks);
void fill_bd_mem_info(void);

/* Overridable PMC multiboot accessor: weak MMIO default, firmware override */
u32 versal2_pmc_multi_boot(void);
/* Direct MMIO read of the multiboot register (EL3 / no-firmware path) */
u32 versal2_multi_boot_reg(void);
/* Weak bootmode decode (MMIO default); a firmware/SCMI build may override */
u8 versal2_get_bootmode(void);
/* EL3 clock/timer register setup, called from board_early_init_r() */
void versal2_timer_setup(void);
/* Overridable chip ID accessor: weak MMIO default, firmware override */
int xilinx_pm_get_chipid(u32 *idcode, u32 *version);
/* Overridable PMC TAP register accessors: weak MMIO default, firmware override */
u32 zynqmp_pm_get_pmc_tap_idcode(void);
u32 zynqmp_pm_get_pmc_tap_version(void);
u32 zynqmp_pm_get_pmc_tap_usercode(void);
/* Overridable PMC GLOBAL PGGS register accessor: weak MMIO default, firmware override */
u32 zynqmp_pm_get_pmc_global_pggs_reg(u32 reg_addr);
/* Overridable OSPI AXI-mux select: weak MMIO default, firmware override */
int zynqmp_pm_ospi_enable_linear_mode(bool enable);
#if !CONFIG_IS_ENABLED(DM_GPIO)
/* Overridable OSPI flash GPIO reset sequence: weak MMIO default, firmware override */
int zynqmp_pm_ospi_flash_reset(void);
/* Direct GPIO reset sequence (EL3 / no-firmware path) */
int zynqmp_pm_ospi_flash_reset_raw(void);
#endif

int zynqmp_pm_wait_mphy_tx_rx_config_ready(u32 timeout_us);
int zynqmp_pm_wait_sram_init_done(u32 timeout_us);
int zynqmp_pm_set_sram_bypass(void);
int zynqmp_pm_get_ufs_calibration_values(u32 *value);

#endif /* _ASM_ARCH_SYS_PROTO_H */

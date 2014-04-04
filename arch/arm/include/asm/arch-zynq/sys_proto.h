/*
 * Copyright (c) 2013 Xilinx Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

extern void zynq_slcr_lock(void);
extern void zynq_slcr_unlock(void);
extern void zynq_slcr_cpu_reset(void);
extern void zynq_slcr_gem_clk_setup(u32 gem_id, unsigned long clk_rate);
extern void zynq_slcr_devcfg_disable(void);
extern void zynq_slcr_devcfg_enable(void);
extern u32 zynq_slcr_get_boot_mode(void);
extern u32 zynq_slcr_get_idcode(void);
extern void zynq_ddrc_init(void);

/* Driver extern functions */
extern int zynq_sdhci_init(u32 regbase);
extern int zynq_sdhci_of_init(const void *blob);

#endif /* _SYS_PROTO_H_ */

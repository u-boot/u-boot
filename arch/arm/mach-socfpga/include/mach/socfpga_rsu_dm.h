/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SoC FPGA RSU driver model (UCLASS_MISC) private data.
 */
#ifndef __SOCFPGA_RSU_DM_H__
#define __SOCFPGA_RSU_DM_H__

#include <asm/arch/rsu_ll.h>

/**
 * struct socfpga_rsu_priv - per-DM-device RSU session anchor
 * @ll: active low-level interface while a console session holds the device
 */
struct socfpga_rsu_priv {
	struct rsu_ll_intf *ll;
};

#endif /* __SOCFPGA_RSU_DM_H__ */

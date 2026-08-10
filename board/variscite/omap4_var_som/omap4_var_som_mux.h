/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef _VAR_SOM_OM44_MUX_DATA_H_
#define _VAR_SOM_OM44_MUX_DATA_H_

#include <asm/arch/mux_omap4.h>

const struct pad_conf_entry core_padconf_array_essential[] = {
{GPMC_AD0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},  /* sdmmc2_dat0 */
{GPMC_AD1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},  /* sdmmc2_dat1 */
{GPMC_AD2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},  /* sdmmc2_dat2 */
{GPMC_AD3, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},  /* sdmmc2_dat3 */
{GPMC_NCS2, (PTD | IEN | M3)},  /* gpio52 som rev */
{GPMC_NOE, (PTU | IEN | OFF_EN | OFF_OUT_PTD | M1)},  /* sdmmc2_clk */
{GPMC_NWE, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},  /* sdmmc2_cmd */
{I2C1_SCL, (PTU | IEN | M0)},  /* i2c1_scl */
{I2C1_SDA, (PTU | IEN | M0)},  /* i2c1_sda */
{UART3_RX_IRRX, (IEN | M0)},  /* uart3_rx */
{UART3_TX_IRTX, (M0)},  /* uart3_tx */
};

const struct pad_conf_entry wkup_padconf_array_essential[] = {
{PAD0_FREF_CLK3_OUT, (M0)}, /* fref_clk3_out */
{PAD0_FREF_SLICER_IN, (M0)},  /* fref_slicer_in */
{PAD1_FREF_CLK_IOREQ, (M0)},  /* fref_clk_ioreq */
{PAD0_FREF_CLK0_OUT, (M2)},  /* sys_drm_msecure */
{PAD1_SYS_32K, (IEN | M0)},  /* sys_32k */
{PAD0_SYS_NRESPWRON, (M0)},  /* sys_nrespwron */
{PAD1_SYS_NRESWARM, (M0)},  /* sys_nreswarm */
{PAD0_SYS_PWR_REQ, (PTU | M0)},  /* sys_pwr_req */
};

#endif /* _VAR_SOM_OM44_MUX_DATA_H_ */

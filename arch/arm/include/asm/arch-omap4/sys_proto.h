/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

#include <asm/arch/omap.h>
#include <asm/arch/clock.h>
#include <asm/io.h>
#include <asm/omap_common.h>
#include <linux/mtd/omap_gpmc.h>
#include <asm/arch/mux_omap4.h>
#include <asm/ti-common/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

extern const struct emif_regs emif_regs_elpida_200_mhz_2cs;
extern const struct emif_regs emif_regs_elpida_380_mhz_1cs;
extern const struct emif_regs emif_regs_elpida_400_mhz_1cs;
extern const struct emif_regs emif_regs_elpida_400_mhz_2cs;
extern const struct dmm_lisa_map_regs lisa_map_2G_x_1_x_2;
extern const struct dmm_lisa_map_regs lisa_map_2G_x_2_x_2;
extern const struct dmm_lisa_map_regs ma_lisa_map_2G_x_2_x_2;
struct omap_sysinfo {
	char *board_string;
};
extern const struct omap_sysinfo sysinfo;

void gpmc_init(void);
void watchdog_init(void);
u32 get_device_type(void);
void do_set_mux(u32 base, struct pad_conf_entry const *array, int size);
void set_muxconf_regs_essential(void);
u32 wait_on_value(u32, u32, void *, u32);
void sdelay(unsigned long);
void set_pl310_ctrl_reg(u32 val);
void setup_clocks_for_console(void);
void prcm_init(void);
void bypass_dpll(u32 const base);
void freq_update_core(void);
u32 get_sys_clk_freq(void);
u32 omap4_ddr_clk(void);
void cancel_out(u32 *num, u32 *den, u32 den_limit);
void sdram_init(void);
u32 omap_sdram_size(void);
u32 cortex_rev(void);
void save_omap_boot_params(void);
void init_omap_revision(void);
void do_io_settings(void);
void sri2c_init(void);
void gpi2c_init(void);
int omap_vc_bypass_send_value(u8 sa, u8 reg_addr, u8 reg_data);
u32 warm_reset(void);
void force_emif_self_refresh(void);
void setup_warmreset_time(void);
#endif

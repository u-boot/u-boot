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
#include <asm/arch/mux_omap4.h>

DECLARE_GLOBAL_DATA_PTR;

struct omap_sysinfo {
	char *board_string;
};
extern const struct omap_sysinfo sysinfo;

void gpmc_init(void);
void watchdog_init(void);
u32 get_device_type(void);
void do_set_mux(u32 base, struct pad_conf_entry const *array, int size);
void set_muxconf_regs_essential(void);
void set_muxconf_regs_non_essential(void);
void sr32(void *, u32, u32, u32);
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

static inline u32 running_from_sdram(void)
{
	u32 pc;
	asm volatile ("mov %0, pc" : "=r" (pc));
	return ((pc >= OMAP44XX_DRAM_ADDR_SPACE_START) &&
	    (pc < OMAP44XX_DRAM_ADDR_SPACE_END));
}

static inline u8 uboot_loaded_by_spl(void)
{
	/*
	 * u-boot can be running from sdram either because of configuration
	 * Header or by SPL. If because of CH, then the romcode sets the
	 * CHSETTINGS executed bit to true in the boot parameter structure that
	 * it passes to the bootloader.This parameter is stored in the ch_flags
	 * variable by both SPL and u-boot.Check out for CHSETTINGS, which is a
	 * mandatory section if CH is present.
	 */
	if ((gd->arch.omap_boot_params.ch_flags) & (CH_FLAGS_CHSETTINGS))
		return 0;
	else
		return running_from_sdram();
}
/*
 * The basic hardware init of OMAP(s_init()) can happen in 4
 * different contexts:
 *  1. SPL running from SRAM
 *  2. U-Boot running from FLASH
 *  3. Non-XIP U-Boot loaded to SDRAM by SPL
 *  4. Non-XIP U-Boot loaded to SDRAM by ROM code using the
 *     Configuration Header feature
 *
 * This function finds this context.
 * Defining as inline may help in compiling out unused functions in SPL
 */
static inline u32 omap_hw_init_context(void)
{
#ifdef CONFIG_SPL_BUILD
	return OMAP_INIT_CONTEXT_SPL;
#else
	if (uboot_loaded_by_spl())
		return OMAP_INIT_CONTEXT_UBOOT_AFTER_SPL;
	else if (running_from_sdram())
		return OMAP_INIT_CONTEXT_UBOOT_AFTER_CH;
	else
		return OMAP_INIT_CONTEXT_UBOOT_FROM_NOR;
#endif
}

#endif

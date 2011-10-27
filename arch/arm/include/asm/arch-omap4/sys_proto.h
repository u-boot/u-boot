/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

#include <asm/arch/omap4.h>
#include <asm/arch/clocks.h>
#include <asm/io.h>
#include <asm/omap_common.h>
#include <asm/arch/mux_omap4.h>

struct omap_sysinfo {
	char *board_string;
};
extern const struct omap_sysinfo sysinfo;

extern struct omap4_prcm_regs *const prcm;

void gpmc_init(void);
void watchdog_init(void);
u32 get_device_type(void);
void do_set_mux(u32 base, struct pad_conf_entry const *array, int size);
void set_muxconf_regs_non_essential(void);
void sr32(void *, u32, u32, u32);
u32 wait_on_value(u32, u32, void *, u32);
void sdelay(unsigned long);
void set_pl310_ctrl_reg(u32 val);
void setup_clocks_for_console(void);
void prcm_init(void);
void bypass_dpll(u32 *const base);
void freq_update_core(void);
u32 get_sys_clk_freq(void);
u32 omap4_ddr_clk(void);
void cancel_out(u32 *num, u32 *den, u32 den_limit);
void sdram_init(void);
u32 omap4_sdram_size(void);

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
	 * Configuration Header is not supported yet, so u-boot init running
	 * from SDRAM implies that it was loaded by SPL. When this situation
	 * changes one of these approaches could be taken:
	 * i.  Pass a magic from SPL to U-Boot and U-Boot save it at a known
	 *     location.
	 * ii. Check the OPP. CH can support only 50% OPP while SPL initializes
	 *     the DPLLs at 100% OPP.
	 */
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
static inline u32 omap4_hw_init_context(void)
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

static inline u32 omap_revision(void)
{
	extern u32 *const omap4_revision;
	return *omap4_revision;
}

#endif

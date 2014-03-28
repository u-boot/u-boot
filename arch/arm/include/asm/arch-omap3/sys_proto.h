/*
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_
#include <linux/mtd/omap_gpmc.h>
#include <asm/omap_common.h>

typedef struct {
	u32 mtype;
	char *board_string;
	char *nand_string;
} omap3_sysinfo;

struct emu_hal_params {
	u32 num_params;
	u32 param1;
};

/* Board SDRC timing values */
struct board_sdrc_timings {
	u32 mcfg;
	u32 ctrla;
	u32 ctrlb;
	u32 rfr_ctrl;
	u32 mr;
};

void prcm_init(void);
void per_clocks_enable(void);
void ehci_clocks_enable(void);

void memif_init(void);
void sdrc_init(void);
void do_sdrc_init(u32, u32);

void get_board_mem_timings(struct board_sdrc_timings *timings);
void identify_nand_chip(int *mfr, int *id);
void emif4_init(void);
void gpmc_init(void);
void enable_gpmc_cs_config(const u32 *gpmc_config, struct gpmc_cs *cs, u32 base,
			u32 size);

void watchdog_init(void);
void set_muxconf_regs(void);

u32 get_cpu_family(void);
u32 get_cpu_rev(void);
u32 get_sku_id(void);
u32 get_sysboot_value(void);
u32 is_gpmc_muxed(void);
u32 get_gpmc0_type(void);
u32 get_gpmc0_width(void);
u32 is_running_in_sdram(void);
u32 is_running_in_sram(void);
u32 is_running_in_flash(void);
u32 get_device_type(void);
void secureworld_exit(void);
void try_unlock_memory(void);
u32 get_boot_type(void);
void invalidate_dcache(u32);
u32 wait_on_value(u32, u32, void *, u32);
void sdelay(unsigned long);
void make_cs1_contiguous(void);
void omap_nand_switch_ecc(uint32_t, uint32_t);
void power_init_r(void);
void dieid_num_r(void);
void get_dieid(u32 *id);
void do_omap3_emu_romcode_call(u32 service_id, u32 parameters);
void omap3_gp_romcode_call(u32 service_id, u32 parameter);
u32 warm_reset(void);
#endif

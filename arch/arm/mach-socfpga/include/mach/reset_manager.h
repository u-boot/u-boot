/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	_RESET_MANAGER_H_
#define	_RESET_MANAGER_H_

void reset_cpu(ulong addr);
void reset_deassert_peripherals_handoff(void);

void socfpga_bridges_reset(int enable);

void socfpga_emac_reset(int enable);
void socfpga_watchdog_reset(void);
void socfpga_spim_enable(void);
void socfpga_uart0_enable(void);
void socfpga_sdram_enable(void);
void socfpga_osc1timer_enable(void);

struct socfpga_reset_manager {
	u32	status;
	u32	ctrl;
	u32	counts;
	u32	padding1;
	u32	mpu_mod_reset;
	u32	per_mod_reset;
	u32	per2_mod_reset;
	u32	brg_mod_reset;
};

#if defined(CONFIG_SOCFPGA_VIRTUAL_TARGET)
#define RSTMGR_CTRL_SWWARMRSTREQ_LSB 2
#else
#define RSTMGR_CTRL_SWWARMRSTREQ_LSB 1
#endif

#define RSTMGR_PERMODRST_EMAC0_LSB	0
#define RSTMGR_PERMODRST_EMAC1_LSB	1
#define RSTMGR_PERMODRST_L4WD0_LSB	6
#define RSTMGR_PERMODRST_OSC1TIMER0_LSB	8
#define RSTMGR_PERMODRST_UART0_LSB	16
#define RSTMGR_PERMODRST_SPIM0_LSB	18
#define RSTMGR_PERMODRST_SPIM1_LSB	19
#define RSTMGR_PERMODRST_SDR_LSB	29

#endif /* _RESET_MANAGER_H_ */

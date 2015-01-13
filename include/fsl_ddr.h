/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#ifndef FSL_DDR_MAIN_H
#define FSL_DDR_MAIN_H

#include <fsl_ddrc_version.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>

#include <common_timing_params.h>

#ifndef CONFIG_SYS_FSL_DDR_MAIN_NUM_CTRLS
/* All controllers are for main memory */
#define CONFIG_SYS_FSL_DDR_MAIN_NUM_CTRLS	CONFIG_NUM_DDR_CONTROLLERS
#endif

#ifdef CONFIG_SYS_FSL_DDR_LE
#define ddr_in32(a)	in_le32(a)
#define ddr_out32(a, v)	out_le32(a, v)
#else
#define ddr_in32(a)	in_be32(a)
#define ddr_out32(a, v)	out_be32(a, v)
#endif

#define _DDR_ADDR CONFIG_SYS_FSL_DDR_ADDR

u32 fsl_ddr_get_version(void);

#if defined(CONFIG_DDR_SPD) || defined(CONFIG_SPD_EEPROM)
/*
 * Bind the main DDR setup driver's generic names
 * to this specific DDR technology.
 */
static __inline__ int
compute_dimm_parameters(const generic_spd_eeprom_t *spd,
			dimm_params_t *pdimm,
			unsigned int dimm_number)
{
	return ddr_compute_dimm_parameters(spd, pdimm, dimm_number);
}
#endif

/*
 * Data Structures
 *
 * All data structures have to be on the stack
 */
#define CONFIG_SYS_NUM_DDR_CTLRS CONFIG_NUM_DDR_CONTROLLERS
#define CONFIG_SYS_DIMM_SLOTS_PER_CTLR CONFIG_DIMM_SLOTS_PER_CTLR

typedef struct {
	generic_spd_eeprom_t
	   spd_installed_dimms[CONFIG_SYS_NUM_DDR_CTLRS][CONFIG_SYS_DIMM_SLOTS_PER_CTLR];
	struct dimm_params_s
	   dimm_params[CONFIG_SYS_NUM_DDR_CTLRS][CONFIG_SYS_DIMM_SLOTS_PER_CTLR];
	memctl_options_t memctl_opts[CONFIG_SYS_NUM_DDR_CTLRS];
	common_timing_params_t common_timing_params[CONFIG_SYS_NUM_DDR_CTLRS];
	fsl_ddr_cfg_regs_t fsl_ddr_config_reg[CONFIG_SYS_NUM_DDR_CTLRS];
	unsigned int first_ctrl;
	unsigned int num_ctrls;
	unsigned long long mem_base;
	unsigned int dimm_slots_per_ctrl;
	int (*board_need_mem_reset)(void);
	void (*board_mem_reset)(void);
	void (*board_mem_de_reset)(void);
} fsl_ddr_info_t;

/* Compute steps */
#define STEP_GET_SPD                 (1 << 0)
#define STEP_COMPUTE_DIMM_PARMS      (1 << 1)
#define STEP_COMPUTE_COMMON_PARMS    (1 << 2)
#define STEP_GATHER_OPTS             (1 << 3)
#define STEP_ASSIGN_ADDRESSES        (1 << 4)
#define STEP_COMPUTE_REGS            (1 << 5)
#define STEP_PROGRAM_REGS            (1 << 6)
#define STEP_ALL                     0xFFF

unsigned long long
fsl_ddr_compute(fsl_ddr_info_t *pinfo, unsigned int start_step,
				       unsigned int size_only);
const char *step_to_string(unsigned int step);

unsigned int compute_fsl_memctl_config_regs(const memctl_options_t *popts,
			       fsl_ddr_cfg_regs_t *ddr,
			       const common_timing_params_t *common_dimm,
			       const dimm_params_t *dimm_parameters,
			       unsigned int dbw_capacity_adjust,
			       unsigned int size_only);
unsigned int compute_lowest_common_dimm_parameters(
				const dimm_params_t *dimm_params,
				common_timing_params_t *outpdimm,
				unsigned int number_of_dimms);
unsigned int populate_memctl_options(int all_dimms_registered,
				memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num);
void check_interleaving_options(fsl_ddr_info_t *pinfo);

unsigned int mclk_to_picos(unsigned int mclk);
unsigned int get_memory_clk_period_ps(void);
unsigned int picos_to_mclk(unsigned int picos);
void fsl_ddr_set_lawbar(
		const common_timing_params_t *memctl_common_params,
		unsigned int memctl_interleaved,
		unsigned int ctrl_num);

int fsl_ddr_interactive_env_var_exists(void);
unsigned long long fsl_ddr_interactive(fsl_ddr_info_t *pinfo, int var_is_set);
void fsl_ddr_get_spd(generic_spd_eeprom_t *ctrl_dimms_spd,
		     unsigned int ctrl_num, unsigned int dimm_slots_per_ctrl);

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
unsigned int check_fsl_memctl_config_regs(const fsl_ddr_cfg_regs_t *ddr);
void board_add_ram_info(int use_default);

/* processor specific function */
void fsl_ddr_set_memctl_regs(const fsl_ddr_cfg_regs_t *regs,
				   unsigned int ctrl_num, int step);

/* board specific function */
int fsl_ddr_get_dimm_params(dimm_params_t *pdimm,
			unsigned int controller_number,
			unsigned int dimm_number);
#endif

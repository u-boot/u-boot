/*
 * Copyright 2008-2011 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

/*
 * Generic driver for Freescale DDR/DDR2/DDR3 memory controller.
 * Based on code from spd_sdram.c
 * Author: James Yang [at freescale.com]
 */

#include <common.h>
#include <i2c.h>
#include <asm/fsl_ddr_sdram.h>

#include "ddr.h"

extern void fsl_ddr_set_lawbar(
		const common_timing_params_t *memctl_common_params,
		unsigned int memctl_interleaved,
		unsigned int ctrl_num);

/* processor specific function */
extern void fsl_ddr_set_memctl_regs(const fsl_ddr_cfg_regs_t *regs,
				   unsigned int ctrl_num);

#if defined(SPD_EEPROM_ADDRESS) || \
    defined(SPD_EEPROM_ADDRESS1) || defined(SPD_EEPROM_ADDRESS2) || \
    defined(SPD_EEPROM_ADDRESS3) || defined(SPD_EEPROM_ADDRESS4)
#if (CONFIG_NUM_DDR_CONTROLLERS == 1) && (CONFIG_DIMM_SLOTS_PER_CTLR == 1)
u8 spd_i2c_addr[CONFIG_NUM_DDR_CONTROLLERS][CONFIG_DIMM_SLOTS_PER_CTLR] = {
	[0][0] = SPD_EEPROM_ADDRESS,
};
#elif (CONFIG_NUM_DDR_CONTROLLERS == 1) && (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
u8 spd_i2c_addr[CONFIG_NUM_DDR_CONTROLLERS][CONFIG_DIMM_SLOTS_PER_CTLR] = {
	[0][0] = SPD_EEPROM_ADDRESS1,	/* controller 1 */
	[0][1] = SPD_EEPROM_ADDRESS2,	/* controller 1 */
};
#elif (CONFIG_NUM_DDR_CONTROLLERS == 2) && (CONFIG_DIMM_SLOTS_PER_CTLR == 1)
u8 spd_i2c_addr[CONFIG_NUM_DDR_CONTROLLERS][CONFIG_DIMM_SLOTS_PER_CTLR] = {
	[0][0] = SPD_EEPROM_ADDRESS1,	/* controller 1 */
	[1][0] = SPD_EEPROM_ADDRESS2,	/* controller 2 */
};
#elif (CONFIG_NUM_DDR_CONTROLLERS == 2) && (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
u8 spd_i2c_addr[CONFIG_NUM_DDR_CONTROLLERS][CONFIG_DIMM_SLOTS_PER_CTLR] = {
	[0][0] = SPD_EEPROM_ADDRESS1,	/* controller 1 */
	[0][1] = SPD_EEPROM_ADDRESS2,	/* controller 1 */
	[1][0] = SPD_EEPROM_ADDRESS3,	/* controller 2 */
	[1][1] = SPD_EEPROM_ADDRESS4,	/* controller 2 */
};
#endif

static void __get_spd(generic_spd_eeprom_t *spd, u8 i2c_address)
{
	int ret = i2c_read(i2c_address, 0, 1, (uchar *)spd,
				sizeof(generic_spd_eeprom_t));

	if (ret) {
		printf("DDR: failed to read SPD from address %u\n", i2c_address);
		memset(spd, 0, sizeof(generic_spd_eeprom_t));
	}
}

__attribute__((weak, alias("__get_spd")))
void get_spd(generic_spd_eeprom_t *spd, u8 i2c_address);

void fsl_ddr_get_spd(generic_spd_eeprom_t *ctrl_dimms_spd,
		      unsigned int ctrl_num)
{
	unsigned int i;
	unsigned int i2c_address = 0;

	if (ctrl_num >= CONFIG_NUM_DDR_CONTROLLERS) {
		printf("%s unexpected ctrl_num = %u\n", __FUNCTION__, ctrl_num);
		return;
	}

	for (i = 0; i < CONFIG_DIMM_SLOTS_PER_CTLR; i++) {
		i2c_address = spd_i2c_addr[ctrl_num][i];
		get_spd(&(ctrl_dimms_spd[i]), i2c_address);
	}
}
#else
void fsl_ddr_get_spd(generic_spd_eeprom_t *ctrl_dimms_spd,
		      unsigned int ctrl_num)
{
}
#endif /* SPD_EEPROM_ADDRESSx */

/*
 * ASSUMPTIONS:
 *    - Same number of CONFIG_DIMM_SLOTS_PER_CTLR on each controller
 *    - Same memory data bus width on all controllers
 *
 * NOTES:
 *
 * The memory controller and associated documentation use confusing
 * terminology when referring to the orgranization of DRAM.
 *
 * Here is a terminology translation table:
 *
 * memory controller/documention  |industry   |this code  |signals
 * -------------------------------|-----------|-----------|-----------------
 * physical bank/bank		  |rank       |rank	  |chip select (CS)
 * logical bank/sub-bank	  |bank       |bank	  |bank address (BA)
 * page/row			  |row	      |page	  |row address
 * ???				  |column     |column	  |column address
 *
 * The naming confusion is further exacerbated by the descriptions of the
 * memory controller interleaving feature, where accesses are interleaved
 * _BETWEEN_ two seperate memory controllers.  This is configured only in
 * CS0_CONFIG[INTLV_CTL] of each memory controller.
 *
 * memory controller documentation | number of chip selects
 *				   | per memory controller supported
 * --------------------------------|-----------------------------------------
 * cache line interleaving	   | 1 (CS0 only)
 * page interleaving		   | 1 (CS0 only)
 * bank interleaving		   | 1 (CS0 only)
 * superbank interleraving	   | depends on bank (chip select)
 *				   |   interleraving [rank interleaving]
 *				   |   mode used on every memory controller
 *
 * Even further confusing is the existence of the interleaving feature
 * _WITHIN_ each memory controller.  The feature is referred to in
 * documentation as chip select interleaving or bank interleaving,
 * although it is configured in the DDR_SDRAM_CFG field.
 *
 * Name of field		| documentation name	| this code
 * -----------------------------|-----------------------|------------------
 * DDR_SDRAM_CFG[BA_INTLV_CTL]	| Bank (chip select)	| rank interleaving
 *				|  interleaving
 */

const char *step_string_tbl[] = {
	"STEP_GET_SPD",
	"STEP_COMPUTE_DIMM_PARMS",
	"STEP_COMPUTE_COMMON_PARMS",
	"STEP_GATHER_OPTS",
	"STEP_ASSIGN_ADDRESSES",
	"STEP_COMPUTE_REGS",
	"STEP_PROGRAM_REGS",
	"STEP_ALL"
};

const char * step_to_string(unsigned int step) {

	unsigned int s = __ilog2(step);

	if ((1 << s) != step)
		return step_string_tbl[7];

	return step_string_tbl[s];
}

int step_assign_addresses(fsl_ddr_info_t *pinfo,
			  unsigned int dbw_cap_adj[],
			  unsigned int *all_memctl_interleaving,
			  unsigned int *all_ctlr_rank_interleaving)
{
	int i, j;

	/*
	 * If a reduced data width is requested, but the SPD
	 * specifies a physically wider device, adjust the
	 * computed dimm capacities accordingly before
	 * assigning addresses.
	 */
	for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
		unsigned int found = 0;

		switch (pinfo->memctl_opts[i].data_bus_width) {
		case 2:
			/* 16-bit */
			for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
				unsigned int dw;
				if (!pinfo->dimm_params[i][j].n_ranks)
					continue;
				dw = pinfo->dimm_params[i][j].primary_sdram_width;
				if ((dw == 72 || dw == 64)) {
					dbw_cap_adj[i] = 2;
					break;
				} else if ((dw == 40 || dw == 32)) {
					dbw_cap_adj[i] = 1;
					break;
				}
			}
			break;

		case 1:
			/* 32-bit */
			for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
				unsigned int dw;
				dw = pinfo->dimm_params[i][j].data_width;
				if (pinfo->dimm_params[i][j].n_ranks
				    && (dw == 72 || dw == 64)) {
					/*
					 * FIXME: can't really do it
					 * like this because this just
					 * further reduces the memory
					 */
					found = 1;
					break;
				}
			}
			if (found) {
				dbw_cap_adj[i] = 1;
			}
			break;

		case 0:
			/* 64-bit */
			break;

		default:
			printf("unexpected data bus width "
				"specified controller %u\n", i);
			return 1;
		}
	}

	j = 0;
	for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++)
		if (pinfo->memctl_opts[i].memctl_interleaving)
			j++;
	/*
	 * Not support less than all memory controllers interleaving
	 * if more than two controllers
	 */
	if (j == CONFIG_NUM_DDR_CONTROLLERS)
		*all_memctl_interleaving = 1;

	/* Check that all controllers are rank interleaving. */
	j = 0;
	for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++)
		if (pinfo->memctl_opts[i].ba_intlv_ctl)
			j++;
	/*
	 * All memory controllers must be populated to qualify for
	 * all controller rank interleaving
	 */
	 if (j == CONFIG_NUM_DDR_CONTROLLERS)
		*all_ctlr_rank_interleaving = 1;

	if (*all_memctl_interleaving) {
		unsigned long long addr, total_mem_per_ctlr = 0;
		/*
		 * If interleaving between memory controllers,
		 * make each controller start at a base address
		 * of 0.
		 *
		 * Also, if bank interleaving (chip select
		 * interleaving) is enabled on each memory
		 * controller, CS0 needs to be programmed to
		 * cover the entire memory range on that memory
		 * controller
		 *
		 * Bank interleaving also implies that each
		 * addressed chip select is identical in size.
		 */

		for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
			addr = 0;
			pinfo->common_timing_params[i].base_address = 0ull;
			for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
				unsigned long long cap
					= pinfo->dimm_params[i][j].capacity;

				pinfo->dimm_params[i][j].base_address = addr;
				addr += cap >> dbw_cap_adj[i];
				total_mem_per_ctlr += cap >> dbw_cap_adj[i];
			}
		}
		pinfo->common_timing_params[0].total_mem = total_mem_per_ctlr;
	} else {
		/*
		 * Simple linear assignment if memory
		 * controllers are not interleaved.
		 */
		unsigned long long cur_memsize = 0;
		for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
			u64 total_mem_per_ctlr = 0;
			pinfo->common_timing_params[i].base_address =
						cur_memsize;
			for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
				/* Compute DIMM base addresses. */
				unsigned long long cap =
					pinfo->dimm_params[i][j].capacity;
				pinfo->dimm_params[i][j].base_address =
					cur_memsize;
				cur_memsize += cap >> dbw_cap_adj[i];
				total_mem_per_ctlr += cap >> dbw_cap_adj[i];
			}
			pinfo->common_timing_params[i].total_mem =
							total_mem_per_ctlr;
		}
	}

	return 0;
}

unsigned long long
fsl_ddr_compute(fsl_ddr_info_t *pinfo, unsigned int start_step,
				       unsigned int size_only)
{
	unsigned int i, j;
	unsigned int all_controllers_memctl_interleaving = 0;
	unsigned int all_controllers_rank_interleaving = 0;
	unsigned long long total_mem = 0;

	fsl_ddr_cfg_regs_t *ddr_reg = pinfo->fsl_ddr_config_reg;
	common_timing_params_t *timing_params = pinfo->common_timing_params;

	/* data bus width capacity adjust shift amount */
	unsigned int dbw_capacity_adjust[CONFIG_NUM_DDR_CONTROLLERS];

	for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
		dbw_capacity_adjust[i] = 0;
	}

	debug("starting at step %u (%s)\n",
	      start_step, step_to_string(start_step));

	switch (start_step) {
	case STEP_GET_SPD:
#if defined(CONFIG_DDR_SPD) || defined(CONFIG_SPD_EEPROM)
		/* STEP 1:  Gather all DIMM SPD data */
		for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
			fsl_ddr_get_spd(pinfo->spd_installed_dimms[i], i);
		}

	case STEP_COMPUTE_DIMM_PARMS:
		/* STEP 2:  Compute DIMM parameters from SPD data */

		for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
			for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
				unsigned int retval;
				generic_spd_eeprom_t *spd =
					&(pinfo->spd_installed_dimms[i][j]);
				dimm_params_t *pdimm =
					&(pinfo->dimm_params[i][j]);

				retval = compute_dimm_parameters(spd, pdimm, i);
#ifdef CONFIG_SYS_DDR_RAW_TIMING
				if (retval != 0) {
					printf("SPD error! Trying fallback to "
					"raw timing calculation\n");
					fsl_ddr_get_dimm_params(pdimm, i, j);
				}
#else
				if (retval == 2) {
					printf("Error: compute_dimm_parameters"
					" non-zero returned FATAL value "
					"for memctl=%u dimm=%u\n", i, j);
					return 0;
				}
#endif
				if (retval) {
					debug("Warning: compute_dimm_parameters"
					" non-zero return value for memctl=%u "
					"dimm=%u\n", i, j);
				}
			}
		}

#else
	case STEP_COMPUTE_DIMM_PARMS:
		for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
			for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
				dimm_params_t *pdimm =
					&(pinfo->dimm_params[i][j]);
				fsl_ddr_get_dimm_params(pdimm, i, j);
			}
		}
		debug("Filling dimm parameters from board specific file\n");
#endif
	case STEP_COMPUTE_COMMON_PARMS:
		/*
		 * STEP 3: Compute a common set of timing parameters
		 * suitable for all of the DIMMs on each memory controller
		 */
		for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
			debug("Computing lowest common DIMM"
				" parameters for memctl=%u\n", i);
			compute_lowest_common_dimm_parameters(
				pinfo->dimm_params[i],
				&timing_params[i],
				CONFIG_DIMM_SLOTS_PER_CTLR);
		}

	case STEP_GATHER_OPTS:
		/* STEP 4:  Gather configuration requirements from user */
		for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
			debug("Reloading memory controller "
				"configuration options for memctl=%u\n", i);
			/*
			 * This "reloads" the memory controller options
			 * to defaults.  If the user "edits" an option,
			 * next_step points to the step after this,
			 * which is currently STEP_ASSIGN_ADDRESSES.
			 */
			populate_memctl_options(
					timing_params[i].all_DIMMs_registered,
					&pinfo->memctl_opts[i],
					pinfo->dimm_params[i], i);
		}
		check_interleaving_options(pinfo);
	case STEP_ASSIGN_ADDRESSES:
		/* STEP 5:  Assign addresses to chip selects */
		step_assign_addresses(pinfo,
				dbw_capacity_adjust,
				&all_controllers_memctl_interleaving,
				&all_controllers_rank_interleaving);

	case STEP_COMPUTE_REGS:
		/* STEP 6:  compute controller register values */
		debug("FSL Memory ctrl cg register computation\n");
		for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
			if (timing_params[i].ndimms_present == 0) {
				memset(&ddr_reg[i], 0,
					sizeof(fsl_ddr_cfg_regs_t));
				continue;
			}

			compute_fsl_memctl_config_regs(
					&pinfo->memctl_opts[i],
					&ddr_reg[i], &timing_params[i],
					pinfo->dimm_params[i],
					dbw_capacity_adjust[i],
					size_only);
		}

	default:
		break;
	}

	/* Compute the total amount of memory. */

	/*
	 * If bank interleaving but NOT memory controller interleaving
	 * CS_BNDS describe the quantity of memory on each memory
	 * controller, so the total is the sum across.
	 */
	if (!all_controllers_memctl_interleaving
	    && all_controllers_rank_interleaving) {
		total_mem = 0;
		for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
			total_mem += timing_params[i].total_mem;
		}

	} else {
		/*
		 * Compute the amount of memory available just by
		 * looking for the highest valid CSn_BNDS value.
		 * This allows us to also experiment with using
		 * only CS0 when using dual-rank DIMMs.
		 */
		unsigned int max_end = 0;

		for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
			for (j = 0; j < CONFIG_CHIP_SELECTS_PER_CTRL; j++) {
				fsl_ddr_cfg_regs_t *reg = &ddr_reg[i];
				if (reg->cs[j].config & 0x80000000) {
					unsigned int end;
					end = reg->cs[j].bnds & 0xFFF;
					if (end > max_end) {
						max_end = end;
					}
				}
			}
		}

		total_mem = 1 + (((unsigned long long)max_end << 24ULL)
				    | 0xFFFFFFULL);
	}

	return total_mem;
}

/*
 * fsl_ddr_sdram() -- this is the main function to be called by
 *	initdram() in the board file.
 *
 * It returns amount of memory configured in bytes.
 */
phys_size_t fsl_ddr_sdram(void)
{
	unsigned int i;
	unsigned int memctl_interleaved;
	unsigned long long total_memory;
	fsl_ddr_info_t info;

	/* Reset info structure. */
	memset(&info, 0, sizeof(fsl_ddr_info_t));

	/* Compute it once normally. */
#ifdef CONFIG_FSL_DDR_INTERACTIVE
	if (getenv("ddr_interactive"))
		total_memory = fsl_ddr_interactive(&info);
	else
#endif
		total_memory = fsl_ddr_compute(&info, STEP_GET_SPD, 0);

	/* Check for memory controller interleaving. */
	memctl_interleaved = 0;
	for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
		memctl_interleaved +=
			info.memctl_opts[i].memctl_interleaving;
	}

	if (memctl_interleaved) {
		if (memctl_interleaved == CONFIG_NUM_DDR_CONTROLLERS) {
			debug("memctl interleaving\n");
			/*
			 * Change the meaning of memctl_interleaved
			 * to be "boolean".
			 */
			memctl_interleaved = 1;
		} else {
			printf("Warning: memctl interleaving not "
				"properly configured on all controllers\n");
			memctl_interleaved = 0;
			for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++)
				info.memctl_opts[i].memctl_interleaving = 0;
			debug("Recomputing with memctl_interleaving off.\n");
			total_memory = fsl_ddr_compute(&info,
						       STEP_ASSIGN_ADDRESSES,
						       0);
		}
	}

	/* Program configuration registers. */
	for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
		debug("Programming controller %u\n", i);
		if (info.common_timing_params[i].ndimms_present == 0) {
			debug("No dimms present on controller %u; "
					"skipping programming\n", i);
			continue;
		}

		fsl_ddr_set_memctl_regs(&(info.fsl_ddr_config_reg[i]), i);
	}

	if (memctl_interleaved) {
		const unsigned int ctrl_num = 0;

		/* Only set LAWBAR1 if memory controller interleaving is on. */
		fsl_ddr_set_lawbar(&info.common_timing_params[0],
					 memctl_interleaved, ctrl_num);
	} else {
		/*
		 * Memory controller interleaving is NOT on;
		 * set each lawbar individually.
		 */
		for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
			fsl_ddr_set_lawbar(&info.common_timing_params[i],
						 0, i);
		}
	}

	debug("total_memory = %llu\n", total_memory);

#if !defined(CONFIG_PHYS_64BIT)
	/* Check for 4G or more.  Bad. */
	if (total_memory >= (1ull << 32)) {
		printf("Detected %lld MB of memory\n", total_memory >> 20);
		printf("       This U-Boot only supports < 4G of DDR\n");
		printf("       You could rebuild it with CONFIG_PHYS_64BIT\n");
		printf("       "); /* re-align to match init_func_ram print */
		total_memory = CONFIG_MAX_MEM_MAPPED;
	}
#endif

	return total_memory;
}

/*
 * fsl_ddr_sdram_size() - This function only returns the size of the total
 * memory without setting ddr control registers.
 */
phys_size_t
fsl_ddr_sdram_size(void)
{
	fsl_ddr_info_t  info;
	unsigned long long total_memory = 0;

	memset(&info, 0 , sizeof(fsl_ddr_info_t));

	/* Compute it once normally. */
	total_memory = fsl_ddr_compute(&info, STEP_GET_SPD, 1);

	return total_memory;
}

/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <i2c.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "ddr3_init.h"

#include "../../../../arch/arm/mach-mvebu/serdes/a38x/sys_env_lib.h"

static struct dlb_config ddr3_dlb_config_table[] = {
	{REG_STATIC_DRAM_DLB_CONTROL, 0x2000005c},
	{DLB_BUS_OPTIMIZATION_WEIGHTS_REG, 0x00880000},
	{DLB_AGING_REGISTER, 0x0f7f007f},
	{DLB_EVICTION_CONTROL_REG, 0x0000129f},
	{DLB_EVICTION_TIMERS_REGISTER_REG, 0x00ff0000},
	{DLB_BUS_WEIGHTS_DIFF_CS, 0x04030802},
	{DLB_BUS_WEIGHTS_DIFF_BG, 0x00000a02},
	{DLB_BUS_WEIGHTS_SAME_BG, 0x09000a01},
	{DLB_BUS_WEIGHTS_RD_WR, 0x00020005},
	{DLB_BUS_WEIGHTS_ATTR_SYS_PRIO, 0x00060f10},
	{DLB_MAIN_QUEUE_MAP, 0x00000543},
	{DLB_LINE_SPLIT, 0x00000000},
	{DLB_USER_COMMAND_REG, 0x00000000},
	{0x0, 0x0}
};

static struct dlb_config ddr3_dlb_config_table_a0[] = {
	{REG_STATIC_DRAM_DLB_CONTROL, 0x2000005c},
	{DLB_BUS_OPTIMIZATION_WEIGHTS_REG, 0x00880000},
	{DLB_AGING_REGISTER, 0x0f7f007f},
	{DLB_EVICTION_CONTROL_REG, 0x0000129f},
	{DLB_EVICTION_TIMERS_REGISTER_REG, 0x00ff0000},
	{DLB_BUS_WEIGHTS_DIFF_CS, 0x04030802},
	{DLB_BUS_WEIGHTS_DIFF_BG, 0x00000a02},
	{DLB_BUS_WEIGHTS_SAME_BG, 0x09000a01},
	{DLB_BUS_WEIGHTS_RD_WR, 0x00020005},
	{DLB_BUS_WEIGHTS_ATTR_SYS_PRIO, 0x00060f10},
	{DLB_MAIN_QUEUE_MAP, 0x00000543},
	{DLB_LINE_SPLIT, 0x00000000},
	{DLB_USER_COMMAND_REG, 0x00000000},
	{0x0, 0x0}
};

#if defined(CONFIG_ARMADA_38X)
struct dram_modes {
	char *mode_name;
	u8 cpu_freq;
	u8 fab_freq;
	u8 chip_id;
	u8 chip_board_rev;
	struct reg_data *regs;
};

struct dram_modes ddr_modes[] = {
#ifdef SUPPORT_STATIC_DUNIT_CONFIG
	/* Conf name, CPUFreq, Fab_freq, Chip ID, Chip/Board, MC regs*/
#ifdef CONFIG_CUSTOMER_BOARD_SUPPORT
	{"a38x_customer_0_800", DDR_FREQ_800, 0, 0x0, A38X_CUSTOMER_BOARD_ID0,
	 ddr3_customer_800},
	{"a38x_customer_1_800", DDR_FREQ_800, 0, 0x0, A38X_CUSTOMER_BOARD_ID1,
	 ddr3_customer_800},
#else
	{"a38x_533", DDR_FREQ_533, 0, 0x0, MARVELL_BOARD, ddr3_a38x_533},
	{"a38x_667", DDR_FREQ_667, 0, 0x0, MARVELL_BOARD, ddr3_a38x_667},
	{"a38x_800", DDR_FREQ_800, 0, 0x0, MARVELL_BOARD, ddr3_a38x_800},
	{"a38x_933", DDR_FREQ_933, 0, 0x0, MARVELL_BOARD, ddr3_a38x_933},
#endif
#endif
};
#endif /* defined(CONFIG_ARMADA_38X) */

/* Translates topology map definitions to real memory size in bits */
u32 mem_size[] = {
	ADDR_SIZE_512MB, ADDR_SIZE_1GB, ADDR_SIZE_2GB, ADDR_SIZE_4GB,
	ADDR_SIZE_8GB
};

static char *ddr_type = "DDR3";

/*
 * Set 1 to use dynamic DUNIT configuration,
 * set 0 (supported for A380 and AC3) to configure DUNIT in values set by
 * ddr3_tip_init_specific_reg_config
 */
u8 generic_init_controller = 1;

#ifdef SUPPORT_STATIC_DUNIT_CONFIG
static u32 ddr3_get_static_ddr_mode(void);
#endif
static int ddr3_hws_tune_training_params(u8 dev_num);

/* device revision */
#define DEV_VERSION_ID_REG		0x1823c
#define REVISON_ID_OFFS			8
#define REVISON_ID_MASK			0xf00

/* A38x revisions */
#define MV_88F68XX_Z1_ID		0x0
#define MV_88F68XX_A0_ID		0x4
/* A39x revisions */
#define MV_88F69XX_Z1_ID		0x2

/*
 * sys_env_device_rev_get - Get Marvell controller device revision number
 *
 * DESCRIPTION:
 *       This function returns 8bit describing the device revision as defined
 *       Revision ID Register.
 *
 * INPUT:
 *       None.
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       8bit desscribing Marvell controller revision number
 */
u8 sys_env_device_rev_get(void)
{
	u32 value;

	value = reg_read(DEV_VERSION_ID_REG);
	return (value & (REVISON_ID_MASK)) >> REVISON_ID_OFFS;
}

/*
 * sys_env_dlb_config_ptr_get
 *
 * DESCRIPTION: defines pointer to to DLB COnfiguration table
 *
 * INPUT: none
 *
 * OUTPUT: pointer to DLB COnfiguration table
 *
 * RETURN:
 *       returns pointer to DLB COnfiguration table
 */
struct dlb_config *sys_env_dlb_config_ptr_get(void)
{
#ifdef CONFIG_ARMADA_39X
	return &ddr3_dlb_config_table_a0[0];
#else
	if (sys_env_device_rev_get() == MV_88F68XX_A0_ID)
		return &ddr3_dlb_config_table_a0[0];
	else
		return &ddr3_dlb_config_table[0];
#endif
}

/*
 * sys_env_get_cs_ena_from_reg
 *
 * DESCRIPTION: Get bit mask of enabled CS
 *
 * INPUT: None
 *
 * OUTPUT: None
 *
 * RETURN:
 *       Bit mask of enabled CS, 1 if only CS0 enabled,
 *       3 if both CS0 and CS1 enabled
 */
u32 sys_env_get_cs_ena_from_reg(void)
{
	return reg_read(REG_DDR3_RANK_CTRL_ADDR) &
		REG_DDR3_RANK_CTRL_CS_ENA_MASK;
}

static void ddr3_restore_and_set_final_windows(u32 *win)
{
	u32 win_ctrl_reg, num_of_win_regs;
	u32 cs_ena = sys_env_get_cs_ena_from_reg();
	u32 ui;

	win_ctrl_reg = REG_XBAR_WIN_4_CTRL_ADDR;
	num_of_win_regs = 16;

	/* Return XBAR windows 4-7 or 16-19 init configuration */
	for (ui = 0; ui < num_of_win_regs; ui++)
		reg_write((win_ctrl_reg + 0x4 * ui), win[ui]);

	printf("%s Training Sequence - Switching XBAR Window to FastPath Window\n",
	       ddr_type);

#if defined DYNAMIC_CS_SIZE_CONFIG
	if (ddr3_fast_path_dynamic_cs_size_config(cs_ena) != MV_OK)
		printf("ddr3_fast_path_dynamic_cs_size_config FAILED\n");
#else
	u32 reg, cs;
	reg = 0x1fffffe1;
	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs)) {
			reg |= (cs << 2);
			break;
		}
	}
	/* Open fast path Window to - 0.5G */
	reg_write(REG_FASTPATH_WIN_0_CTRL_ADDR, reg);
#endif
}

static int ddr3_save_and_set_training_windows(u32 *win)
{
	u32 cs_ena;
	u32 reg, tmp_count, cs, ui;
	u32 win_ctrl_reg, win_base_reg, win_remap_reg;
	u32 num_of_win_regs, win_jump_index;
	win_ctrl_reg = REG_XBAR_WIN_4_CTRL_ADDR;
	win_base_reg = REG_XBAR_WIN_4_BASE_ADDR;
	win_remap_reg = REG_XBAR_WIN_4_REMAP_ADDR;
	win_jump_index = 0x10;
	num_of_win_regs = 16;
	struct hws_topology_map *tm = ddr3_get_topology_map();

#ifdef DISABLE_L2_FILTERING_DURING_DDR_TRAINING
	/*
	 * Disable L2 filtering during DDR training
	 * (when Cross Bar window is open)
	 */
	reg_write(ADDRESS_FILTERING_END_REGISTER, 0);
#endif

	cs_ena = tm->interface_params[0].as_bus_params[0].cs_bitmask;

	/* Close XBAR Window 19 - Not needed */
	/* {0x000200e8}  -   Open Mbus Window - 2G */
	reg_write(REG_XBAR_WIN_19_CTRL_ADDR, 0);

	/* Save XBAR Windows 4-19 init configurations */
	for (ui = 0; ui < num_of_win_regs; ui++)
		win[ui] = reg_read(win_ctrl_reg + 0x4 * ui);

	/* Open XBAR Windows 4-7 or 16-19 for other CS */
	reg = 0;
	tmp_count = 0;
	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs)) {
			switch (cs) {
			case 0:
				reg = 0x0e00;
				break;
			case 1:
				reg = 0x0d00;
				break;
			case 2:
				reg = 0x0b00;
				break;
			case 3:
				reg = 0x0700;
				break;
			}
			reg |= (1 << 0);
			reg |= (SDRAM_CS_SIZE & 0xffff0000);

			reg_write(win_ctrl_reg + win_jump_index * tmp_count,
				  reg);
			reg = (((SDRAM_CS_SIZE + 1) * (tmp_count)) &
			       0xffff0000);
			reg_write(win_base_reg + win_jump_index * tmp_count,
				  reg);

			if (win_remap_reg <= REG_XBAR_WIN_7_REMAP_ADDR)
				reg_write(win_remap_reg +
					  win_jump_index * tmp_count, 0);

			tmp_count++;
		}
	}

	return MV_OK;
}

/*
 * Name:     ddr3_init - Main DDR3 Init function
 * Desc:     This routine initialize the DDR3 MC and runs HW training.
 * Args:     None.
 * Notes:
 * Returns:  None.
 */
int ddr3_init(void)
{
	u32 reg = 0;
	u32 soc_num;
	int status;
	u32 win[16];

	/* SoC/Board special Initializtions */
	/* Get version from internal library */
	ddr3_print_version();

	/*Add sub_version string */
	DEBUG_INIT_C("", SUB_VERSION, 1);

	/* Switching CPU to MRVL ID */
	soc_num = (reg_read(REG_SAMPLE_RESET_HIGH_ADDR) & SAR1_CPU_CORE_MASK) >>
		SAR1_CPU_CORE_OFFSET;
	switch (soc_num) {
	case 0x3:
	case 0x1:
		reg_bit_set(CPU_CONFIGURATION_REG(1), CPU_MRVL_ID_OFFSET);
	case 0x0:
		reg_bit_set(CPU_CONFIGURATION_REG(0), CPU_MRVL_ID_OFFSET);
	default:
		break;
	}

	/*
	 * Set DRAM Reset Mask in case detected GPIO indication of wakeup from
	 * suspend i.e the DRAM values will not be overwritten / reset when
	 * waking from suspend
	 */
	if (sys_env_suspend_wakeup_check() ==
	    SUSPEND_WAKEUP_ENABLED_GPIO_DETECTED) {
		reg_bit_set(REG_SDRAM_INIT_CTRL_ADDR,
			    1 << REG_SDRAM_INIT_RESET_MASK_OFFS);
	}

	/*
	 * Stage 0 - Set board configuration
	 */

	/* Check if DRAM is already initialized  */
	if (reg_read(REG_BOOTROM_ROUTINE_ADDR) &
	    (1 << REG_BOOTROM_ROUTINE_DRAM_INIT_OFFS)) {
		printf("%s Training Sequence - 2nd boot - Skip\n", ddr_type);
		return MV_OK;
	}

	/*
	 * Stage 1 - Dunit Setup
	 */

	/* Fix read ready phases for all SOC in reg 0x15c8 */
	reg = reg_read(REG_TRAINING_DEBUG_3_ADDR);
	reg &= ~(REG_TRAINING_DEBUG_3_MASK);
	reg |= 0x4;		/* Phase 0 */
	reg &= ~(REG_TRAINING_DEBUG_3_MASK << REG_TRAINING_DEBUG_3_OFFS);
	reg |= (0x4 << (1 * REG_TRAINING_DEBUG_3_OFFS));	/* Phase 1 */
	reg &= ~(REG_TRAINING_DEBUG_3_MASK << (3 * REG_TRAINING_DEBUG_3_OFFS));
	reg |= (0x6 << (3 * REG_TRAINING_DEBUG_3_OFFS));	/* Phase 3 */
	reg &= ~(REG_TRAINING_DEBUG_3_MASK << (4 * REG_TRAINING_DEBUG_3_OFFS));
	reg |= (0x6 << (4 * REG_TRAINING_DEBUG_3_OFFS));
	reg &= ~(REG_TRAINING_DEBUG_3_MASK << (5 * REG_TRAINING_DEBUG_3_OFFS));
	reg |= (0x6 << (5 * REG_TRAINING_DEBUG_3_OFFS));
	reg_write(REG_TRAINING_DEBUG_3_ADDR, reg);

	/*
	 * Axi_bresp_mode[8] = Compliant,
	 * Axi_addr_decode_cntrl[11] = Internal,
	 * Axi_data_bus_width[0] = 128bit
	 * */
	/* 0x14a8 - AXI Control Register */
	reg_write(REG_DRAM_AXI_CTRL_ADDR, 0);

	/*
	 * Stage 2 - Training Values Setup
	 */
	/* Set X-BAR windows for the training sequence */
	ddr3_save_and_set_training_windows(win);

#ifdef SUPPORT_STATIC_DUNIT_CONFIG
	/*
	 * Load static controller configuration (in case dynamic/generic init
	 * is not enabled
	 */
	if (generic_init_controller == 0) {
		ddr3_tip_init_specific_reg_config(0,
						  ddr_modes
						  [ddr3_get_static_ddr_mode
						   ()].regs);
	}
#endif

	/* Tune training algo paramteres */
	status = ddr3_hws_tune_training_params(0);
	if (MV_OK != status)
		return status;

	/* Set log level for training lib */
	ddr3_hws_set_log_level(DEBUG_BLOCK_ALL, DEBUG_LEVEL_ERROR);

	/* Start New Training IP */
	status = ddr3_hws_hw_training();
	if (MV_OK != status) {
		printf("%s Training Sequence - FAILED\n", ddr_type);
		return status;
	}

	/*
	 * Stage 3 - Finish
	 */
	/* Restore and set windows */
	ddr3_restore_and_set_final_windows(win);

	/* Update DRAM init indication in bootROM register */
	reg = reg_read(REG_BOOTROM_ROUTINE_ADDR);
	reg_write(REG_BOOTROM_ROUTINE_ADDR,
		  reg | (1 << REG_BOOTROM_ROUTINE_DRAM_INIT_OFFS));

	/* DLB config */
	ddr3_new_tip_dlb_config();

#if defined(ECC_SUPPORT)
	if (ddr3_if_ecc_enabled())
		ddr3_new_tip_ecc_scrub();
#endif

	printf("%s Training Sequence - Ended Successfully\n", ddr_type);

	return MV_OK;
}

/*
 * Name:     ddr3_get_cpu_freq
 * Desc:     read S@R and return CPU frequency
 * Args:
 * Notes:
 * Returns:  required value
 */
u32 ddr3_get_cpu_freq(void)
{
	return ddr3_tip_get_init_freq();
}

/*
 * Name:     ddr3_get_fab_opt
 * Desc:     read S@R and return CPU frequency
 * Args:
 * Notes:
 * Returns:  required value
 */
u32 ddr3_get_fab_opt(void)
{
	return 0;		/* No fabric */
}

/*
 * Name:     ddr3_get_static_m_cValue - Init Memory controller with
 *           static parameters
 * Desc:     Use this routine to init the controller without the HW training
 *           procedure.
 *           User must provide compatible header file with registers data.
 * Args:     None.
 * Notes:
 * Returns:  None.
 */
u32 ddr3_get_static_mc_value(u32 reg_addr, u32 offset1, u32 mask1,
			     u32 offset2, u32 mask2)
{
	u32 reg, temp;

	reg = reg_read(reg_addr);

	temp = (reg >> offset1) & mask1;
	if (mask2)
		temp |= (reg >> offset2) & mask2;

	return temp;
}

/*
 * Name:     ddr3_get_static_ddr_mode - Init Memory controller with
 *           static parameters
 * Desc:     Use this routine to init the controller without the HW training
 *           procedure.
 *           User must provide compatible header file with registers data.
 * Args:     None.
 * Notes:
 * Returns:  None.
 */
u32 ddr3_get_static_ddr_mode(void)
{
	u32 chip_board_rev, i;
	u32 size;

	/* Valid only for A380 only, MSYS using dynamic controller config */
#ifdef CONFIG_CUSTOMER_BOARD_SUPPORT
	/*
	 * Customer boards select DDR mode according to
	 * board ID & Sample@Reset
	 */
	chip_board_rev = mv_board_id_get();
#else
	/* Marvell boards select DDR mode according to Sample@Reset only */
	chip_board_rev = MARVELL_BOARD;
#endif

	size = ARRAY_SIZE(ddr_modes);
	for (i = 0; i < size; i++) {
		if ((ddr3_get_cpu_freq() == ddr_modes[i].cpu_freq) &&
		    (ddr3_get_fab_opt() == ddr_modes[i].fab_freq) &&
		    (chip_board_rev == ddr_modes[i].chip_board_rev))
			return i;
	}

	DEBUG_INIT_S("\n*** Error: ddr3_get_static_ddr_mode: No match for requested DDR mode. ***\n\n");

	return 0;
}

/******************************************************************************
 * Name:     ddr3_get_cs_num_from_reg
 * Desc:
 * Args:
 * Notes:
 * Returns:
 */
u32 ddr3_get_cs_num_from_reg(void)
{
	u32 cs_ena = sys_env_get_cs_ena_from_reg();
	u32 cs_count = 0;
	u32 cs;

	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs))
			cs_count++;
	}

	return cs_count;
}

void get_target_freq(u32 freq_mode, u32 *ddr_freq, u32 *hclk_ps)
{
	u32 tmp, hclk = 200;

	switch (freq_mode) {
	case 4:
		tmp = 1;	/* DDR_400; */
		hclk = 200;
		break;
	case 0x8:
		tmp = 1;	/* DDR_666; */
		hclk = 333;
		break;
	case 0xc:
		tmp = 1;	/* DDR_800; */
		hclk = 400;
		break;
	default:
		*ddr_freq = 0;
		*hclk_ps = 0;
		break;
	}

	*ddr_freq = tmp;		/* DDR freq define */
	*hclk_ps = 1000000 / hclk;	/* values are 1/HCLK in ps */

	return;
}

void ddr3_new_tip_dlb_config(void)
{
	u32 reg, i = 0;
	struct dlb_config *config_table_ptr = sys_env_dlb_config_ptr_get();

	/* Write the configuration */
	while (config_table_ptr[i].reg_addr != 0) {
		reg_write(config_table_ptr[i].reg_addr,
			  config_table_ptr[i].reg_data);
		i++;
	}

	/* Enable DLB */
	reg = reg_read(REG_STATIC_DRAM_DLB_CONTROL);
	reg |= DLB_ENABLE | DLB_WRITE_COALESING | DLB_AXI_PREFETCH_EN |
		DLB_MBUS_PREFETCH_EN | PREFETCH_N_LN_SZ_TR;
	reg_write(REG_STATIC_DRAM_DLB_CONTROL, reg);
}

int ddr3_fast_path_dynamic_cs_size_config(u32 cs_ena)
{
	u32 reg, cs;
	u32 mem_total_size = 0;
	u32 cs_mem_size = 0;
	u32 mem_total_size_c, cs_mem_size_c;

#ifdef DEVICE_MAX_DRAM_ADDRESS_SIZE
	u32 physical_mem_size;
	u32 max_mem_size = DEVICE_MAX_DRAM_ADDRESS_SIZE;
	struct hws_topology_map *tm = ddr3_get_topology_map();
#endif

	/* Open fast path windows */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs)) {
			/* get CS size */
			if (ddr3_calc_mem_cs_size(cs, &cs_mem_size) != MV_OK)
				return MV_FAIL;

#ifdef DEVICE_MAX_DRAM_ADDRESS_SIZE
			/*
			 * if number of address pins doesn't allow to use max
			 * mem size that is defined in topology
			 * mem size is defined by DEVICE_MAX_DRAM_ADDRESS_SIZE
			 */
			physical_mem_size = mem_size
				[tm->interface_params[0].memory_size];

			if (ddr3_get_device_width(cs) == 16) {
				/*
				 * 16bit mem device can be twice more - no need
				 * in less significant pin
				 */
				max_mem_size = DEVICE_MAX_DRAM_ADDRESS_SIZE * 2;
			}

			if (physical_mem_size > max_mem_size) {
				cs_mem_size = max_mem_size *
					(ddr3_get_bus_width() /
					 ddr3_get_device_width(cs));
				printf("Updated Physical Mem size is from 0x%x to %x\n",
				       physical_mem_size,
				       DEVICE_MAX_DRAM_ADDRESS_SIZE);
			}
#endif

			/* set fast path window control for the cs */
			reg = 0xffffe1;
			reg |= (cs << 2);
			reg |= (cs_mem_size - 1) & 0xffff0000;
			/*Open fast path Window */
			reg_write(REG_FASTPATH_WIN_CTRL_ADDR(cs), reg);

			/* Set fast path window base address for the cs */
			reg = ((cs_mem_size) * cs) & 0xffff0000;
			/* Set base address */
			reg_write(REG_FASTPATH_WIN_BASE_ADDR(cs), reg);

			/*
			 * Since memory size may be bigger than 4G the summ may
			 * be more than 32 bit word,
			 * so to estimate the result divide mem_total_size and
			 * cs_mem_size by 0x10000 (it is equal to >> 16)
			 */
			mem_total_size_c = mem_total_size >> 16;
			cs_mem_size_c = cs_mem_size >> 16;
			/* if the sum less than 2 G - calculate the value */
			if (mem_total_size_c + cs_mem_size_c < 0x10000)
				mem_total_size += cs_mem_size;
			else	/* put max possible size */
				mem_total_size = L2_FILTER_FOR_MAX_MEMORY_SIZE;
		}
	}

	/* Set L2 filtering to Max Memory size */
	reg_write(ADDRESS_FILTERING_END_REGISTER, mem_total_size);

	return MV_OK;
}

u32 ddr3_get_bus_width(void)
{
	u32 bus_width;

	bus_width = (reg_read(REG_SDRAM_CONFIG_ADDR) & 0x8000) >>
		REG_SDRAM_CONFIG_WIDTH_OFFS;

	return (bus_width == 0) ? 16 : 32;
}

u32 ddr3_get_device_width(u32 cs)
{
	u32 device_width;

	device_width = (reg_read(REG_SDRAM_ADDRESS_CTRL_ADDR) &
			(0x3 << (REG_SDRAM_ADDRESS_CTRL_STRUCT_OFFS * cs))) >>
		(REG_SDRAM_ADDRESS_CTRL_STRUCT_OFFS * cs);

	return (device_width == 0) ? 8 : 16;
}

float ddr3_get_device_size(u32 cs)
{
	u32 device_size_low, device_size_high, device_size;
	u32 data, cs_low_offset, cs_high_offset;

	cs_low_offset = REG_SDRAM_ADDRESS_SIZE_OFFS + cs * 4;
	cs_high_offset = REG_SDRAM_ADDRESS_SIZE_OFFS +
		REG_SDRAM_ADDRESS_SIZE_HIGH_OFFS + cs;

	data = reg_read(REG_SDRAM_ADDRESS_CTRL_ADDR);
	device_size_low = (data >> cs_low_offset) & 0x3;
	device_size_high = (data >> cs_high_offset) & 0x1;

	device_size = device_size_low | (device_size_high << 2);

	switch (device_size) {
	case 0:
		return 2;
	case 2:
		return 0.5;
	case 3:
		return 1;
	case 4:
		return 4;
	case 5:
		return 8;
	case 1:
	default:
		DEBUG_INIT_C("Error: Wrong device size of Cs: ", cs, 1);
		/*
		 * Small value will give wrong emem size in
		 * ddr3_calc_mem_cs_size
		 */
		return 0.01;
	}
}

int ddr3_calc_mem_cs_size(u32 cs, u32 *cs_size)
{
	float cs_mem_size;

	/* Calculate in GiB */
	cs_mem_size = ((ddr3_get_bus_width() / ddr3_get_device_width(cs)) *
		       ddr3_get_device_size(cs)) / 8;

	/*
	 * Multiple controller bus width, 2x for 64 bit
	 * (SoC controller may be 32 or 64 bit,
	 * so bit 15 in 0x1400, that means if whole bus used or only half,
	 * have a differnt meaning
	 */
	cs_mem_size *= DDR_CONTROLLER_BUS_WIDTH_MULTIPLIER;

	if (cs_mem_size == 0.125) {
		*cs_size = 128 << 20;
	} else if (cs_mem_size == 0.25) {
		*cs_size = 256 << 20;
	} else if (cs_mem_size == 0.5) {
		*cs_size = 512 << 20;
	} else if (cs_mem_size == 1) {
		*cs_size = 1 << 30;
	} else if (cs_mem_size == 2) {
		*cs_size = 2 << 30;
	} else {
		DEBUG_INIT_C("Error: Wrong Memory size of Cs: ", cs, 1);
		return MV_BAD_VALUE;
	}

	return MV_OK;
}

/*
 * Name:     ddr3_hws_tune_training_params
 * Desc:
 * Args:
 * Notes: Tune internal training params
 * Returns:
 */
static int ddr3_hws_tune_training_params(u8 dev_num)
{
	struct tune_train_params params;
	int status;

	/* NOTE: do not remove any field initilization */
	params.ck_delay = TUNE_TRAINING_PARAMS_CK_DELAY;
	params.ck_delay_16 = TUNE_TRAINING_PARAMS_CK_DELAY_16;
	params.p_finger = TUNE_TRAINING_PARAMS_PFINGER;
	params.n_finger = TUNE_TRAINING_PARAMS_NFINGER;
	params.phy_reg3_val = TUNE_TRAINING_PARAMS_PHYREG3VAL;

	status = ddr3_tip_tune_training_params(dev_num, &params);
	if (MV_OK != status) {
		printf("%s Training Sequence - FAILED\n", ddr_type);
		return status;
	}

	return MV_OK;
}

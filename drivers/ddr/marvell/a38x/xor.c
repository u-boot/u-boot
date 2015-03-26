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
#include "xor_regs.h"

/* defines  */
#ifdef MV_DEBUG
#define DB(x)	x
#else
#define DB(x)
#endif

static u32 ui_xor_regs_ctrl_backup;
static u32 ui_xor_regs_base_backup[MAX_CS];
static u32 ui_xor_regs_mask_backup[MAX_CS];

void mv_sys_xor_init(u32 num_of_cs, u32 cs_ena, u32 cs_size, u32 base_delta)
{
	u32 reg, ui, base, cs_count;

	ui_xor_regs_ctrl_backup = reg_read(XOR_WINDOW_CTRL_REG(0, 0));
	for (ui = 0; ui < MAX_CS; ui++)
		ui_xor_regs_base_backup[ui] =
			reg_read(XOR_BASE_ADDR_REG(0, ui));
	for (ui = 0; ui < MAX_CS; ui++)
		ui_xor_regs_mask_backup[ui] =
			reg_read(XOR_SIZE_MASK_REG(0, ui));

	reg = 0;
	for (ui = 0; ui < (num_of_cs); ui++) {
		/* Enable Window x for each CS */
		reg |= (0x1 << (ui));
		/* Enable Window x for each CS */
		reg |= (0x3 << ((ui * 2) + 16));
	}

	reg_write(XOR_WINDOW_CTRL_REG(0, 0), reg);

	cs_count = 0;
	for (ui = 0; ui < num_of_cs; ui++) {
		if (cs_ena & (1 << ui)) {
			/*
			 * window x - Base - 0x00000000,
			 * Attribute 0x0e - DRAM
			 */
			base = cs_size * ui + base_delta;
			switch (ui) {
			case 0:
				base |= 0xe00;
				break;
			case 1:
				base |= 0xd00;
				break;
			case 2:
				base |= 0xb00;
				break;
			case 3:
				base |= 0x700;
				break;
			}

			reg_write(XOR_BASE_ADDR_REG(0, cs_count), base);

			/* window x - Size */
			reg_write(XOR_SIZE_MASK_REG(0, cs_count), 0x7fff0000);
			cs_count++;
		}
	}

	mv_xor_hal_init(1);

	return;
}

void mv_sys_xor_finish(void)
{
	u32 ui;

	reg_write(XOR_WINDOW_CTRL_REG(0, 0), ui_xor_regs_ctrl_backup);
	for (ui = 0; ui < MAX_CS; ui++)
		reg_write(XOR_BASE_ADDR_REG(0, ui),
			  ui_xor_regs_base_backup[ui]);
	for (ui = 0; ui < MAX_CS; ui++)
		reg_write(XOR_SIZE_MASK_REG(0, ui),
			  ui_xor_regs_mask_backup[ui]);

	reg_write(XOR_ADDR_OVRD_REG(0, 0), 0);
}

/*
 * mv_xor_hal_init - Initialize XOR engine
 *
 * DESCRIPTION:
 *               This function initialize XOR unit.
 * INPUT:
 *       None.
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       MV_BAD_PARAM if parameters to function invalid, MV_OK otherwise.
 */
void mv_xor_hal_init(u32 xor_chan_num)
{
	u32 i;

	/* Abort any XOR activity & set default configuration */
	for (i = 0; i < xor_chan_num; i++) {
		mv_xor_command_set(i, MV_STOP);
		mv_xor_ctrl_set(i, (1 << XEXCR_REG_ACC_PROTECT_OFFS) |
				(4 << XEXCR_DST_BURST_LIMIT_OFFS) |
				(4 << XEXCR_SRC_BURST_LIMIT_OFFS));
	}
}

/*
 * mv_xor_ctrl_set - Set XOR channel control registers
 *
 * DESCRIPTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       MV_BAD_PARAM if parameters to function invalid, MV_OK otherwise.
 * NOTE:
 *  This function does not modify the Operation_mode field of control register.
 */
int mv_xor_ctrl_set(u32 chan, u32 xor_ctrl)
{
	u32 old_value;

	/* update the XOR Engine [0..1] Configuration Registers (XEx_c_r) */
	old_value = reg_read(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan))) &
		XEXCR_OPERATION_MODE_MASK;
	xor_ctrl &= ~XEXCR_OPERATION_MODE_MASK;
	xor_ctrl |= old_value;
	reg_write(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan)), xor_ctrl);

	return MV_OK;
}

int mv_xor_mem_init(u32 chan, u32 start_ptr, u32 block_size,
		    u32 init_val_high, u32 init_val_low)
{
	u32 temp;

	/* Parameter checking */
	if (chan >= MV_XOR_MAX_CHAN)
		return MV_BAD_PARAM;

	if (MV_ACTIVE == mv_xor_state_get(chan))
		return MV_BUSY;

	if ((block_size < XEXBSR_BLOCK_SIZE_MIN_VALUE) ||
	    (block_size > XEXBSR_BLOCK_SIZE_MAX_VALUE))
		return MV_BAD_PARAM;

	/* set the operation mode to Memory Init */
	temp = reg_read(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan)));
	temp &= ~XEXCR_OPERATION_MODE_MASK;
	temp |= XEXCR_OPERATION_MODE_MEM_INIT;
	reg_write(XOR_CONFIG_REG(XOR_UNIT(chan), XOR_CHAN(chan)), temp);

	/*
	 * update the start_ptr field in XOR Engine [0..1] Destination Pointer
	 * Register
	 */
	reg_write(XOR_DST_PTR_REG(XOR_UNIT(chan), XOR_CHAN(chan)), start_ptr);

	/*
	 * update the Block_size field in the XOR Engine[0..1] Block Size
	 * Registers
	 */
	reg_write(XOR_BLOCK_SIZE_REG(XOR_UNIT(chan), XOR_CHAN(chan)),
		  block_size);

	/*
	 * update the field Init_val_l in the XOR Engine Initial Value Register
	 * Low (XEIVRL)
	 */
	reg_write(XOR_INIT_VAL_LOW_REG(XOR_UNIT(chan)), init_val_low);

	/*
	 * update the field Init_val_h in the XOR Engine Initial Value Register
	 * High (XEIVRH)
	 */
	reg_write(XOR_INIT_VAL_HIGH_REG(XOR_UNIT(chan)), init_val_high);

	/* start transfer */
	reg_bit_set(XOR_ACTIVATION_REG(XOR_UNIT(chan), XOR_CHAN(chan)),
		    XEXACTR_XESTART_MASK);

	return MV_OK;
}

/*
 * mv_xor_state_get - Get XOR channel state.
 *
 * DESCRIPTION:
 *       XOR channel activity state can be active, idle, paused.
 *       This function retrunes the channel activity state.
 *
 * INPUT:
 *       chan     - the channel number
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       XOR_CHANNEL_IDLE    - If the engine is idle.
 *       XOR_CHANNEL_ACTIVE  - If the engine is busy.
 *       XOR_CHANNEL_PAUSED  - If the engine is paused.
 *       MV_UNDEFINED_STATE  - If the engine state is undefind or there is no
 *                             such engine
 */
enum mv_state mv_xor_state_get(u32 chan)
{
	u32 state;

	/* Parameter checking   */
	if (chan >= MV_XOR_MAX_CHAN) {
		DB(printf("%s: ERR. Invalid chan num %d\n", __func__, chan));
		return MV_UNDEFINED_STATE;
	}

	/* read the current state */
	state = reg_read(XOR_ACTIVATION_REG(XOR_UNIT(chan), XOR_CHAN(chan)));
	state &= XEXACTR_XESTATUS_MASK;

	/* return the state */
	switch (state) {
	case XEXACTR_XESTATUS_IDLE:
		return MV_IDLE;
	case XEXACTR_XESTATUS_ACTIVE:
		return MV_ACTIVE;
	case XEXACTR_XESTATUS_PAUSED:
		return MV_PAUSED;
	}

	return MV_UNDEFINED_STATE;
}

/*
 * mv_xor_command_set - Set command of XOR channel
 *
 * DESCRIPTION:
 *       XOR channel can be started, idle, paused and restarted.
 *       Paused can be set only if channel is active.
 *       Start can be set only if channel is idle or paused.
 *       Restart can be set only if channel is paused.
 *       Stop can be set only if channel is active.
 *
 * INPUT:
 *       chan     - The channel number
 *       command  - The command type (start, stop, restart, pause)
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       MV_OK on success , MV_BAD_PARAM on erroneous parameter, MV_ERROR on
 *       undefind XOR engine mode
 */
int mv_xor_command_set(u32 chan, enum mv_command command)
{
	enum mv_state state;

	/* Parameter checking */
	if (chan >= MV_XOR_MAX_CHAN) {
		DB(printf("%s: ERR. Invalid chan num %d\n", __func__, chan));
		return MV_BAD_PARAM;
	}

	/* get the current state */
	state = mv_xor_state_get(chan);

	if ((command == MV_START) && (state == MV_IDLE)) {
		/* command is start and current state is idle */
		reg_bit_set(XOR_ACTIVATION_REG
			    (XOR_UNIT(chan), XOR_CHAN(chan)),
			    XEXACTR_XESTART_MASK);
		return MV_OK;
	} else if ((command == MV_STOP) && (state == MV_ACTIVE)) {
		/* command is stop and current state is active */
		reg_bit_set(XOR_ACTIVATION_REG
			    (XOR_UNIT(chan), XOR_CHAN(chan)),
			    XEXACTR_XESTOP_MASK);
		return MV_OK;
	} else if (((enum mv_state)command == MV_PAUSED) &&
		   (state == MV_ACTIVE)) {
		/* command is paused and current state is active */
		reg_bit_set(XOR_ACTIVATION_REG
			    (XOR_UNIT(chan), XOR_CHAN(chan)),
			    XEXACTR_XEPAUSE_MASK);
		return MV_OK;
	} else if ((command == MV_RESTART) && (state == MV_PAUSED)) {
		/* command is restart and current state is paused */
		reg_bit_set(XOR_ACTIVATION_REG
			    (XOR_UNIT(chan), XOR_CHAN(chan)),
			    XEXACTR_XERESTART_MASK);
		return MV_OK;
	} else if ((command == MV_STOP) && (state == MV_IDLE)) {
		/* command is stop and current state is active */
		return MV_OK;
	}

	/* illegal command */
	DB(printf("%s: ERR. Illegal command\n", __func__));

	return MV_BAD_PARAM;
}

void ddr3_new_tip_ecc_scrub(void)
{
	u32 cs_c, max_cs;
	u32 cs_ena = 0;

	printf("DDR3 Training Sequence - Start scrubbing\n");

	max_cs = hws_ddr3_tip_max_cs_get();
	for (cs_c = 0; cs_c < max_cs; cs_c++)
		cs_ena |= 1 << cs_c;

	mv_sys_xor_init(max_cs, cs_ena, 0x80000000, 0);

	mv_xor_mem_init(0, 0x00000000, 0x80000000, 0xdeadbeef, 0xdeadbeef);
	/* wait for previous transfer completion */
	while (mv_xor_state_get(0) != MV_IDLE)
		;

	mv_xor_mem_init(0, 0x80000000, 0x40000000, 0xdeadbeef, 0xdeadbeef);

	/* wait for previous transfer completion */
	while (mv_xor_state_get(0) != MV_IDLE)
		;

	/* Return XOR State */
	mv_sys_xor_finish();

	printf("DDR3 Training Sequence - End scrubbing\n");
}

/* SPDX-License-Identifier: BSD-3-Clause */
/******************************************************************************
 *
 * Copyright (C) 2017-2018 Cadence Design Systems, Inc.
 * Copyright (C) 2019 Texas Instruments Incorporated - http://www.ti.com/
 *
 * cps_drv_lpddr4.h
 * Interface for the Register Accaess Layer of Cadence Platform Service (CPS)
 *****************************************************************************
 */

#ifndef CPS_DRV_H_
#define CPS_DRV_H_

#include <stddef.h>
#include <inttypes.h>
#include <asm/io.h>

/**
 *  \brief    Read a 32-bit value from memory.
 *  \param    reg   address of the memory mapped hardware register
 *  \return   the value at the given address
 */
#define CPS_REG_READ(reg) (readl((volatile uint32_t*)(reg)))

/**
 *  \brief   Write a 32-bit address value to memory.
 *  \param   reg     address of the memory mapped hardware register
 *  \param   value   unsigned 32-bit value to write
 */
#define CPS_REG_WRITE(reg, value) (writel((uint32_t)(value), (volatile uint32_t*)(reg)))

/**
 *  \brief    Subtitue the value of fld macro and concatinate with required string
 *  \param    fld         field name
 */
#define CPS_FLD_MASK(fld)  (fld ## _MASK)
#define CPS_FLD_SHIFT(fld) (fld ## _SHIFT)
#define CPS_FLD_WIDTH(fld) (fld ## _WIDTH)
#define CPS_FLD_WOCLR(fld) (fld ## _WOCLR)
#define CPS_FLD_WOSET(fld) (fld ## _WOSET)

/**
 *  \brief    Read a value of bit-field from the register value.
 *  \param    reg         register name
 *  \param    fld         field name
 *  \param    reg_value   register value
 *  \return   bit-field value
 */
#define CPS_FLD_READ(fld, reg_value) (cps_fldread((uint32_t)(CPS_FLD_MASK(fld)),  \
						(uint32_t)(CPS_FLD_SHIFT(fld)), \
						(uint32_t)(reg_value)))

/**
 *  \brief    Write a value of the bit-field into the register value.
 *  \param    reg         register name
 *  \param    fld         field name
 *  \param    reg_value   register value
 *  \param    value       value to be written to bit-field
 *  \return   modified register value
 */
#define CPS_FLD_WRITE(fld, reg_value, value) (cps_fldwrite((uint32_t)(CPS_FLD_MASK(fld)),  \
						(uint32_t)(CPS_FLD_SHIFT(fld)), \
						(uint32_t)(reg_value), (uint32_t)(value)))

/**
 *  \brief    Set bit within the register value.
 *  \param    reg         register name
 *  \param    fld         field name
 *  \param    reg_value   register value
 *  \return   modified register value
 */
#define CPS_FLD_SET(fld, reg_value) (cps_fldset((uint32_t)(CPS_FLD_WIDTH(fld)), \
					(uint32_t)(CPS_FLD_MASK(fld)),  \
					(uint32_t)(CPS_FLD_WOCLR(fld)), \
					(uint32_t)(reg_value)))

static inline uint32_t cps_fldread(uint32_t mask, uint32_t shift, uint32_t reg_value)
{
	uint32_t result = (reg_value & mask) >> shift;

	return (result);
}

/**
 *  \brief    Write a value of the bit-field into the register value.
 *  \param    mask        mask for the bit-field
 *  \param    shift       bit-field shift from LSB
 *  \param    reg_value   register value
 *  \param    value       value to be written to bit-field
 *  \return   modified register value
 */
static inline uint32_t cps_fldwrite(uint32_t mask, uint32_t shift, uint32_t reg_value, uint32_t value)
{
	uint32_t new_value = (value << shift) & mask;

	new_value = (reg_value & ~mask) | new_value;
	return (new_value);
}

/**
 *  \brief    Set bit within the register value.
 *  \param    width       width of the bit-field
 *  \param    mask        mask for the bit-field
 *  \param    is_woclr    is bit-field has 'write one to clear' flag set
 *  \param    reg_value   register value
 *  \return   modified register value
 */
static inline uint32_t cps_fldset(uint32_t width, uint32_t mask, uint32_t is_woclr, uint32_t reg_value)
{
	uint32_t new_value = reg_value;
	/* Confirm the field to be bit and not write to clear type */
	if ((width == 1U) && (is_woclr == 0U)) {
		new_value |= mask;
	}

	return (new_value);
}
#endif /* CPS_DRV_H_ */

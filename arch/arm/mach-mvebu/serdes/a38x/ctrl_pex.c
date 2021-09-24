// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#include "ctrl_pex.h"
#include "sys_env_lib.h"

__weak void board_pex_config(void)
{
	/* nothing in this weak default implementation */
}

int hws_pex_config(const struct serdes_map *serdes_map, u8 count)
{
	enum serdes_type serdes_type;
	u32 idx, tmp;

	DEBUG_INIT_FULL_S("\n### hws_pex_config ###\n");

	tmp = reg_read(SOC_CONTROL_REG1);
	tmp &= ~0x03;

	for (idx = 0; idx < count; idx++) {
		serdes_type = serdes_map[idx].serdes_type;
		if ((serdes_type != PEX0) &&
		    ((serdes_map[idx].serdes_mode == PEX_ROOT_COMPLEX_X4) ||
		     (serdes_map[idx].serdes_mode == PEX_END_POINT_X4))) {
			/* for PEX by4 - relevant for the first port only */
			continue;
		}

		switch (serdes_type) {
		case PEX0:
			tmp |= 0x1 << PCIE0_ENABLE_OFFS;
			break;
		case PEX1:
			tmp |= 0x1 << PCIE1_ENABLE_OFFS;
			break;
		case PEX2:
			tmp |= 0x1 << PCIE2_ENABLE_OFFS;
			break;
		case PEX3:
			tmp |= 0x1 << PCIE3_ENABLE_OFFS;
			break;
		default:
			break;
		}
	}

	reg_write(SOC_CONTROL_REG1, tmp);

	board_pex_config();

	return MV_OK;
}

int pex_local_bus_num_set(u32 pex_if, u32 bus_num)
{
	u32 pex_status;

	DEBUG_INIT_FULL_S("\n### pex_local_bus_num_set ###\n");

	if (bus_num >= MAX_PEX_BUSSES) {
		DEBUG_INIT_C("pex_local_bus_num_set: Illegal bus number %d\n",
			     bus_num, 4);
		return MV_BAD_PARAM;
	}

	pex_status = reg_read(PEX_STATUS_REG(pex_if));
	pex_status &= ~PXSR_PEX_BUS_NUM_MASK;
	pex_status |=
	    (bus_num << PXSR_PEX_BUS_NUM_OFFS) & PXSR_PEX_BUS_NUM_MASK;
	reg_write(PEX_STATUS_REG(pex_if), pex_status);

	return MV_OK;
}

int pex_local_dev_num_set(u32 pex_if, u32 dev_num)
{
	u32 pex_status;

	DEBUG_INIT_FULL_S("\n### pex_local_dev_num_set ###\n");

	pex_status = reg_read(PEX_STATUS_REG(pex_if));
	pex_status &= ~PXSR_PEX_DEV_NUM_MASK;
	pex_status |=
	    (dev_num << PXSR_PEX_DEV_NUM_OFFS) & PXSR_PEX_DEV_NUM_MASK;
	reg_write(PEX_STATUS_REG(pex_if), pex_status);

	return MV_OK;
}

/*
 * pex_config_read - Read from configuration space
 *
 * DESCRIPTION:
 *       This function performs a 32 bit read from PEX configuration space.
 *       It supports both type 0 and type 1 of Configuration Transactions
 *       (local and over bridge). In order to read from local bus segment, use
 *       bus number retrieved from pex_local_bus_num_get(). Other bus numbers
 *       will result configuration transaction of type 1 (over bridge).
 *
 * INPUT:
 *       pex_if   - PEX interface number.
 *       bus      - PEX segment bus number.
 *       dev      - PEX device number.
 *       func     - Function number.
 *       reg_offs - Register offset.
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       32bit register data, 0xffffffff on error
 */
u32 pex_config_read(u32 pex_if, u32 bus, u32 dev, u32 func, u32 reg_off)
{
	u32 pex_data = 0;
	u32 local_dev, local_bus;
	u32 pex_status;

	pex_status = reg_read(PEX_STATUS_REG(pex_if));
	local_dev =
	    ((pex_status & PXSR_PEX_DEV_NUM_MASK) >> PXSR_PEX_DEV_NUM_OFFS);
	local_bus =
	    ((pex_status & PXSR_PEX_BUS_NUM_MASK) >> PXSR_PEX_BUS_NUM_OFFS);

	/*
	 * In PCI Express we have only one device number
	 * and this number is the first number we encounter
	 * else that the local_dev
	 * spec pex define return on config read/write on any device
	 */
	if (bus == local_bus) {
		if (local_dev == 0) {
			/*
			 * if local dev is 0 then the first number we encounter
			 * after 0 is 1
			 */
			if ((dev != 1) && (dev != local_dev))
				return MV_ERROR;
		} else {
			/*
			 * if local dev is not 0 then the first number we
			 * encounter is 0
			 */
			if ((dev != 0) && (dev != local_dev))
				return MV_ERROR;
		}
	}

	/* Creating PEX address to be passed */
	pex_data = (bus << PXCAR_BUS_NUM_OFFS);
	pex_data |= (dev << PXCAR_DEVICE_NUM_OFFS);
	pex_data |= (func << PXCAR_FUNC_NUM_OFFS);
	/* Legacy register space */
	pex_data |= (reg_off & PXCAR_REG_NUM_MASK);
	/* Extended register space */
	pex_data |= (((reg_off & PXCAR_REAL_EXT_REG_NUM_MASK) >>
		      PXCAR_REAL_EXT_REG_NUM_OFFS) << PXCAR_EXT_REG_NUM_OFFS);
	pex_data |= PXCAR_CONFIG_EN;

	/* Write the address to the PEX configuration address register */
	reg_write(PEX_CFG_ADDR_REG(pex_if), pex_data);

	/*
	 * In order to let the PEX controller absorbed the address
	 * of the read transaction we perform a validity check that
	 * the address was written
	 */
	if (pex_data != reg_read(PEX_CFG_ADDR_REG(pex_if)))
		return MV_ERROR;

	/* Cleaning Master Abort */
	reg_bit_set(PEX_CFG_DIRECT_ACCESS(pex_if, PEX_STATUS_AND_COMMAND),
		    PXSAC_MABORT);
	/* Read the Data returned in the PEX Data register */
	pex_data = reg_read(PEX_CFG_DATA_REG(pex_if));

	DEBUG_INIT_FULL_C(" --> ", pex_data, 4);

	return pex_data;
}

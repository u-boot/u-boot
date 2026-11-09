// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <dm/device_compat.h>
#include <linux/mtd/spi-nor.h>
#include "sf_internal.h"

static const struct flash_info macronix_nor_parts[] = {
	{ INFO("mx25l2005a",  0xc22012, 0, 64 * 1024,   4, SECT_4K) },
	{ INFO("mx25l4005a",  0xc22013, 0, 64 * 1024,   8, SECT_4K) },
	{ INFO("mx25l8005",   0xc22014, 0, 64 * 1024,  16, 0) },
	{ INFO("mx25l1606e",  0xc22015, 0, 64 * 1024,  32, SECT_4K) },
	{ INFO("mx25l3205d",  0xc22016, 0, 64 * 1024,  64, SECT_4K) },
	{ INFO("mx25l6405d",  0xc22017, 0, 64 * 1024, 128, SECT_4K) },
	{ INFO("mx25u2033e",  0xc22532, 0, 64 * 1024,   4, SECT_4K) },
	{ INFO("mx25u1635e",  0xc22535, 0, 64 * 1024,  32, SECT_4K) },
	{ INFO("mx25u3235f",  0xc22536, 0, 4 * 1024,  1024, SECT_4K) },
	{ INFO("mx25u6435f",  0xc22537, 0, 64 * 1024, 128, SECT_4K) },
	{ INFO("mx25l12805d", 0xc22018, 0, 64 * 1024, 256, SECT_4K) },
	{ INFO("mx25u12835f", 0xc22538, 0, 64 * 1024, 256, SECT_4K |
	       SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("mx25u51245g", 0xc2253a, 0, 64 * 1024, 1024, SECT_4K |
	       SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25l12855e", 0xc22618, 0, 64 * 1024, 256, 0) },
	{ INFO("mx25l25635e", 0xc22019, 0, 64 * 1024, 512, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("mx25u25635f", 0xc22539, 0, 64 * 1024, 512, SECT_4K | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25v8035f",  0xc22314, 0, 64 * 1024,  16, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("mx25r1635f",  0xc22815, 0, 64 * 1024,  32, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("mx25l25655e", 0xc22619, 0, 64 * 1024, 512, 0) },
	{ INFO("mx66l51235l", 0xc2201a, 0, 64 * 1024, 1024, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx66u51235f", 0xc2253a, 0, 64 * 1024, 1024, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25u51245f", 0xc2953a, 0, 64 * 1024, 1024, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx66u1g45g",  0xc2253b, 0, 64 * 1024, 2048, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx66u2g45g",  0xc2253c, 0, 64 * 1024, 4096, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx66l1g45g",  0xc2201b, 0, 64 * 1024, 2048, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("mx66l2g45g",  0xc2201c, 0, 64 * 1024, 4096, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25l1633e", 0xc22415, 0, 64 * 1024,   32, SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES | SECT_4K) },
	{ INFO("mx25r6435f", 0xc22817, 0, 64 * 1024,   128,  SECT_4K) },
	{ INFO("mx66uw2g345gx0", 0xc2943c, 0, 64 * 1024, 4096, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES | SPI_NOR_OCTAL_READ) },
	{ INFO("mx66lm1g45g",    0xc2853b, 0, 64 * 1024, 2048, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25lm51245g",   0xc2853a, 0, 64 * 1024, 1024, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25lw51245g",   0xc2863a, 0, 64 * 1024, 1024, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25lm25645g",   0xc28539, 0, 64 * 1024, 512, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx66uw2g345g",   0xc2843c, 0, 64 * 1024, 4096, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx66um1g45g",    0xc2803b, 0, 64 * 1024, 2048, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx66uw1g45g",    0xc2813b, 0, 64 * 1024, 2048, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25uw51245g",   0xc2813a, 0, 64 * 1024, 1024, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25uw51345g",   0xc2843a, 0, 64 * 1024, 1024, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25um25645g",   0xc28039, 0, 64 * 1024, 512, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25uw25645g",   0xc28139, 0, 64 * 1024, 512, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25um25345g",   0xc28339, 0, 64 * 1024, 512, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25uw25345g",   0xc28439, 0, 64 * 1024, 512, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25uw12845g",   0xc28138, 0, 64 * 1024, 256, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25uw12345g",   0xc28438, 0, 64 * 1024, 256, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25uw6445g",    0xc28137, 0, 64 * 1024, 128, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("mx25uw6345g",    0xc28437, 0, 64 * 1024, 128, SECT_4K | SPI_NOR_OCTAL_DTR_READ | SPI_NOR_4B_OPCODES) },
};

#if !CONFIG_IS_ENABLED(SPI_FLASH_TINY)
/**
 * spi_nor_macronix_octal_dtr_enable() - Enable octal DTR on Macronix flashes.
 * @nor:	pointer to a 'struct spi_nor'
 *
 * Set Macronix max dummy cycles 20 to allow the flash to run at fastest frequency.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int spi_nor_macronix_octal_dtr_enable(struct spi_nor *nor)
{
	struct spi_mem_op op;
	int ret;
	u8 buf;

	ret = write_enable(nor);
	if (ret)
		return ret;

	buf = SPINOR_REG_MXIC_DC_20;
	op = (struct spi_mem_op)
		SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_WR_CR2, 1),
			   SPI_MEM_OP_ADDR(4, SPINOR_REG_MXIC_CR2_DC, 1),
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_DATA_OUT(1, &buf, 1));

	ret = spi_mem_exec_op(nor->spi, &op);
	if (ret)
		return ret;

	ret = spi_nor_wait_till_ready(nor);
	if (ret)
		return ret;

	nor->read_dummy = MXIC_MAX_DC;
	ret = write_enable(nor);
	if (ret)
		return ret;

	buf = SPINOR_REG_MXIC_OPI_DTR_EN;
	op = (struct spi_mem_op)
		SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_WR_CR2, 1),
			   SPI_MEM_OP_ADDR(4, SPINOR_REG_MXIC_CR2_MODE, 1),
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_DATA_OUT(1, &buf, 1));

	ret = spi_mem_exec_op(nor->spi, &op);
	if (ret) {
		dev_err(nor->dev, "Failed to enable octal DTR mode\n");
		return ret;
	}
	nor->reg_proto = SNOR_PROTO_8_8_8_DTR;

	return 0;
}

static void macronix_octal_late_init(struct spi_nor *nor,
				     struct spi_nor_flash_parameter *params)
{
	nor->octal_dtr_enable = spi_nor_macronix_octal_dtr_enable;
}

static void macronix_octal_post_sfdp_fixup(struct spi_nor *nor,
					 struct spi_nor_flash_parameter *params)
{
	/*
	 * Adding SNOR_HWCAPS_PP_8_8_8_DTR in hwcaps.mask when
	 * SPI_NOR_OCTAL_DTR_READ flag exists.
	 */
	if (params->hwcaps.mask & SNOR_HWCAPS_READ_8_8_8_DTR)
		params->hwcaps.mask |= SNOR_HWCAPS_PP_8_8_8_DTR;
}

static struct spi_nor_fixups macronix_octal_fixups = {
	.post_sfdp = macronix_octal_post_sfdp_fixup,
	.late_init = macronix_octal_late_init,
};

static void macronix_set_fixups(struct spi_nor *nor)
{
	if (nor->info->flags & SPI_NOR_OCTAL_DTR_READ)
		nor->fixups = &macronix_octal_fixups;
}
#endif

const struct spi_nor_manufacturer spi_nor_macronix = {
	.parts = macronix_nor_parts,
	.nparts = ARRAY_SIZE(macronix_nor_parts),
#if !CONFIG_IS_ENABLED(SPI_FLASH_TINY)
	.set_fixups = macronix_set_fixups,
#endif
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <dm/device_compat.h>
#include <linux/mtd/spi-nor.h>
#include "sf_internal.h"

static const struct flash_info micron_nor_parts[] = {
	{ INFO("n25q016a",	 0x20bb15, 0, 64 * 1024,   32, SECT_4K | SPI_NOR_QUAD_READ) },
	{ INFO("n25q032",	 0x20ba16, 0, 64 * 1024,   64, SPI_NOR_QUAD_READ) },
	{ INFO("n25q032a",	0x20bb16, 0, 64 * 1024,   64, SPI_NOR_QUAD_READ) },
	{ INFO("n25q064",     0x20ba17, 0, 64 * 1024,  128, SECT_4K | SPI_NOR_QUAD_READ) },
	{ INFO("n25q064a",    0x20bb17, 0, 64 * 1024,  128, SECT_4K | SPI_NOR_QUAD_READ) },
	{ INFO("n25q128a11",  0x20bb18, 0, 64 * 1024,  256, SECT_4K | SPI_NOR_QUAD_READ) },
	{ INFO("n25q128a13",  0x20ba18, 0, 64 * 1024,  256, SECT_4K | SPI_NOR_QUAD_READ) },
	{ INFO6("mt25ql256a",    0x20ba19, 0x104400, 64 * 1024,  512, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES | USE_FSR) },
	{ INFO("n25q256a",    0x20ba19, 0, 64 * 1024,  512, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | USE_FSR) },
	{ INFO6("mt25qu256a",  0x20bb19, 0x104400, 64 * 1024,  512, SECT_4K | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES | USE_FSR) },
	{ INFO("n25q256ax1",  0x20bb19, 0, 64 * 1024,  512, SECT_4K | SPI_NOR_QUAD_READ | USE_FSR) },
	{ INFO("mt25qu128ab", 0x20bb18, 0, 64 * 1024,  256, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ) },
	{ INFO6("mt25qu512a",  0x20bb20, 0x104400, 64 * 1024, 1024,
		 SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES |
		 USE_FSR) },
	{ INFO("n25q512a",    0x20bb20, 0, 64 * 1024, 1024, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ) },
	{ INFO6("mt25ql512a",  0x20ba20, 0x104400, 64 * 1024, 1024, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("n25q512ax3",  0x20ba20, 0, 64 * 1024, 1024, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ) },
	{ INFO("n25q00",      0x20ba21, 0, 64 * 1024, 2048, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ | NO_CHIP_ERASE) },
	{ INFO("n25q00a",     0x20bb21, 0, 64 * 1024, 2048, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ | NO_CHIP_ERASE) },
	{ INFO("mt25ql01g",   0x21ba20, 0, 64 * 1024, 2048, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ | NO_CHIP_ERASE) },
	{ INFO6("mt25qu01g",  0x20bb21, 0x104400, 64 * 1024, 2048, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ | NO_CHIP_ERASE | SPI_NOR_4B_OPCODES) },
	{ INFO("mt25qu02g",   0x20bb22, 0, 64 * 1024, 4096, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ | NO_CHIP_ERASE) },
	{ INFO("mt25ql02g",   0x20ba22, 0, 64 * 1024, 4096, SECT_4K | USE_FSR | SPI_NOR_QUAD_READ | NO_CHIP_ERASE | SPI_NOR_4B_OPCODES) },
#ifdef CONFIG_SPI_FLASH_MT35XU
	{ INFO("mt35xl512aba", 0x2c5a1a, 0,  128 * 1024,  512, USE_FSR | SPI_NOR_OCTAL_READ | SPI_NOR_4B_OPCODES | SPI_NOR_OCTAL_DTR_READ) },
	{ INFO("mt35xu512aba", 0x2c5b1a, 0,  128 * 1024,  512, USE_FSR | SPI_NOR_OCTAL_READ | SPI_NOR_4B_OPCODES | SPI_NOR_OCTAL_DTR_READ) },
	{ INFO("mt35xu01gaba", 0x2c5b1b, 0,  128 * 1024,  1024,
		USE_FSR | SPI_NOR_OCTAL_READ | SPI_NOR_4B_OPCODES | NO_CHIP_ERASE) },
#endif /* CONFIG_SPI_FLASH_MT35XU */
	{ INFO6("mt35xu01g",  0x2c5b1b, 0x104100, 128 * 1024,  1024,
		USE_FSR | SPI_NOR_OCTAL_READ | SPI_NOR_4B_OPCODES | NO_CHIP_ERASE) },
	{ INFO("mt35xu02g",  0x2c5b1c, 0, 128 * 1024,  2048,
		USE_FSR | SPI_NOR_OCTAL_READ | SPI_NOR_4B_OPCODES | NO_CHIP_ERASE) },
};

static const struct flash_info st_nor_parts[] = {
	{ INFO("m25p10",  0x202011,  0,  32 * 1024,   4, 0) },
	{ INFO("m25p20",  0x202012,  0,  64 * 1024,   4, 0) },
	{ INFO("m25p40",  0x202013,  0,  64 * 1024,   8, 0) },
	{ INFO("m25p80",  0x202014,  0,  64 * 1024,  16, 0) },
	{ INFO("m25p16",  0x202015,  0,  64 * 1024,  32, 0) },
	{ INFO("m25p32",  0x202016,  0,  64 * 1024,  64, 0) },
	{ INFO("m25p64",  0x202017,  0,  64 * 1024, 128, 0) },
	{ INFO("m25p128", 0x202018,  0, 256 * 1024,  64, 0) },
	{ INFO("m25pe16", 0x208015,  0, 64 * 1024, 32, SECT_4K) },
	{ INFO("m25px16",    0x207115,  0, 64 * 1024, 32, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("m25px64",    0x207117,  0, 64 * 1024, 128, 0) },
};

#if !CONFIG_IS_ENABLED(SPI_FLASH_TINY) && CONFIG_IS_ENABLED(SPI_FLASH_MT35XU)
static int spi_nor_micron_octal_dtr_enable(struct spi_nor *nor)
{
	struct spi_mem_op op;
	u8 buf;
	u8 addr_width = 3;
	int ret;

	/* Set dummy cycles for Fast Read to the default of 20. */
	ret = write_enable(nor);
	if (ret)
		return ret;

	buf = 20;
	op = (struct spi_mem_op)
		SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_MT_WR_ANY_REG, 1),
			   SPI_MEM_OP_ADDR(addr_width, SPINOR_REG_MT_CFR1V, 1),
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_DATA_OUT(1, &buf, 1));
	ret = spi_mem_exec_op(nor->spi, &op);
	if (ret)
		return ret;

	ret = spi_nor_wait_till_ready(nor);
	if (ret)
		return ret;

	nor->read_dummy = 20;

	ret = write_enable(nor);
	if (ret)
		return ret;

	buf = SPINOR_MT_OCT_DTR;
	op = (struct spi_mem_op)
		SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_MT_WR_ANY_REG, 1),
			   SPI_MEM_OP_ADDR(addr_width, SPINOR_REG_MT_CFR0V, 1),
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_DATA_OUT(1, &buf, 1));
	ret = spi_mem_exec_op(nor->spi, &op);
	if (ret) {
		dev_err(nor->dev, "Failed to enable octal DTR mode\n");
		return ret;
	}

	return 0;
}

static void mt35xu512aba_late_init(struct spi_nor *nor,
				   struct spi_nor_flash_parameter *params)
{
	nor->octal_dtr_enable = spi_nor_micron_octal_dtr_enable;
}

static void mt35xu512aba_post_sfdp_fixup(struct spi_nor *nor,
					 struct spi_nor_flash_parameter *params)
{
	/* Set the Fast Read settings. */
	params->hwcaps.mask |= SNOR_HWCAPS_READ_8_8_8_DTR;
	spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_8_8_8_DTR],
				  0, 20, SPINOR_OP_MT_DTR_RD,
				  SNOR_PROTO_8_8_8_DTR);

	params->hwcaps.mask |= SNOR_HWCAPS_PP_8_8_8_DTR;

	nor->cmd_ext_type = SPI_NOR_EXT_REPEAT;
	params->rdsr_dummy = 8;
	params->rdsr_addr_nbytes = 0;

	/*
	 * SCCR Map 22nd DWORD does not indicate DTR Octal Mode Enable
	 * for MT35XU512ABA but is actually supported by device.
	 */
	nor->flags |= SNOR_F_IO_MODE_EN_VOLATILE;

	/*
	 * The BFPT quad enable field is set to a reserved value so the quad
	 * enable function is ignored by spi_nor_parse_bfpt(). Make sure we
	 * disable it.
	 */
	params->quad_enable = NULL;
}

static struct spi_nor_fixups mt35xu512aba_fixups = {
	.post_sfdp = mt35xu512aba_post_sfdp_fixup,
	.late_init = mt35xu512aba_late_init,
};

static void micron_set_fixups(struct spi_nor *nor)
{
	if (!strcmp(nor->info->name, "mt35xu512aba"))
		nor->fixups = &mt35xu512aba_fixups;
}
#endif

const struct spi_nor_manufacturer spi_nor_micron = {
	.parts = micron_nor_parts,
	.nparts = ARRAY_SIZE(micron_nor_parts),
#if !CONFIG_IS_ENABLED(SPI_FLASH_TINY) && CONFIG_IS_ENABLED(SPI_FLASH_MT35XU)
	.set_fixups = micron_set_fixups,
#endif
};

const struct spi_nor_manufacturer spi_nor_st = {
	.parts = st_nor_parts,
	.nparts = ARRAY_SIZE(st_nor_parts),
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <dm/device_compat.h>
#include <linux/sizes.h>
#include <linux/mtd/spi-nor.h>
#include "sf_internal.h"

static const struct flash_info spansion_nor_parts[] = {
	{ INFO("s25sl032p",  0x010215, 0x4d00,  64 * 1024,  64, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("s25sl064p",  0x010216, 0x4d00,  64 * 1024, 128, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO6("s25fl256s0", 0x010219, 0x4d0080, 256 * 1024, 128, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | USE_CLSR) },
	{ INFO6("s25fl256s1", 0x010219, 0x4d0180,  64 * 1024, 512, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | USE_CLSR) },
	{ INFO6("s25fl512s",  0x010220, 0x4d0080, 256 * 1024, 256, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | USE_CLSR) },
	{ INFO6("s25fs064s",  0x010217, 0x4d0181,  64 * 1024, 128, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | USE_CLSR) },
	{ INFO6("s25fs128s",  0x012018, 0x4d0181,  64 * 1024, 256, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | USE_CLSR) },
	{ INFO6("s25fs256s",  0x010219, 0x4d0181,  64 * 1024, 512, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | USE_CLSR) },
	{ INFO6("s25fs512s",  0x010220, 0x4d0081, 256 * 1024, 256, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | USE_CLSR) },
	{ INFO("s25fl512s_256k",  0x010220, 0x4d00, 256 * 1024, 256, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | USE_CLSR) },
	{ INFO("s25fl512s_64k",  0x010220, 0x4d01, 64 * 1024, 1024, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | USE_CLSR) },
	{ INFO("s25fl512s_512k", 0x010220, 0x4f00, 256 * 1024, 256, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | USE_CLSR) },
	{ INFO("s70fs01gs_256k", 0x010221, 0x4d00, 256 * 1024, 512, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("s25sl12800", 0x012018, 0x0300, 256 * 1024,  64, 0) },
	{ INFO("s25sl12801", 0x012018, 0x0301,  64 * 1024, 256, 0) },
	{ INFO6("s25fl128s",  0x012018, 0x4d0180, 64 * 1024, 256, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | USE_CLSR) },
	{ INFO("s25fl129p0", 0x012018, 0x4d00, 256 * 1024,  64, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | USE_CLSR) },
	{ INFO("s25fl129p1", 0x012018, 0x4d01,  64 * 1024, 256, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | USE_CLSR) },
	{ INFO("s25sl008a",  0x010213,      0,  64 * 1024,  16, 0) },
	{ INFO("s25sl016a",  0x010214,      0,  64 * 1024,  32, 0) },
	{ INFO("s25sl032a",  0x010215,      0,  64 * 1024,  64, 0) },
	{ INFO("s25sl064a",  0x010216,      0,  64 * 1024, 128, 0) },
	{ INFO("s25fl116k",  0x014015,      0,  64 * 1024,  32, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("s25fl164k",  0x014017,      0,  64 * 1024, 128, SECT_4K) },
	{ INFO("s25fl208k",  0x014014,      0,  64 * 1024,  16, SECT_4K | SPI_NOR_DUAL_READ) },
	{ INFO("s25fl064l",  0x016017,      0,  64 * 1024, 128, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("s25fl128l",  0x016018,      0,  64 * 1024, 256, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO("s25fl256l",  0x016019,      0,  64 * 1024, 512, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
	{ INFO6("s25hl512t",  0x342a1a, 0x0f0390, 256 * 1024, 256,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES |
		USE_CLSR) },
	{ INFO6("s25hl01gt",  0x342a1b, 0x0f0390, 256 * 1024, 512,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES |
		USE_CLSR) },
	{ INFO6("s25hl02gt",  0x342a1c, 0x0f0090, 256 * 1024, 1024,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES | NO_CHIP_ERASE) },
	{ INFO6("s25hs512t",  0x342b1a, 0x0f0390, 256 * 1024, 256,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES |
		USE_CLSR) },
	{ INFO6("s25hs01gt",  0x342b1b, 0x0f0390, 256 * 1024, 512,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES |
		USE_CLSR) },
	{ INFO6("s25hs02gt",  0x342b1c, 0x0f0090, 256 * 1024, 1024,
		SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES | NO_CHIP_ERASE) },
	{ INFO6("s25fs256t",  0x342b19, 0x0f0890, 128 * 1024, 256,
		SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
#ifdef CONFIG_SPI_FLASH_S28HX_T
	{ INFO("s28hl256t",  0x345a19,      0, 256 * 1024, 128, SPI_NOR_OCTAL_DTR_READ) },
	{ INFO("s28hl512t",  0x345a1a,      0, 256 * 1024, 256, SPI_NOR_OCTAL_DTR_READ) },
	{ INFO("s28hl01gt",  0x345a1b,      0, 256 * 1024, 512, SPI_NOR_OCTAL_DTR_READ) },
	{ INFO("s28hl02gt",  0x345a1c,      0, 256 * 1024, 1024, SPI_NOR_OCTAL_DTR_READ | NO_CHIP_ERASE) },
	{ INFO("s28hs256t",  0x345b19,      0, 256 * 1024, 128, SPI_NOR_OCTAL_DTR_READ) },
	{ INFO("s28hs512t",  0x345b1a,      0, 256 * 1024, 256, SPI_NOR_OCTAL_DTR_READ) },
	{ INFO("s28hs01gt",  0x345b1b,      0, 256 * 1024, 512, SPI_NOR_OCTAL_DTR_READ) },
	{ INFO("s28hs02gt",  0x345b1c,      0, 256 * 1024, 1024, SPI_NOR_OCTAL_DTR_READ | NO_CHIP_ERASE) },
#endif
};

#if !CONFIG_IS_ENABLED(SPI_FLASH_TINY)

/* Use ID byte 4 to distinguish S25FS256T and S25Hx-T */
#define S25FS256T_ID4	(0x08)

/* Number of dummy cycle for Read Any Register (RDAR) op. */
#define S25FS_S_RDAR_DUMMY	8

static int spansion_read_any_reg(struct spi_nor *nor, u32 addr, u8 dummy,
				 u8 *val)
{
	struct spi_mem_op op =
		SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_RD_ANY_REG, 0),
			   SPI_MEM_OP_ADDR(nor->addr_mode_nbytes, addr, 0),
			   SPI_MEM_OP_DUMMY(dummy, 0),
			   SPI_MEM_OP_DATA_IN(1, NULL, 0));
	u8 buf[2];
	int ret;

	spi_nor_setup_op(nor, &op, nor->reg_proto);

	/*
	 * In Octal DTR mode, the number of address bytes is always 4 regardless
	 * of addressing mode setting.
	 */
	if (nor->reg_proto == SNOR_PROTO_8_8_8_DTR)
		op.addr.nbytes = 4;

	/*
	 * We don't want to read only one byte in DTR mode. So, read 2 and then
	 * discard the second byte.
	 */
	if (spi_nor_protocol_is_dtr(nor->reg_proto))
		op.data.nbytes = 2;

	ret = spi_nor_read_write_reg(nor, &op, buf);
	if (ret)
		return ret;

	*val = buf[0];

	return 0;
}

static int spansion_write_any_reg(struct spi_nor *nor, u32 addr, u8 val)
{
	struct spi_mem_op op =
		SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_WR_ANY_REG, 1),
			   SPI_MEM_OP_ADDR(nor->addr_mode_nbytes, addr, 1),
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_DATA_OUT(1, NULL, 1));

	return spi_nor_read_write_reg(nor, &op, &val);
}

/**
 * spansion_quad_enable_volatile() - enable Quad I/O mode in volatile register.
 * @nor:	pointer to a 'struct spi_nor'
 * @addr_base:	base address of register (can be >0 in multi-die parts)
 * @dummy:	number of dummy cycles for register read
 *
 * It is recommended to update volatile registers in the field application due
 * to a risk of the non-volatile registers corruption by power interrupt. This
 * function sets Quad Enable bit in CFR1 volatile.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int spansion_quad_enable_volatile(struct spi_nor *nor, u32 addr_base,
					 u8 dummy)
{
	u32 addr = addr_base + SPINOR_REG_CYPRESS_CFR1V;

	u8 cr;
	int ret;

	/* Check current Quad Enable bit value. */
	ret = spansion_read_any_reg(nor, addr, dummy, &cr);
	if (ret < 0) {
		dev_dbg(nor->dev,
			"error while reading configuration register\n");
		return -EINVAL;
	}

	if (cr & CR_QUAD_EN_SPAN)
		return 0;

	cr |= CR_QUAD_EN_SPAN;

	write_enable(nor);

	ret = spansion_write_any_reg(nor, addr, cr);

	if (ret < 0) {
		dev_dbg(nor->dev,
			"error while writing configuration register\n");
		return -EINVAL;
	}

	/* Read back and check it. */
	ret = spansion_read_any_reg(nor, addr, dummy, &cr);
	if (ret || !(cr & CR_QUAD_EN_SPAN)) {
		dev_dbg(nor->dev, "Spansion Quad bit not set\n");
		return -EINVAL;
	}

	return 0;
}

/*
 * Read status register 1 by using Read Any Register command to support multi
 * die package parts.
 */
static int spansion_sr_ready(struct spi_nor *nor, u32 addr_base, u8 dummy)
{
	u32 reg_addr = addr_base + SPINOR_REG_CYPRESS_STR1V;
	u8 sr;
	int ret;

	ret = spansion_read_any_reg(nor, reg_addr, dummy, &sr);
	if (ret < 0)
		return ret;

	if (sr & (SR_E_ERR | SR_P_ERR)) {
		if (sr & SR_E_ERR)
			dev_dbg(nor->dev, "Erase Error occurred\n");
		else
			dev_dbg(nor->dev, "Programming Error occurred\n");

		nor->write_reg(nor, SPINOR_OP_CYPRESS_CLPEF, NULL, 0);
		return -EIO;
	}

	return !(sr & SR_WIP);
}

/**
 * spansion_erase_non_uniform() - erase non-uniform sectors for Spansion/Cypress
 *                                chips
 * @nor:	pointer to a 'struct spi_nor'
 * @addr:	address of the sector to erase
 * @opcode_4k:	opcode for 4K sector erase
 * @ovlsz_top:	size of overlaid portion at the top address
 * @ovlsz_btm:	size of overlaid portion at the bottom address
 *
 * Erase an address range on the nor chip that can contain 4KB sectors overlaid
 * on top and/or bottom. The appropriate erase opcode and size are chosen by
 * address to erase and size of overlaid portion.
 *
 * Return: number of bytes erased on success, -errno otherwise.
 */
static int spansion_erase_non_uniform(struct spi_nor *nor, u32 addr,
				      u8 opcode_4k, u32 ovlsz_top,
				      u32 ovlsz_btm)
{
	struct spi_mem_op op =
		SPI_MEM_OP(SPI_MEM_OP_CMD(nor->erase_opcode, 0),
			   SPI_MEM_OP_ADDR(nor->addr_width, addr, 0),
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_NO_DATA);
	struct mtd_info *mtd = &nor->mtd;
	u32 erasesize;
	int ret;

	/* 4KB sectors */
	if (op.addr.val < ovlsz_btm ||
	    op.addr.val >= mtd->size - ovlsz_top) {
		op.cmd.opcode = opcode_4k;
		erasesize = SZ_4K;

	/* Non-overlaid portion in the normal sector at the bottom */
	} else if (op.addr.val == ovlsz_btm) {
		op.cmd.opcode = nor->erase_opcode;
		erasesize = mtd->erasesize - ovlsz_btm;

	/* Non-overlaid portion in the normal sector at the top */
	} else if (op.addr.val == mtd->size - mtd->erasesize) {
		op.cmd.opcode = nor->erase_opcode;
		erasesize = mtd->erasesize - ovlsz_top;

	/* Normal sectors */
	} else {
		op.cmd.opcode = nor->erase_opcode;
		erasesize = mtd->erasesize;
	}

	spi_nor_setup_op(nor, &op, nor->write_proto);

	ret = spi_mem_exec_op(nor->spi, &op);
	if (ret)
		return ret;

	return erasesize;
}

static int s25fs_s_quad_enable(struct spi_nor *nor)
{
	return spansion_quad_enable_volatile(nor, 0, S25FS_S_RDAR_DUMMY);
}

static int s25fs_s_erase_non_uniform(struct spi_nor *nor, loff_t addr)
{
	u8 opcode = nor->addr_width == 4 ? SPINOR_OP_BE_4K_4B : SPINOR_OP_BE_4K;

	/* Support 8 x 4KB sectors at bottom */
	return spansion_erase_non_uniform(nor, addr, opcode, 0, SZ_32K);
}

static int s25fs_s_setup(struct spi_nor *nor, const struct flash_info *info,
			 const struct spi_nor_flash_parameter *params)
{
	int ret;
	u8 cfr3v;

	/* Bank Address Register is not supported */
	if (CONFIG_IS_ENABLED(SPI_FLASH_BAR))
		return -EOPNOTSUPP;

	/*
	 * Read CR3V to check if uniform sector is selected. If not, assign an
	 * erase hook that supports non-uniform erase.
	 */
	ret = spansion_read_any_reg(nor, SPINOR_REG_CYPRESS_CFR3V,
				    S25FS_S_RDAR_DUMMY, &cfr3v);
	if (ret)
		return ret;
	if (!(cfr3v & SPINOR_REG_CYPRESS_CFR3_UNISECT))
		nor->erase = s25fs_s_erase_non_uniform;

	return spi_nor_default_setup(nor, info, params);
}

static void s25fs_s_late_init(struct spi_nor *nor,
			      struct spi_nor_flash_parameter *params)
{
	nor->setup = s25fs_s_setup;
}

static int s25fs_s_post_bfpt_fixup(struct spi_nor *nor,
				   const struct sfdp_parameter_header *header,
				   const struct sfdp_bfpt *bfpt,
				   struct spi_nor_flash_parameter *params)
{
	/* The erase size is set to 4K from BFPT, but it's wrong. Fix it. */
	nor->erase_opcode = SPINOR_OP_SE;
	nor->mtd.erasesize = nor->info->sector_size;

	/* The S25FS-S chip family reports 512-byte pages in BFPT but
	 * in reality the write buffer still wraps at the safe default
	 * of 256 bytes.  Overwrite the page size advertised by BFPT
	 * to get the writes working.
	 */
	params->page_size = 256;

	return 0;
}

static void s25fs_s_post_sfdp_fixup(struct spi_nor *nor,
				    struct spi_nor_flash_parameter *params)
{
	/*
	 * The S25FS064S(8MB) supports 1-1-2 and 1-1-4 commands, but params for
	 * read ops in SFDP are wrong. The other density parts do not support
	 * 1-1-2 and 1-1-4 commands.
	 */
	if (params->size == SZ_8M) {
		spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_1_1_2],
					  0, 8, SPINOR_OP_READ_1_1_2,
					  SNOR_PROTO_1_1_2);
		spi_nor_set_read_settings(&params->reads[SNOR_CMD_READ_1_1_4],
					  0, 8, SPINOR_OP_READ_1_1_4,
					  SNOR_PROTO_1_1_4);
	} else {
		params->hwcaps.mask &= ~SNOR_HWCAPS_READ_1_1_2;
		params->hwcaps.mask &= ~SNOR_HWCAPS_READ_1_1_4;
		params->hwcaps.mask &= ~SNOR_HWCAPS_PP_1_1_4;
	}

	/* Use volatile register to enable quad */
	params->quad_enable = s25fs_s_quad_enable;
}

static struct spi_nor_fixups s25fs_s_fixups = {
	.post_bfpt = s25fs_s_post_bfpt_fixup,
	.post_sfdp = s25fs_s_post_sfdp_fixup,
	.late_init = s25fs_s_late_init,
};

static int s25_s28_mdp_ready(struct spi_nor *nor)
{
	u32 addr;
	int ret;

	for (addr = 0; addr < nor->mtd.size; addr += SZ_128M) {
		ret = spansion_sr_ready(nor, addr, nor->rdsr_dummy);
		if (ret < 0)
			return ret;
		else if (ret == 0)
			return 0;
	}

	return 1;
}

static int s25_quad_enable(struct spi_nor *nor)
{
	u32 addr;
	int ret;

	for (addr = 0; addr < nor->mtd.size; addr += SZ_128M) {
		ret = spansion_quad_enable_volatile(nor, addr, 0);
		if (ret)
			return ret;
	}

	return 0;
}

static int s25_s28_erase_non_uniform(struct spi_nor *nor, loff_t addr)
{
	/* Support 32 x 4KB sectors at bottom */
	return spansion_erase_non_uniform(nor, addr, SPINOR_OP_BE_4K_4B, 0,
					  SZ_128K);
}

static int s25_s28_setup(struct spi_nor *nor, const struct flash_info *info,
			 const struct spi_nor_flash_parameter *params)
{
	int ret;
	u8 cr;

#if CONFIG_IS_ENABLED(SPI_FLASH_BAR)
	return -ENOTSUPP; /* Bank Address Register is not supported */
#endif
	/*
	 * S25FS256T has multiple sector architecture options, with selection of
	 * count and location of 128KB and 64KB sectors. This driver supports
	 * uniform 128KB only due to complexity of non-uniform layout.
	 */
	if (nor->info->id[4] == S25FS256T_ID4) {
		ret = spansion_read_any_reg(nor, SPINOR_REG_CYPRESS_ARCFN, 8,
					    &cr);
		if (ret)
			return ret;

		if (cr) /* Option 0 (ARCFN[7:0] == 0x00) is uniform */
			return -EOPNOTSUPP;

		return spi_nor_default_setup(nor, info, params);
	}

	/*
	 * Read CFR3V to check if uniform sector is selected. If not, assign an
	 * erase hook that supports non-uniform erase.
	 */
	ret = spansion_read_any_reg(nor, SPINOR_REG_CYPRESS_CFR3V, 0, &cr);
	if (ret)
		return ret;
	if (!(cr & SPINOR_REG_CYPRESS_CFR3_UNISECT))
		nor->erase = s25_s28_erase_non_uniform;

	/*
	 * For the multi-die package parts, the ready() hook is needed to check
	 * all dies' status via read any register.
	 */
	if (nor->mtd.size > SZ_128M)
		nor->ready = s25_s28_mdp_ready;

	return spi_nor_default_setup(nor, info, params);
}

static void s25_late_init(struct spi_nor *nor,
			  struct spi_nor_flash_parameter *params)
{
	nor->setup = s25_s28_setup;

	/*
	 * Programming is supported only in 16-byte ECC data unit granularity.
	 * Byte-programming, bit-walking, or multiple program operations to the
	 * same ECC data unit without an erase are not allowed.
	 */
	params->writesize = 16;
}

static int s25_s28_post_bfpt_fixup(struct spi_nor *nor,
				   const struct sfdp_parameter_header *header,
				   const struct sfdp_bfpt *bfpt,
				   struct spi_nor_flash_parameter *params)
{
	int ret;
	u32 addr;
	u8 cfr3v;

	/* erase size in case it is set to 4K from BFPT */
	nor->erase_opcode = SPINOR_OP_SE_4B;
	nor->mtd.erasesize = nor->info->sector_size;

	/*
	 * The default address mode in multi-die package parts (>1Gb) may be
	 * 3- or 4-byte, depending on model number. BootROM code in some SoCs
	 * use 3-byte mode for backward compatibility and should switch to
	 * 4-byte mode after BootROM phase. Since registers in the 2nd die are
	 * mapped within 32-bit address space, we need to make sure the flash is
	 * in 4-byte address mode. The default address mode can be distinguished
	 * by BFPT 16th DWORD. Power cycle exits 4-byte address mode if default
	 * is 3-byte address mode.
	 */
	if (params->size > SZ_128M) {
		if (bfpt->dwords[BFPT_DWORD(16)] & BFPT_DWORD16_EX4B_PWRCYC) {
			ret = set_4byte(nor, nor->info, 1);
			if (ret)
				return ret;
		}
		nor->addr_mode_nbytes = 4;
	}

	/* The default address mode in S25FS256T is 4. */
	if (nor->info->id[4] == S25FS256T_ID4)
		nor->addr_mode_nbytes = 4;

	/*
	 * The page_size is set to 512B from BFPT, but it actually depends on
	 * the configuration register. Look up the CFR3V and determine the
	 * page_size. For multi-die package parts, use 512B only when the all
	 * dies are configured to 512B buffer.
	 */
	for (addr = 0; addr < params->size; addr += SZ_128M) {
		ret = spansion_read_any_reg(nor,
					    addr + SPINOR_REG_CYPRESS_CFR3V, 0,
					    &cfr3v);
		if (ret)
			return ret;

		if (!(cfr3v & SPINOR_REG_CYPRESS_CFR3_PGSZ)) {
			params->page_size = 256;
			return 0;
		}
	}
	params->page_size = 512;

	return 0;
}

static void s25_post_sfdp_fixup(struct spi_nor *nor,
				struct spi_nor_flash_parameter *params)
{
	if (nor->info->id[4] == S25FS256T_ID4) {
		/* PP_1_1_4 is supported */
		params->hwcaps.mask |= SNOR_HWCAPS_PP_1_1_4;
	} else {
		/* READ_FAST_4B (0Ch) requires mode cycles*/
		params->reads[SNOR_CMD_READ_FAST].num_mode_clocks = 8;
		/* PP_1_1_4 is not supported */
		params->hwcaps.mask &= ~SNOR_HWCAPS_PP_1_1_4;
		/* Use volatile register to enable quad */
		params->quad_enable = s25_quad_enable;
	}
}

static struct spi_nor_fixups s25_fixups = {
	.post_bfpt = s25_s28_post_bfpt_fixup,
	.post_sfdp = s25_post_sfdp_fixup,
	.late_init = s25_late_init,
};

static int s25fl256l_setup(struct spi_nor *nor, const struct flash_info *info,
			   const struct spi_nor_flash_parameter *params)
{
	return -ENOTSUPP; /* Bank Address Register is not supported */
}

static void s25fl256l_late_init(struct spi_nor *nor,
				struct spi_nor_flash_parameter *params)
{
	nor->setup = s25fl256l_setup;
}

static struct spi_nor_fixups s25fl256l_fixups = {
	.late_init = s25fl256l_late_init,
};

#ifdef CONFIG_SPI_FLASH_S28HX_T
/**
 * spi_nor_cypress_octal_dtr_enable() - Enable octal DTR on Cypress flashes.
 * @nor:		pointer to a 'struct spi_nor'
 *
 * This also sets the memory access latency cycles to 24 to allow the flash to
 * run at up to 200MHz.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int spi_nor_cypress_octal_dtr_enable(struct spi_nor *nor)
{
	u32 addr;
	u8 buf;
	int ret;

	ret = write_enable(nor);
	if (ret)
		return ret;

	/* Use 24 dummy cycles for memory array reads. */
	for (addr = 0; addr < nor->mtd.size; addr += SZ_128M) {
		ret = spansion_read_any_reg(nor,
					    addr + SPINOR_REG_CYPRESS_CFR2V, 0,
					    &buf);
		if (ret)
			return ret;

		buf &= ~SPINOR_REG_CYPRESS_CFR2_MEMLAT_MASK;
		buf |= SPINOR_REG_CYPRESS_CFR2_MEMLAT_11_24;
		ret = spansion_write_any_reg(nor,
					     addr + SPINOR_REG_CYPRESS_CFR2V,
					     buf);
		if (ret) {
			dev_warn(nor->dev, "failed to set default memory latency value: %d\n", ret);
			return ret;
		}
	}
	nor->read_dummy = 24;

	ret = write_enable(nor);
	if (ret)
		return ret;

	/* Set the octal and DTR enable bits. */
	buf = SPINOR_REG_CYPRESS_CFR5_OCT_DTR_EN;
	for (addr = 0; addr < nor->mtd.size; addr += SZ_128M) {
		ret = spansion_write_any_reg(nor,
					     addr + SPINOR_REG_CYPRESS_CFR5V,
					     buf);
		if (ret) {
			dev_warn(nor->dev, "Failed to enable octal DTR mode\n");
			return ret;
		}
	}

	return 0;
}

static void s28hx_t_late_init(struct spi_nor *nor,
			      struct spi_nor_flash_parameter *params)
{
	nor->octal_dtr_enable = spi_nor_cypress_octal_dtr_enable;
	nor->setup = s25_s28_setup;

	/*
	 * Programming is supported only in 16-byte ECC data unit granularity.
	 * Byte-programming, bit-walking, or multiple program operations to the
	 * same ECC data unit without an erase are not allowed.
	 */
	params->writesize = 16;
}

static void s28hx_t_post_sfdp_fixup(struct spi_nor *nor,
				    struct spi_nor_flash_parameter *params)
{
	/*
	 * On older versions of the flash the xSPI Profile 1.0 table has the
	 * 8D-8D-8D Fast Read opcode as 0x00. But it actually should be 0xEE.
	 */
	if (params->reads[SNOR_CMD_READ_8_8_8_DTR].opcode == 0)
		params->reads[SNOR_CMD_READ_8_8_8_DTR].opcode =
			SPINOR_OP_CYPRESS_RD_FAST;

	params->hwcaps.mask |= SNOR_HWCAPS_PP_8_8_8_DTR;

	/* This flash is also missing the 4-byte Page Program opcode bit. */
	spi_nor_set_pp_settings(&params->page_programs[SNOR_CMD_PP],
				SPINOR_OP_PP_4B, SNOR_PROTO_1_1_1);
	/*
	 * Since xSPI Page Program opcode is backward compatible with
	 * Legacy SPI, use Legacy SPI opcode there as well.
	 */
	spi_nor_set_pp_settings(&params->page_programs[SNOR_CMD_PP_8_8_8_DTR],
				SPINOR_OP_PP_4B, SNOR_PROTO_8_8_8_DTR);

	/*
	 * The xSPI Profile 1.0 table advertises the number of additional
	 * address bytes needed for Read Status Register command as 0 but the
	 * actual value for that is 4.
	 */
	params->rdsr_addr_nbytes = 4;
}

static struct spi_nor_fixups s28hx_t_fixups = {
	.post_sfdp = s28hx_t_post_sfdp_fixup,
	.post_bfpt = s25_s28_post_bfpt_fixup,
	.late_init = s28hx_t_late_init,
};
#endif /* CONFIG_SPI_FLASH_S28HX_T */

static void spansion_set_fixups(struct spi_nor *nor)
{
	if (JEDEC_MFR(nor->info) == SNOR_MFR_CYPRESS) {
		switch (nor->info->id[1]) {
		case 0x2a: /* S25HL (QSPI, 3.3V) */
		case 0x2b: /* S25HS (QSPI, 1.8V) */
			nor->fixups = &s25_fixups;
			break;

#ifdef CONFIG_SPI_FLASH_S28HX_T
		case 0x5a: /* S28HL (Octal, 3.3V) */
		case 0x5b: /* S28HS (Octal, 1.8V) */
			nor->fixups = &s28hx_t_fixups;
			break;
#endif

		default:
			break;
		}
	}

	if (CONFIG_IS_ENABLED(SPI_FLASH_BAR) &&
	    !strcmp(nor->info->name, "s25fl256l"))
		nor->fixups = &s25fl256l_fixups;

	/* For FS-S (family ID = 0x81)  */
	if (JEDEC_MFR(nor->info) == SNOR_MFR_SPANSION && nor->info->id[5] == 0x81)
		nor->fixups = &s25fs_s_fixups;
}
#endif /* !CONFIG_IS_ENABLED(SPI_FLASH_TINY) */

const struct spi_nor_manufacturer spi_nor_spansion = {
	.parts = spansion_nor_parts,
	.nparts = ARRAY_SIZE(spansion_nor_parts),
#if !CONFIG_IS_ENABLED(SPI_FLASH_TINY)
	.set_fixups = spansion_set_fixups,
#endif
};

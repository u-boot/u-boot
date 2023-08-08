/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:       GPL-2.0+
 * https://spdx.org/licenses
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <errno.h>
#include <mvebu/fuse-mvebu.h>

#define GET_LEN(width)  DIV_ROUND_UP(width, 32)
#define ROW_WORDS_LEN	8

static int valid_prog_words;
static u32 prog_val[ROW_WORDS_LEN];

int mvebu_efuse_ld_read(struct udevice *dev, int row_id, u32 *val)
{
	void __iomem *otp_mem, *ctrl_reg;
	int row_base, i, row_widths;
	struct mvebu_fuse_block_data *priv = dev_get_priv(dev);

	row_base = priv->row_base;
	otp_mem = priv->target_otp_mem + (row_id - row_base) *
		   priv->pdata->row_step;
	ctrl_reg = priv->control_reg;
	row_widths = priv->pdata->row_bit_width;

	/* when read_only flag is set, which means read LD0 */
	if (device_is_compatible(dev, "marvell,mvebu-fuse-ld-user"))
		clrbits_le32(ctrl_reg, MVEBU_EFUSE_SRV_CTRL_LD_SEL_USER);
	else
		setbits_le32(ctrl_reg, MVEBU_EFUSE_SRV_CTRL_LD_SEL_USER);

	for (i = 0; i < GET_LEN(row_widths); i++)
		*(val + i) = readl(otp_mem + 4 * i);

	return 0;
}

int do_mvebu_efuse_ld_prog(struct udevice *dev, int row_id, u32 *new_val)
{
	void __iomem *otp_mem, *ctrl_reg;
	int row_base, i, row_widths;
	struct mvebu_fuse_block_data *priv = dev_get_priv(dev);
	u32 fuse_read_value[ROW_WORDS_LEN];

	row_base = priv->row_base;
	otp_mem = priv->target_otp_mem + (row_id - row_base) *
		   priv->pdata->row_step;
	ctrl_reg = priv->control_reg;
	row_widths = priv->pdata->row_bit_width;

	if (device_is_compatible(dev, "marvell,mvebu-fuse-ld-user")) {
		printf("This efuse row is LD0 and read-only\n");
		return -EINVAL;
	}
	/* select LD1 for fuse burn */
	setbits_le32(ctrl_reg, MVEBU_EFUSE_SRV_CTRL_LD_SEL_USER);

	/* enable fuse prog */
	setbits_le32(ctrl_reg, MVEBU_EFUSE_CTRL_PROGRAM_ENABLE);

	/* enable security bit to lock LD efuse row for further programming */
	setbits_le32(ctrl_reg, MVEBU_EFUSE_CTRL_LD_SEC_EN_MASK);
	/* read fuse row value before burn fuse */
	for (i = 0; i < GET_LEN(row_widths); i++)
		fuse_read_value[i] = readl(otp_mem + 4 * i);
	/* fuse row value burn */
	for (i = 0; i < GET_LEN(row_widths); i++) {
		fuse_read_value[i] |= *(new_val + i);
		writel(fuse_read_value[i], otp_mem + 4 * i);
	}

	/* write all 0 for LD eFuse to burn efuse */
	writel(0x0, otp_mem + 0x20);

	/* wait 1 ms for burn efuse */
	mdelay(1);

	/* Disable efuse write */
	clrbits_le32(ctrl_reg, MVEBU_EFUSE_CTRL_PROGRAM_ENABLE);

	return 0;
}

int mvebu_efuse_ld_prog(struct udevice *dev, int word, int row_id, u32 new_val)
{
	int res = 0;

#ifdef EFUSE_READ_ONLY
	dev_err(&dev->dev, "ERROR: fuse programming disabled!\n");
	return -EPERM;
#endif

	if (word < ROW_WORDS_LEN - 1) {
		prog_val[word] = new_val;
		valid_prog_words |= (1 << word);
	} else if ((valid_prog_words & 0x7F) != 0x7F) {
		res = -EINVAL;
	} else {
		prog_val[ROW_WORDS_LEN - 1] = new_val;
		res = do_mvebu_efuse_ld_prog(dev, row_id, prog_val);
		valid_prog_words = 0;
	}

	return res;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <errno.h>
#include <mvebu/fuse-mvebu.h>

#define GET_LEN(width)  DIV_ROUND_UP(width, 32)
#define ROW_WORDS_LEN	3
#define ECC_BITS_MASK	0xfe000000

static int valid_prog_words;
static u32 prog_val[ROW_WORDS_LEN];

int mvebu_efuse_hd_read(struct udevice *dev, int row_id, u32 *val)
{
	void __iomem *otp_mem;
	int row_base, i, row_widths;
	struct mvebu_fuse_block_data *priv = dev_get_priv(dev);

	row_base = priv->row_base;
	otp_mem = priv->target_otp_mem + (row_id - row_base) *
		   priv->pdata->row_step;
	row_widths = priv->pdata->row_bit_width;

	for (i = 0; i < GET_LEN(row_widths); i++)
		*(val + i) = readl(otp_mem + 4 * i);

	return 0;
}

#define EFUSE_MAX_RETRIES	3

static int do_mvebu_efuse_hd_prog(struct udevice *dev, int row_id, u32 *new_val)
{
	void __iomem *otp_mem, *ctrl_reg;
	int row_base, i, row_widths;
	struct mvebu_fuse_block_data *priv = dev_get_priv(dev);
	u32 fuse_read_value[ROW_WORDS_LEN];
	int retry_cnt = 0, err_flag = 0;

	row_base = priv->row_base;
	otp_mem = priv->target_otp_mem + (row_id - row_base) *
		   priv->pdata->row_step;
	ctrl_reg = priv->control_reg;
	row_widths = priv->pdata->row_bit_width;

	/* only write a fuse line with lock bit */
	if (!(*(new_val + 2) & 0x1))
		return -EINVAL;
	/* according to specs ECC protection bits must be 0 on write */
	if (*(new_val + 1) & ECC_BITS_MASK)
		return -EINVAL;

	do {
		err_flag = 0;

		/* enable fuse prog */
		setbits_le32(ctrl_reg, MVEBU_EFUSE_CTRL_PROGRAM_ENABLE);

		/* read fuse row value before burn fuse */
		for (i = 0; i < GET_LEN(row_widths); i++)
			fuse_read_value[i] = readl(otp_mem + 4 * i);

		/* fuse row value burn */
		for (i = 0; i < GET_LEN(row_widths); i++) {
			fuse_read_value[i] |= *(new_val + i);
			writel(fuse_read_value[i], otp_mem + 4 * i);
		}

		/* wait 1 ms for burn efuse */
		mdelay(1);

		/* Disable efuse write */
		clrbits_le32(ctrl_reg, MVEBU_EFUSE_CTRL_PROGRAM_ENABLE);

		/* ERRATA: 5073
		 * When burning a row in the HD eFuse, the SM first read the
		 * row to be burned to check it wasnt already burned, to prevent
		 * re-buring. However, this errata causes the first HD burn
		 * to read row zero instead of the targeted row, causing the
		 * write to fail even if its empty. (The row zero is always
		 * programmed in the production chips)
		 *
		 * The workaround is to detect failure by read and compare
		 * and retry the fuse burn again (for max_retries = 3)
		 */

		/* Read & compare new read values with expected values */
		for (i = 0; i < GET_LEN(row_widths); i++)
			fuse_read_value[i] = readl(otp_mem + 4 * i);

		for (i = 0; i < GET_LEN(row_widths); i++) {
			dev_dbg(&dev->dev, "Expected value: 0x%x\n",
				*(new_val + i));
			/* Bank 1 has ECC bits which should
			 * be masked while comparing
			 */
			if (i == 1)
				fuse_read_value[i] &= ~ECC_BITS_MASK;
			dev_dbg(&dev->dev, "Read value: 0x%x\n",
				fuse_read_value[i]);
			/* If comparison fails,
			 * set error flag to initiate retry
			 */
			if (fuse_read_value[i] != *(new_val + i)) {
				dev_dbg(&dev->dev, "Fuse prog failed\n");
				err_flag = 1;
				retry_cnt++;
				dev_dbg(&dev->dev, "Retrying fuse prog..\n");
			}
		}
	} while ((err_flag != 0) && (retry_cnt < EFUSE_MAX_RETRIES));

	/* If we exceed retries limit, report error */
	if (err_flag != 0) {
		dev_err(&dev->dev, "fuse prog failed after %d retries\n",
			retry_cnt);
		return -EIO;
	}
	return 0;
}

int mvebu_efuse_hd_prog(struct udevice *dev, int word, int row_id, u32 new_val)
{
	int res = 0;

#ifdef EFUSE_READ_ONLY
	dev_err(&dev->dev, "ERROR: fuse programming disabled!\n");
	return -EPERM;
#endif

	if (word < ROW_WORDS_LEN - 1) {
		prog_val[word] = new_val;
		valid_prog_words |= (1 << word);
	}

	if (word == ROW_WORDS_LEN - 1) {
		if ((valid_prog_words & 3) == 0 && new_val) {
			prog_val[0] = 0;
			prog_val[1] = 0;
			/* lock bit is set to 1*/
			prog_val[2] = 1;
			res = do_mvebu_efuse_hd_prog(dev, row_id, prog_val);
			valid_prog_words = 0;
		} else if ((valid_prog_words & 3) == 3 && new_val) {
			prog_val[ROW_WORDS_LEN - 1] = new_val;
			res = do_mvebu_efuse_hd_prog(dev, row_id, prog_val);
			valid_prog_words = 0;
		} else {
			res = -EINVAL;
			valid_prog_words = 0;
		}
	}

	return res;
}

/*
 * FSL UPM NAND driver
 *
 * Copyright (C) 2007 MontaVista Software, Inc.
 *                    Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <config.h>

#if defined(CONFIG_CMD_NAND) && defined(CONFIG_NAND_FSL_UPM)
#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/fsl_upm.h>
#include <nand.h>

static int fsl_upm_in_pattern;

static void fsl_upm_start_pattern(struct fsl_upm *upm, u32 pat_offset)
{
	clrsetbits_be32(upm->mxmr, MxMR_MAD_MSK, MxMR_OP_RUNP | pat_offset);
}

static void fsl_upm_end_pattern(struct fsl_upm *upm)
{
	clrbits_be32(upm->mxmr, MxMR_OP_RUNP);

	while (in_be32(upm->mxmr) & MxMR_OP_RUNP)
		eieio();
}

static void fsl_upm_run_pattern(struct fsl_upm *upm, int width, u32 cmd)
{
	out_be32(upm->mar, cmd << (32 - width));
	switch (width) {
	case 8:
		out_8(upm->io_addr, 0x0);
		break;
	case 16:
		out_be16(upm->io_addr, 0x0);
		break;
	case 32:
		out_be32(upm->io_addr, 0x0);
		break;
	}
}

static void nand_hwcontrol (struct mtd_info *mtd, int cmd)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_upm_nand *fun = chip->priv;

	switch (cmd) {
	case NAND_CTL_SETCLE:
		fsl_upm_start_pattern(&fun->upm, fun->upm_cmd_offset);
		fsl_upm_in_pattern++;
		break;
	case NAND_CTL_SETALE:
		fsl_upm_start_pattern(&fun->upm, fun->upm_addr_offset);
		fsl_upm_in_pattern++;
		break;
	case NAND_CTL_CLRCLE:
	case NAND_CTL_CLRALE:
		fsl_upm_end_pattern(&fun->upm);
		fsl_upm_in_pattern--;
		break;
	}
}

static void nand_write_byte(struct mtd_info *mtd, u_char byte)
{
	struct nand_chip *chip = mtd->priv;

	if (fsl_upm_in_pattern) {
		struct fsl_upm_nand *fun = chip->priv;

		fsl_upm_run_pattern(&fun->upm, fun->width, byte);

		/*
		 * Some boards/chips needs this. At least on MPC8360E-RDK we
		 * need it. Probably weird chip, because I don't see any need
		 * for this on MPC8555E + Samsung K9F1G08U0A. Usually here are
		 * 0-2 unexpected busy states per block read.
		 */
		if (fun->wait_pattern) {
			while (!fun->dev_ready())
				debug("unexpected busy state\n");
		}
	} else {
		out_8(chip->IO_ADDR_W, byte);
	}
}

static u8 nand_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;

	return in_8(chip->IO_ADDR_R);
}

static void nand_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	int i;
	struct nand_chip *chip = mtd->priv;

	for (i = 0; i < len; i++)
		out_8(chip->IO_ADDR_W, buf[i]);
}

static void nand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	int i;
	struct nand_chip *chip = mtd->priv;

	for (i = 0; i < len; i++)
		buf[i] = in_8(chip->IO_ADDR_R);
}

static int nand_verify_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	int i;
	struct nand_chip *chip = mtd->priv;

	for (i = 0; i < len; i++) {
		if (buf[i] != in_8(chip->IO_ADDR_R))
			return -EFAULT;
	}

	return 0;
}

static int nand_dev_ready(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_upm_nand *fun = chip->priv;

	return fun->dev_ready();
}

int fsl_upm_nand_init(struct nand_chip *chip, struct fsl_upm_nand *fun)
{
	if (fun->width != 8 && fun->width != 16 && fun->width != 32)
		return -ENOSYS;

	chip->priv = fun;
	chip->chip_delay = fun->chip_delay;
	chip->eccmode = NAND_ECC_SOFT;
	chip->hwcontrol = nand_hwcontrol;
	chip->read_byte = nand_read_byte;
	chip->read_buf = nand_read_buf;
	chip->write_byte = nand_write_byte;
	chip->write_buf = nand_write_buf;
	chip->verify_buf = nand_verify_buf;
	if (fun->dev_ready)
		chip->dev_ready = nand_dev_ready;

	return 0;
}
#endif /* CONFIG_CMD_NAND */

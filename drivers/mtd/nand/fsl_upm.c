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

#define FSL_UPM_MxMR_OP_NO (0 << 28) /* normal operation */
#define FSL_UPM_MxMR_OP_WA (1 << 28) /* write array */
#define FSL_UPM_MxMR_OP_RA (2 << 28) /* read array */
#define FSL_UPM_MxMR_OP_RP (3 << 28) /* run pattern */

static void fsl_upm_start_pattern(struct fsl_upm *upm, u32 pat_offset)
{
	out_be32(upm->mxmr, FSL_UPM_MxMR_OP_RP | pat_offset);
}

static void fsl_upm_end_pattern(struct fsl_upm *upm)
{
	out_be32(upm->mxmr, FSL_UPM_MxMR_OP_NO);
	while (in_be32(upm->mxmr) != FSL_UPM_MxMR_OP_NO)
		eieio();
}

static void fsl_upm_run_pattern(struct fsl_upm *upm, int width, u32 cmd)
{
	out_be32(upm->mar, cmd << (32 - width * 8));
	out_8(upm->io_addr, 0x0);
}

static void fsl_upm_setup(struct fsl_upm *upm)
{
	int i;

	/* write upm array */
	out_be32(upm->mxmr, FSL_UPM_MxMR_OP_WA);

	for (i = 0; i < 64; i++) {
		out_be32(upm->mdr, upm->array[i]);
		out_8(upm->io_addr, 0x0);
	}

	/* normal operation */
	out_be32(upm->mxmr, FSL_UPM_MxMR_OP_NO);
	while (in_be32(upm->mxmr) != FSL_UPM_MxMR_OP_NO)
		eieio();
}

static void fun_cmdfunc(struct mtd_info *mtd, unsigned command, int column,
			int page_addr)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_upm_nand *fun = chip->priv;

	fsl_upm_start_pattern(&fun->upm, fun->upm_cmd_offset);

	if (command == NAND_CMD_SEQIN) {
		int readcmd;

		if (column >= mtd->oobblock) {
			/* OOB area */
			column -= mtd->oobblock;
			readcmd = NAND_CMD_READOOB;
		} else if (column < 256) {
			/* First 256 bytes --> READ0 */
			readcmd = NAND_CMD_READ0;
		} else {
			column -= 256;
			readcmd = NAND_CMD_READ1;
		}
		fsl_upm_run_pattern(&fun->upm, fun->width, readcmd);
	}

	fsl_upm_run_pattern(&fun->upm, fun->width, command);

	fsl_upm_end_pattern(&fun->upm);

	fsl_upm_start_pattern(&fun->upm, fun->upm_addr_offset);

	if (column != -1)
		fsl_upm_run_pattern(&fun->upm, fun->width, column);

	if (page_addr != -1) {
		fsl_upm_run_pattern(&fun->upm, fun->width, page_addr);
		fsl_upm_run_pattern(&fun->upm, fun->width,
				    (page_addr >> 8) & 0xFF);
		if (chip->chipsize > (32 << 20)) {
			fsl_upm_run_pattern(&fun->upm, fun->width,
					    (page_addr >> 16) & 0x0f);
		}
	}

	fsl_upm_end_pattern(&fun->upm);

	if (fun->wait_pattern) {
		/*
		 * Some boards/chips needs this. At least on MPC8360E-RDK we
		 * need it. Probably weird chip, because I don't see any need
		 * for this on MPC8555E + Samsung K9F1G08U0A. Usually here are
		 * 0-2 unexpected busy states per block read.
		 */
		while (!fun->dev_ready())
			debug("unexpected busy state\n");
	}
}

static void nand_write_byte(struct mtd_info *mtd, u_char byte)
{
	struct nand_chip *chip = mtd->priv;

	out_8(chip->IO_ADDR_W, byte);
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

static void nand_hwcontrol(struct mtd_info *mtd, int cmd)
{
}

static int nand_dev_ready(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_upm_nand *fun = chip->priv;

	return fun->dev_ready();
}

int fsl_upm_nand_init(struct nand_chip *chip, struct fsl_upm_nand *fun)
{
	/* yet only 8 bit accessors implemented */
	if (fun->width != 1)
		return -ENOSYS;

	fsl_upm_setup(&fun->upm);

	chip->priv = fun;
	chip->chip_delay = fun->chip_delay;
	chip->eccmode = NAND_ECC_SOFT;
	chip->cmdfunc = fun_cmdfunc;
	chip->hwcontrol = nand_hwcontrol;
	chip->read_byte = nand_read_byte;
	chip->read_buf = nand_read_buf;
	chip->write_byte = nand_write_byte;
	chip->write_buf = nand_write_buf;
	chip->verify_buf = nand_verify_buf;
	chip->dev_ready = nand_dev_ready;

	return 0;
}
#endif /* CONFIG_CMD_NAND */

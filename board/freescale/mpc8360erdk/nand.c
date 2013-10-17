/*
 * MPC8360E-RDK support for the NAND on FSL UPM
 *
 * Copyright (C) 2007 MontaVista Software, Inc.
 *                    Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/immap_83xx.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/fsl_upm.h>
#include <nand.h>

static struct immap *im = (struct immap *)CONFIG_SYS_IMMR;

static const u32 upm_array[] = {
	0x0ff03c30, 0x0ff03c30, 0x0ff03c34, 0x0ff33c30, /* Words  0 to  3 */
	0xfff33c31, 0xfffffc30, 0xfffffc30, 0xfffffc30, /* Words  4 to  7 */
	0x0faf3c30, 0x0faf3c30, 0x0faf3c30, 0x0fff3c34, /* Words  8 to 11 */
	0xffff3c31, 0xfffffc30, 0xfffffc30, 0xfffffc30, /* Words 12 to 15 */
	0x0fa3fc30, 0x0fa3fc30, 0x0fa3fc30, 0x0ff3fc34, /* Words 16 to 19 */
	0xfff3fc31, 0xfffffc30, 0xfffffc30, 0xfffffc30, /* Words 20 to 23 */
	0x0ff33c30, 0x0fa33c30, 0x0fa33c34, 0x0ff33c30, /* Words 24 to 27 */
	0xfff33c31, 0xfff0fc30, 0xfff0fc30, 0xfff0fc30, /* Words 28 to 31 */
	0xfff3fc30, 0xfff3fc30, 0xfff6fc30, 0xfffcfc30, /* Words 32 to 35 */
	0xfffcfc30, 0xfffcfc30, 0xfffcfc30, 0xfffcfc30, /* Words 36 to 39 */
	0xfffcfc30, 0xfffcfc30, 0xfffcfc30, 0xfffcfc30, /* Words 40 to 43 */
	0xfffdfc30, 0xfffffc30, 0xfffffc30, 0xfffffc31, /* Words 44 to 47 */
	0xfffffc30, 0xfffffc00, 0xfffffc00, 0xfffffc00, /* Words 48 to 51 */
	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00, /* Words 52 to 55 */
	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01, /* Words 56 to 59 */
	0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01, /* Words 60 to 63 */
};

static void upm_setup(struct fsl_upm *upm)
{
	int i;

	/* write upm array */
	out_be32(upm->mxmr, MxMR_OP_WARR);

	for (i = 0; i < 64; i++) {
		out_be32(upm->mdr, upm_array[i]);
		out_8(upm->io_addr, 0x0);
	}

	/* normal operation */
	out_be32(upm->mxmr, MxMR_OP_NORM);
	while (in_be32(upm->mxmr) != MxMR_OP_NORM)
		eieio();
}

static int dev_ready(int chip_nr)
{
	if (in_be32(&im->qepio.ioport[4].pdat) & 0x00002000) {
		debug("nand ready\n");
		return 1;
	}

	debug("nand busy\n");
	return 0;
}

static struct fsl_upm_nand fun = {
	.upm = {
		.io_addr = (void *)CONFIG_SYS_NAND_BASE,
	},
	.width = 8,
	.upm_cmd_offset = 8,
	.upm_addr_offset = 16,
	.dev_ready = dev_ready,
	.wait_flags = FSL_UPM_WAIT_RUN_PATTERN,
	.chip_delay = 50,
};

int board_nand_init(struct nand_chip *nand)
{
	fun.upm.mxmr = &im->im_lbc.mamr;
	fun.upm.mdr = &im->im_lbc.mdr;
	fun.upm.mar = &im->im_lbc.mar;

	upm_setup(&fun.upm);

	return fsl_upm_nand_init(nand, &fun);
}

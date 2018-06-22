/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * NXP GPMI NAND flash driver
 *
 * Copyright (C) 2018 Toradex
 * Authors:
 * Stefan Agner <stefan.agner@toradex.com>
 */

int mxs_nand_init_spl(struct nand_chip *nand);
int mxs_nand_setup_ecc(struct mtd_info *mtd);

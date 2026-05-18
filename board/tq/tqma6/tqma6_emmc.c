// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <fdt_support.h>
#include <mmc.h>

#include "tqma6_emmc.h"

struct emmc_dsr_lookup {
	uint mfgid;
	char *pnm;
	int dsr_needed;
};

static const struct emmc_dsr_lookup dsr_tbl[] = {
	/* Micron, eMMC 4.41 */
	{ 0xfe, "MMC02G", 1 },
	{ 0xfe, "MMC04G", 1 },
	{ 0xfe, "MMC08G", 1 },
	/* Micron, eMMC 5.0 4 GB*/
	{ 0x13, "Q1J54A", 1 },
	{ 0x13, "Q2J54A", 1 },
	/* Micron, eMMC 5.0 8 GB*/
	{ 0x13, "Q2J55L", 0 },
	/* Samsung, eMMC 5.0 */
	{ 0x15, "8GSD3R", 0 },
	{ 0x15, "AGSD3R", 0 },
	{ 0x15, "BGSD3R", 0 },
	{ 0x15, "CGSD3R", 0 },
	/* SanDisk, iNAND 7250 5.1 */
	{ 0x45, "DG4008", 0 },
	{ 0x45, "DG4016", 0 },
	{ 0x45, "DG4032", 0 },
	{ 0x45, "DG4064", 0 },
	/* Kingston */
	{ 0x100, "?????", 0 },
};

int tqma6_emmc_need_dsr(const struct mmc *mmc)
{
	uint mfgid = mmc->cid[0] >> 24;
	char name[7];
	int ret = -1;
	size_t i;

	if (IS_SD(mmc))
		return 0;

	sprintf(name, "%c%c%c%c%c%c", mmc->cid[0] & 0xff, (mmc->cid[1] >> 24),
		(mmc->cid[1] >> 16) & 0xff, (mmc->cid[1] >> 8) & 0xff,
		mmc->cid[1] & 0xff, (mmc->cid[2] >> 24));

	for (i = 0; i < ARRAY_SIZE(dsr_tbl) && (ret < 0); ++i) {
		if (dsr_tbl[i].mfgid == mfgid &&
		    (!strncmp(name, dsr_tbl[i].pnm, 6))) {
			ret = dsr_tbl[i].dsr_needed;
			debug("MFG: %x PNM: %s\n", mfgid, name);
		}
	}

	if (ret < 0) {
		printf("eMMC unknown: MFG: %x PNM: %s\n", mfgid, name);
		/* request DSR, even if not known if supported to be safe */
		ret = 1;
	}

	return ret;
}

void tqma6_ft_fixup_emmc_dsr(void *blob, const char *path, u32 value)
{
	do_fixup_by_path_u32(blob, path, "dsr", value, 1);
}

void tqma6_mmc_detect_card_type(struct mmc *mmc)
{
	struct mmc *emmc = find_mmc_device(0);

	if (emmc != mmc)
		return;

	if (tqma6_emmc_need_dsr(mmc) > 0)
		mmc_set_dsr(mmc, TQMA6_EMMC_DSR);
}

/*
 * Adapted from Linux v2.6.36 kernel: arch/powerpc/kernel/asm-offsets.c
 *
 * This program is used to generate definitions needed by
 * assembly language modules.
 *
 * We use the technique used in the OSF Mach kernel code:
 * generate asm statements containing #defines,
 * compile this file to assembler, and then extract the
 * #defines from the assembly-language output.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/mb86r0x.h>

#include <linux/kbuild.h>

int main(void)
{
	/* ddr2 controller */
	DEFINE(DDR2_DRIC, offsetof(struct mb86r0x_ddr2c, dric));
	DEFINE(DDR2_DRIC1, offsetof(struct mb86r0x_ddr2c, dric1));
	DEFINE(DDR2_DRIC2, offsetof(struct mb86r0x_ddr2c, dric2));
	DEFINE(DDR2_DRCA, offsetof(struct mb86r0x_ddr2c, drca));
	DEFINE(DDR2_DRCM, offsetof(struct mb86r0x_ddr2c, drcm));
	DEFINE(DDR2_DRCST1, offsetof(struct mb86r0x_ddr2c, drcst1));
	DEFINE(DDR2_DRCST2, offsetof(struct mb86r0x_ddr2c, drcst2));
	DEFINE(DDR2_DRCR, offsetof(struct mb86r0x_ddr2c, drcr));
	DEFINE(DDR2_DRCF, offsetof(struct mb86r0x_ddr2c, drcf));
	DEFINE(DDR2_DRASR, offsetof(struct mb86r0x_ddr2c, drasr));
	DEFINE(DDR2_DRIMS, offsetof(struct mb86r0x_ddr2c, drims));
	DEFINE(DDR2_DROS, offsetof(struct mb86r0x_ddr2c, dros));
	DEFINE(DDR2_DRIBSODT1, offsetof(struct mb86r0x_ddr2c, dribsodt1));
	DEFINE(DDR2_DROABA, offsetof(struct mb86r0x_ddr2c, droaba));
	DEFINE(DDR2_DROBS, offsetof(struct mb86r0x_ddr2c, drobs));

	/* clock reset generator */
	DEFINE(CRG_CRPR, offsetof(struct mb86r0x_crg, crpr));
	DEFINE(CRG_CRHA, offsetof(struct mb86r0x_crg, crha));
	DEFINE(CRG_CRPA, offsetof(struct mb86r0x_crg, crpa));
	DEFINE(CRG_CRPB, offsetof(struct mb86r0x_crg, crpb));
	DEFINE(CRG_CRHB, offsetof(struct mb86r0x_crg, crhb));
	DEFINE(CRG_CRAM, offsetof(struct mb86r0x_crg, cram));

	/* chip control module */
	DEFINE(CCNT_CDCRC, offsetof(struct mb86r0x_ccnt, cdcrc));

	/* external bus interface */
	DEFINE(MEMC_MCFMODE0, offsetof(struct mb86r0x_memc, mcfmode[0]));
	DEFINE(MEMC_MCFMODE2, offsetof(struct mb86r0x_memc, mcfmode[2]));
	DEFINE(MEMC_MCFMODE4, offsetof(struct mb86r0x_memc, mcfmode[4]));
	DEFINE(MEMC_MCFTIM0, offsetof(struct mb86r0x_memc, mcftim[0]));
	DEFINE(MEMC_MCFTIM2, offsetof(struct mb86r0x_memc, mcftim[2]));
	DEFINE(MEMC_MCFTIM4, offsetof(struct mb86r0x_memc, mcftim[4]));
	DEFINE(MEMC_MCFAREA0, offsetof(struct mb86r0x_memc, mcfarea[0]));
	DEFINE(MEMC_MCFAREA2, offsetof(struct mb86r0x_memc, mcfarea[2]));
	DEFINE(MEMC_MCFAREA4, offsetof(struct mb86r0x_memc, mcfarea[4]));

	return 0;
}

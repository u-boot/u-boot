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
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <common.h>
#include <asm/arch/sc520.h>

#include <linux/kbuild.h>

int main(void)
{
	DEFINE(GENERATED_GD_RELOC_OFF, offsetof(gd_t, reloc_off));

	DEFINE(GENERATED_SC520_PAR0, offsetof(struct sc520_mmcr, par[0]));
	DEFINE(GENERATED_SC520_PAR1, offsetof(struct sc520_mmcr, par[1]));
	DEFINE(GENERATED_SC520_PAR2, offsetof(struct sc520_mmcr, par[2]));
	DEFINE(GENERATED_SC520_PAR3, offsetof(struct sc520_mmcr, par[3]));
	DEFINE(GENERATED_SC520_PAR4, offsetof(struct sc520_mmcr, par[4]));
	DEFINE(GENERATED_SC520_PAR5, offsetof(struct sc520_mmcr, par[5]));
	DEFINE(GENERATED_SC520_PAR6, offsetof(struct sc520_mmcr, par[6]));
	DEFINE(GENERATED_SC520_PAR7, offsetof(struct sc520_mmcr, par[7]));
	DEFINE(GENERATED_SC520_PAR8, offsetof(struct sc520_mmcr, par[8]));
	DEFINE(GENERATED_SC520_PAR9, offsetof(struct sc520_mmcr, par[9]));
	DEFINE(GENERATED_SC520_PAR10, offsetof(struct sc520_mmcr, par[10]));
	DEFINE(GENERATED_SC520_PAR11, offsetof(struct sc520_mmcr, par[11]));
	DEFINE(GENERATED_SC520_PAR12, offsetof(struct sc520_mmcr, par[12]));
	DEFINE(GENERATED_SC520_PAR13, offsetof(struct sc520_mmcr, par[13]));
	DEFINE(GENERATED_SC520_PAR14, offsetof(struct sc520_mmcr, par[14]));
	DEFINE(GENERATED_SC520_PAR15, offsetof(struct sc520_mmcr, par[15]));

	return 0;
}

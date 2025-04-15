// SPDX-License-Identifier: GPL-2.0+
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
 */

#include <asm-offsets.h>
#include <asm/global_data.h>
#include <asm/u-boot.h>

#include <linux/kbuild.h>

int main(void)
{
	/* Round up to make sure size gives nice stack alignment */
	DEFINE(GENERATED_GBL_DATA_SIZE,
		(sizeof(struct global_data) + 15) & ~15);

	DEFINE(GENERATED_BD_INFO_SIZE,
		(sizeof(struct bd_info) + 15) & ~15);

	DEFINE(GD_SIZE, sizeof(struct global_data));

	DEFINE(GD_BD, offsetof(struct global_data, bd));

	DEFINE(GD_FLAGS, offsetof(struct global_data, flags));

#if CONFIG_IS_ENABLED(SYS_MALLOC_F)
	DEFINE(GD_MALLOC_BASE, offsetof(struct global_data, malloc_base));
#endif

	DEFINE(GD_RELOCADDR, offsetof(struct global_data, relocaddr));

	DEFINE(GD_RELOC_OFF, offsetof(struct global_data, reloc_off));

	DEFINE(GD_START_ADDR_SP, offsetof(struct global_data, start_addr_sp));

	DEFINE(GD_NEW_GD, offsetof(struct global_data, new_gd));

#if CONFIG_IS_ENABLED(ENV_SUPPORT)
	DEFINE(GD_ENV_ADDR, offsetof(struct global_data, env_addr));
#endif

	return 0;
}

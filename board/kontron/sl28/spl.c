// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <asm/io.h>
#include <asm/spl.h>

#define DCFG_RCWSR25 0x160
#define GPINFO_HW_VARIANT_MASK 0xff

int sl28_variant(void)
{
	return in_le32(DCFG_BASE + DCFG_RCWSR25) & GPINFO_HW_VARIANT_MASK;
}

int board_fit_config_name_match(const char *name)
{
	int variant = sl28_variant();

	switch (variant) {
	case 3:
		return strcmp(name, "fsl-ls1028a-kontron-sl28-var3");
	case 4:
		return strcmp(name, "fsl-ls1028a-kontron-sl28-var4");
	default:
		return strcmp(name, "fsl-ls1028a-kontron-sl28");
	}
}

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = BOOT_DEVICE_SPI;
}

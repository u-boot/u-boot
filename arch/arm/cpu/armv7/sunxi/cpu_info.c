/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <axp221.h>

#ifdef CONFIG_MACH_SUN6I
int sunxi_get_ss_bonding_id(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	static int bonding_id = -1;

	if (bonding_id != -1)
		return bonding_id;

	/* Enable Security System */
	setbits_le32(&ccm->ahb_reset0_cfg, 1 << AHB_RESET_OFFSET_SS);
	setbits_le32(&ccm->ahb_gate0, 1 << AHB_GATE_OFFSET_SS);

	bonding_id = readl(SUNXI_SS_BASE);
	bonding_id = (bonding_id >> 16) & 0x7;

	/* Disable Security System again */
	clrbits_le32(&ccm->ahb_gate0, 1 << AHB_GATE_OFFSET_SS);
	clrbits_le32(&ccm->ahb_reset0_cfg, 1 << AHB_RESET_OFFSET_SS);

	return bonding_id;
}
#endif

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
#ifdef CONFIG_MACH_SUN4I
	puts("CPU:   Allwinner A10 (SUN4I)\n");
#elif defined CONFIG_MACH_SUN5I
	u32 val = readl(SUNXI_SID_BASE + 0x08);
	switch ((val >> 12) & 0xf) {
	case 0: puts("CPU:   Allwinner A12 (SUN5I)\n"); break;
	case 3: puts("CPU:   Allwinner A13 (SUN5I)\n"); break;
	case 7: puts("CPU:   Allwinner A10s (SUN5I)\n"); break;
	default: puts("CPU:   Allwinner A1X (SUN5I)\n");
	}
#elif defined CONFIG_MACH_SUN6I
	switch (sunxi_get_ss_bonding_id()) {
	case SUNXI_SS_BOND_ID_A31:
		puts("CPU:   Allwinner A31 (SUN6I)\n");
		break;
	case SUNXI_SS_BOND_ID_A31S:
		puts("CPU:   Allwinner A31s (SUN6I)\n");
		break;
	default:
		printf("CPU:   Allwinner A31? (SUN6I, id: %d)\n",
		       sunxi_get_ss_bonding_id());
	}
#elif defined CONFIG_MACH_SUN7I
	puts("CPU:   Allwinner A20 (SUN7I)\n");
#elif defined CONFIG_MACH_SUN8I
	puts("CPU:   Allwinner A23 (SUN8I)\n");
#else
#warning Please update cpu_info.c with correct CPU information
	puts("CPU:   SUNXI Family\n");
#endif
	return 0;
}
#endif

int sunxi_get_sid(unsigned int *sid)
{
#if defined CONFIG_MACH_SUN6I || defined CONFIG_MACH_SUN8I
#ifdef CONFIG_AXP221_POWER
	return axp221_get_sid(sid);
#else
	return -ENODEV;
#endif
#else
	int i;

	for (i = 0; i< 4; i++)
		sid[i] = readl(SUNXI_SID_BASE + 4 * i);

	return 0;
#endif
}

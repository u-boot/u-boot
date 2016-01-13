/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>
#include <linux/types.h>
#include <mach/sg-regs.h>
#include <mach/soc_info.h>

#if UNIPHIER_MULTI_SOC
enum uniphier_soc_id uniphier_get_soc_type(void)
{
	u32 revision = readl(SG_REVISION);
	enum uniphier_soc_id ret;

	switch ((revision & SG_REVISION_TYPE_MASK) >> SG_REVISION_TYPE_SHIFT) {
#ifdef CONFIG_ARCH_UNIPHIER_PH1_SLD3
	case 0x25:
		ret = SOC_UNIPHIER_PH1_SLD3;
		break;
#endif
#ifdef CONFIG_ARCH_UNIPHIER_PH1_LD4
	case 0x26:
		ret = SOC_UNIPHIER_PH1_LD4;
		break;
#endif
#ifdef CONFIG_ARCH_UNIPHIER_PH1_PRO4
	case 0x28:
		ret = SOC_UNIPHIER_PH1_PRO4;
		break;
#endif
#ifdef CONFIG_ARCH_UNIPHIER_PH1_SLD8
	case 0x29:
		ret = SOC_UNIPHIER_PH1_SLD8;
		break;
#endif
#ifdef CONFIG_ARCH_UNIPHIER_PH1_PRO5
	case 0x2A:
		ret = SOC_UNIPHIER_PH1_PRO5;
		break;
#endif
#ifdef CONFIG_ARCH_UNIPHIER_PROXSTREAM2
	case 0x2E:
		ret = SOC_UNIPHIER_PROXSTREAM2;
		break;
#endif
#ifdef CONFIG_ARCH_UNIPHIER_PH1_LD6B
	case 0x2F:
		ret = SOC_UNIPHIER_PH1_LD6B;
		break;
#endif
#ifdef CONFIG_ARCH_UNIPHIER_PH1_SLD11
	case 0x31:
		ret = SOC_UNIPHIER_PH1_SLD11;
		break;
#endif
#ifdef CONFIG_ARCH_UNIPHIER_PH1_LD10
	case 0x32:
		ret = SOC_UNIPHIER_PH1_LD10;
		break;
#endif
	default:
		ret = SOC_UNIPHIER_UNKNOWN;
		break;
	}

	return ret;
}
#endif

int uniphier_get_soc_model(void)
{
	return (readl(SG_REVISION) & SG_REVISION_MODEL_MASK) >>
						SG_REVISION_MODEL_SHIFT;
}

int uniphier_get_soc_revision(void)
{
	return (readl(SG_REVISION) & SG_REVISION_REV_MASK) >>
						SG_REVISION_REV_SHIFT;
}

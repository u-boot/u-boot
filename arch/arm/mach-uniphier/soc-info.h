/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __MACH_SOC_INFO_H__
#define __MACH_SOC_INFO_H__

enum uniphier_soc_id {
	SOC_UNIPHIER_SLD3,
	SOC_UNIPHIER_LD4,
	SOC_UNIPHIER_PRO4,
	SOC_UNIPHIER_SLD8,
	SOC_UNIPHIER_PRO5,
	SOC_UNIPHIER_PXS2,
	SOC_UNIPHIER_LD6B,
	SOC_UNIPHIER_LD11,
	SOC_UNIPHIER_LD20,
	SOC_UNIPHIER_UNKNOWN,
};

#define UNIPHIER_NR_ENABLED_SOCS		\
	IS_ENABLED(CONFIG_ARCH_UNIPHIER_SLD3) +	\
	IS_ENABLED(CONFIG_ARCH_UNIPHIER_LD4) +	\
	IS_ENABLED(CONFIG_ARCH_UNIPHIER_PRO4) +	\
	IS_ENABLED(CONFIG_ARCH_UNIPHIER_SLD8) +	\
	IS_ENABLED(CONFIG_ARCH_UNIPHIER_PRO5) +	\
	IS_ENABLED(CONFIG_ARCH_UNIPHIER_PXS2) +	\
	IS_ENABLED(CONFIG_ARCH_UNIPHIER_LD6B) + \
	IS_ENABLED(CONFIG_ARCH_UNIPHIER_LD11) + \
	IS_ENABLED(CONFIG_ARCH_UNIPHIER_LD20)

#define UNIPHIER_MULTI_SOC	((UNIPHIER_NR_ENABLED_SOCS) > 1)

#if UNIPHIER_MULTI_SOC
enum uniphier_soc_id uniphier_get_soc_type(void);
#else
static inline enum uniphier_soc_id uniphier_get_soc_type(void)
{
#if defined(CONFIG_ARCH_UNIPHIER_SLD3)
	return SOC_UNIPHIER_SLD3;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD4)
	return SOC_UNIPHIER_LD4;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO4)
	return SOC_UNIPHIER_PRO4;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_SLD8)
	return SOC_UNIPHIER_SLD8;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	return SOC_UNIPHIER_PRO5;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2)
	return SOC_UNIPHIER_PXS2;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD6B)
	return SOC_UNIPHIER_LD6B;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD11)
	return SOC_UNIPHIER_LD11;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD20)
	return SOC_UNIPHIER_LD20;
#endif

	return SOC_UNIPHIER_UNKNOWN;
}
#endif

int uniphier_get_soc_model(void);
int uniphier_get_soc_revision(void);

#endif /* __MACH_SOC_INFO_H__ */

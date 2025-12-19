// SPDX-License-Identifier: GPL-2.0+
/*
 * Portions Copyright (C) 2015, Amlogic, Inc. All rights reserved.
 * Copyright (C) 2023, Ferass El Hafidi <funderscore@postmarketos.org>
 */
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/gx.h>

void meson_power_init_gxl(void)
{
	/* TODO: Support more voltages */

	/* Init PWM B */
	clrsetbits_32(GX_PWM_MISC_REG_AB, 0x7f << 16, (1 << 23) | (1 << 1));

	/* Set voltage */
	if (CONFIG_IS_ENABLED(MESON_GX_VCCK_1120MV))
		writel(0x02001a, GX_PWM_PWM_B);
	else if (CONFIG_IS_ENABLED(MESON_GX_VCCK_1100MV))
		writel(0x040018, GX_PWM_PWM_B);
	else if (CONFIG_IS_ENABLED(MESON_GX_VCCK_1000MV))
		writel(0x0e000e, GX_PWM_PWM_B);

	clrbits_32(GX_PIN_MUX_REG1, 1 << 10);
	clrsetbits_32(GX_PIN_MUX_REG2, 1 << 5, 1 << 11);

	/* Init PWM D */
	clrsetbits_32(GX_PWM_MISC_REG_CD, 0x7f << 16, (1 << 23) | (1 << 1));

	/* Set voltage */
	if (CONFIG_IS_ENABLED(MESON_GX_VDDEE_1100MV))
		writel(0x040018, GX_PWM_PWM_B);
	else if (CONFIG_IS_ENABLED(MESON_GX_VDDEE_1000MV))
		writel(0x0e000e, GX_PWM_PWM_B);

	clrbits_32(GX_PIN_MUX_REG1, (1 << 9) | (1 << 11));
	setbits_32(GX_PIN_MUX_REG2, 1 << 12);
}

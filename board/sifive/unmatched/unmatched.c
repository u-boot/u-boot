// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020-2021, SiFive Inc
 *
 * Authors:
 *   Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <asm/sections.h>
#include <asm/cache.h>
#include <linux/io.h>
#include <asm/arch/eeprom.h>

struct pwm_sifive_regs {
	unsigned int cfg;	/* PWM configuration register */
	unsigned int pad0;	/* Reserved */
	unsigned int cnt;	/* PWM count register */
	unsigned int pad1;	/* Reserved */
	unsigned int pwms;	/* Scaled PWM count register */
	unsigned int pad2;	/* Reserved */
	unsigned int pad3;	/* Reserved */
	unsigned int pad4;	/* Reserved */
	unsigned int cmp0;	/* PWM 0 compare register */
	unsigned int cmp1;	/* PWM 1 compare register */
	unsigned int cmp2;	/* PWM 2 compare register */
	unsigned int cmp3;	/* PWM 3 compare register */
};

#define PWM0_BASE		0x10020000
#define PWM1_BASE		0x10021000
#define PWM_CFG_INIT		0x1000
#define PWM_CMP_ENABLE_VAL	0x0
#define PWM_CMP_DISABLE_VAL	0xffff

void pwm_device_init(void)
{
	struct pwm_sifive_regs *pwm0, *pwm1;
	pwm0 = (struct pwm_sifive_regs *)PWM0_BASE;
	pwm1 = (struct pwm_sifive_regs *)PWM1_BASE;
#ifdef CONFIG_SPL_BUILD
	writel(PWM_CMP_DISABLE_VAL, (void *)&pwm0->cmp0);
	/* Set the 3-color PWM LEDs to yellow in SPL */
	writel(PWM_CMP_ENABLE_VAL, (void *)&pwm0->cmp1);
	writel(PWM_CMP_ENABLE_VAL, (void *)&pwm0->cmp2);
	writel(PWM_CMP_DISABLE_VAL, (void *)&pwm0->cmp3);
	writel(PWM_CFG_INIT, (void *)&pwm0->cfg);

	writel(PWM_CMP_DISABLE_VAL, (void *)&pwm0->cmp3);
	/* Turn on all the fans, (J21), (J23) and (J24), on the unmatched board */
	/* The SoC fan(J21) on the rev3 board cannot be controled by PWM_COMP0,
	   so here sets the initial value of PWM_COMP0 as DISABLE */
	if (get_pcb_revision_from_eeprom() == PCB_REVISION_REV3)
		writel(PWM_CMP_DISABLE_VAL, (void *)&pwm1->cmp1);
	else
		writel(PWM_CMP_ENABLE_VAL, (void *)&pwm1->cmp1);
	writel(PWM_CMP_ENABLE_VAL, (void *)&pwm1->cmp2);
	writel(PWM_CMP_ENABLE_VAL, (void *)&pwm1->cmp3);
	writel(PWM_CFG_INIT, (void *)&pwm1->cfg);
#else
	/* Set the 3-color PWM LEDs to purple in U-boot */
	writel(PWM_CMP_DISABLE_VAL, (void *)&pwm0->cmp1);
	writel(PWM_CMP_ENABLE_VAL, (void *)&pwm0->cmp2);
	writel(PWM_CMP_ENABLE_VAL, (void *)&pwm0->cmp3);
#endif

}

int board_early_init_f(void)
{
	pwm_device_init();
	return 0;
}

void *board_fdt_blob_setup(int *err)
{
	*err = 0;
	if (IS_ENABLED(CONFIG_OF_SEPARATE)) {
		if (gd->arch.firmware_fdt_addr)
			return (ulong *)(uintptr_t)gd->arch.firmware_fdt_addr;
	}

	return (ulong *)&_end;
}

int board_init(void)
{
	/* enable all cache ways */
	enable_caches();

	return 0;
}

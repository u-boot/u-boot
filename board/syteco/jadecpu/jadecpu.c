/*
 * (c) 2010 Graf-Syteco, Matthias Weisser
 * <weisserm@arcor.de>
 *
 * (C) Copyright 2007, mycable GmbH
 * Carsten Schneider <cs@mycable.de>, Alexander Bigga <ab@mycable.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/mb86r0x.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscellaneous platform dependent initialisations
 */
int board_init(void)
{
	struct mb86r0x_ccnt * ccnt = (struct mb86r0x_ccnt *)
					MB86R0x_CCNT_BASE;

	/* We select mode 0 for group 2 and mode 1 for group 4 */
	writel(0x00000010, &ccnt->cmux_md);

	gd->flags = 0;
	gd->bd->bi_arch_number = MACH_TYPE_JADECPU;
	gd->bd->bi_boot_params = PHYS_SDRAM + PHYS_SDRAM_SIZE - 0x10000;

	icache_enable();
	dcache_enable();

	return 0;
}

static void setup_display_power(uint32_t pwr_bit, char *pwm_opts,
				unsigned long pwm_base)
{
	struct mb86r0x_gpio *gpio = (struct mb86r0x_gpio *)
					MB86R0x_GPIO_BASE;
	struct mb86r0x_pwm *pwm = (struct mb86r0x_pwm *) pwm_base;
	const char *e;

	writel(readl(&gpio->gpdr2) | pwr_bit, &gpio->gpdr2);

	e = getenv(pwm_opts);
	if (e != NULL) {
		const char *s;
		uint32_t freq, init;

		freq = 0;
		init = 0;

		s = strchr(e, 'f');
		if (s != NULL)
			freq = simple_strtol(s + 2, NULL, 0);

		s = strchr(e, 'i');
		if (s != NULL)
			init = simple_strtol(s + 2, NULL, 0);

		if (freq > 0) {
			writel(CONFIG_MB86R0x_IOCLK / 1000 / freq,
				&pwm->bcr);
			writel(1002, &pwm->tpr);
			writel(1, &pwm->pr);
			writel(init * 10 + 1, &pwm->dr);
			writel(1, &pwm->cr);
			writel(1, &pwm->sr);
		}
	}
}

int board_late_init(void)
{
	struct mb86r0x_gpio *gpio = (struct mb86r0x_gpio *)
					MB86R0x_GPIO_BASE;
	uint32_t in_word;

#ifdef CONFIG_VIDEO_MB86R0xGDC
	/* Check if we have valid display settings and turn on power if so */
	/* Display 0 */
	if (getenv("gs_dsp_0_param") || getenv("videomode"))
		setup_display_power((1 << 3), "gs_dsp_0_pwm",
					MB86R0x_PWM0_BASE);

	/* The corresponding GPIO is always an output */
	writel(readl(&gpio->gpddr2) | (1 << 3), &gpio->gpddr2);

	/* Display 1 */
	if (getenv("gs_dsp_1_param") || getenv("videomode1"))
		setup_display_power((1 << 4), "gs_dsp_1_pwm",
					MB86R0x_PWM1_BASE);

	/* The corresponding GPIO is always an output */
	writel(readl(&gpio->gpddr2) | (1 << 4), &gpio->gpddr2);
#endif /* CONFIG_VIDEO_MB86R0xGDC */

	/* 5V enable */
	writel(readl(&gpio->gpdr1) & ~(1 << 5), &gpio->gpdr1);
	writel(readl(&gpio->gpddr1) | (1 << 5), &gpio->gpddr1);

	/* We have special boot options if told by GPIOs */
	in_word = readl(&gpio->gpdr1);

	if ((in_word & 0xC0) == 0xC0) {
		setenv("stdin", "serial");
		setenv("stdout", "serial");
		setenv("stderr", "serial");
		setenv("preboot", "run gs_slow_boot");
	} else if ((in_word & 0xC0) != 0) {
		setenv("stdout", "vga");
		setenv("preboot", "run gs_slow_boot");
	} else {
		setenv("stdin", "serial");
		setenv("stdout", "serial");
		setenv("stderr", "serial");
		if (getenv("gs_devel")) {
			setenv("preboot", "run gs_slow_boot");
		} else {
			setenv("preboot", "run gs_fast_boot");
		}
	}

	return 0;
}

int misc_init_r(void)
{
	return 0;
}

/*
 * DRAM configuration
 */
int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM,
					PHYS_SDRAM_SIZE);

	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = gd->ram_size;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}

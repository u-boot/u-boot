/*
 * (C) Copyright 2007-2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/ppc4xx-gpio.h>

/* Only compile this file for boards with GPIO support */
#if defined(GPIO0_BASE)

#if defined(CONFIG_SYS_4xx_GPIO_TABLE)
gpio_param_s const gpio_tab[GPIO_GROUP_MAX][GPIO_MAX] = CONFIG_SYS_4xx_GPIO_TABLE;
#endif

#if defined(GPIO0_OSRL)
/* Only some 4xx variants support alternate funtions on the GPIO's */
void gpio_config(int pin, int in_out, int gpio_alt, int out_val)
{
	u32 mask;
	u32 mask2;
	u32 val;
	u32 offs = 0;
	u32 offs2 = 0;
	int pin2 = pin << 1;

	if (pin >= GPIO_MAX) {
		offs = 0x100;
		pin -= GPIO_MAX;
	}

	if (pin >= GPIO_MAX/2) {
		offs2 = 0x4;
		pin2 = (pin - GPIO_MAX/2) << 1;
	}

	mask = 0x80000000 >> pin;
	mask2 = 0xc0000000 >> pin2;

	/* first set TCR to 0 */
	out_be32((void *)GPIO0_TCR + offs, in_be32((void *)GPIO0_TCR + offs) & ~mask);

	if (in_out == GPIO_OUT) {
		val = in_be32((void *)GPIO0_OSRL + offs + offs2) & ~mask2;
		switch (gpio_alt) {
		case GPIO_ALT1:
			val |= GPIO_ALT1_SEL >> pin2;
			break;
		case GPIO_ALT2:
			val |= GPIO_ALT2_SEL >> pin2;
			break;
		case GPIO_ALT3:
			val |= GPIO_ALT3_SEL >> pin2;
			break;
		}
		out_be32((void *)GPIO0_OSRL + offs + offs2, val);

		/* setup requested output value */
		if (out_val == GPIO_OUT_0)
			out_be32((void *)GPIO0_OR + offs,
				 in_be32((void *)GPIO0_OR + offs) & ~mask);
		else if (out_val == GPIO_OUT_1)
			out_be32((void *)GPIO0_OR + offs,
				 in_be32((void *)GPIO0_OR + offs) | mask);

		/* now configure TCR to drive output if selected */
		out_be32((void *)GPIO0_TCR + offs,
			 in_be32((void *)GPIO0_TCR + offs) | mask);
	} else {
		val = in_be32((void *)GPIO0_ISR1L + offs + offs2) & ~mask2;
		val |= GPIO_IN_SEL >> pin2;
		out_be32((void *)GPIO0_ISR1L + offs + offs2, val);
	}
}
#endif /* GPIO_OSRL */

void gpio_write_bit(int pin, int val)
{
	u32 offs = 0;

	if (pin >= GPIO_MAX) {
		offs = 0x100;
		pin -= GPIO_MAX;
	}

	if (val)
		out_be32((void *)GPIO0_OR + offs,
			 in_be32((void *)GPIO0_OR + offs) | GPIO_VAL(pin));
	else
		out_be32((void *)GPIO0_OR + offs,
			 in_be32((void *)GPIO0_OR + offs) & ~GPIO_VAL(pin));
}

int gpio_read_out_bit(int pin)
{
	u32 offs = 0;

	if (pin >= GPIO_MAX) {
		offs = 0x100;
		pin -= GPIO_MAX;
	}

	return (in_be32((void *)GPIO0_OR + offs) & GPIO_VAL(pin) ? 1 : 0);
}

int gpio_read_in_bit(int pin)
{
	u32 offs = 0;

	if (pin >= GPIO_MAX) {
		offs = 0x100;
		pin -= GPIO_MAX;
	}

	return (in_be32((void *)GPIO0_IR + offs) & GPIO_VAL(pin) ? 1 : 0);
}

#if defined(CONFIG_SYS_4xx_GPIO_TABLE)
void gpio_set_chip_configuration(void)
{
	unsigned char i=0, j=0, offs=0, gpio_core;
	unsigned long reg, core_add;

	for (gpio_core=0; gpio_core<GPIO_GROUP_MAX; gpio_core++) {
		j = 0;
		offs = 0;
		/* GPIO config of the GPIOs 0 to 31 */
		for (i=0; i<GPIO_MAX; i++, j++) {
			if (i == GPIO_MAX/2) {
				offs = 4;
				j = i-16;
			}

			core_add = gpio_tab[gpio_core][i].add;

			if ((gpio_tab[gpio_core][i].in_out == GPIO_IN) ||
			    (gpio_tab[gpio_core][i].in_out == GPIO_BI)) {

				switch (gpio_tab[gpio_core][i].alt_nb) {
				case GPIO_SEL:
					break;

				case GPIO_ALT1:
					reg = in_be32((void *)GPIO_IS1(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					reg = reg | (GPIO_IN_SEL >> (j*2));
					out_be32((void *)GPIO_IS1(core_add+offs), reg);
					break;

				case GPIO_ALT2:
					reg = in_be32((void *)GPIO_IS2(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					reg = reg | (GPIO_IN_SEL >> (j*2));
					out_be32((void *)GPIO_IS2(core_add+offs), reg);
					break;

				case GPIO_ALT3:
					reg = in_be32((void *)GPIO_IS3(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					reg = reg | (GPIO_IN_SEL >> (j*2));
					out_be32((void *)GPIO_IS3(core_add+offs), reg);
					break;
				}
			}

			if ((gpio_tab[gpio_core][i].in_out == GPIO_OUT) ||
			    (gpio_tab[gpio_core][i].in_out == GPIO_BI)) {

				u32 gpio_alt_sel = 0;

				switch (gpio_tab[gpio_core][i].alt_nb) {
				case GPIO_SEL:
					/*
					 * Setup output value
					 * 1 -> high level
					 * 0 -> low level
					 * else -> don't touch
					 */
					reg = in_be32((void *)GPIO_OR(core_add));
					if (gpio_tab[gpio_core][i].out_val == GPIO_OUT_1)
						reg |= (0x80000000 >> (i));
					else if (gpio_tab[gpio_core][i].out_val == GPIO_OUT_0)
						reg &= ~(0x80000000 >> (i));
					out_be32((void *)GPIO_OR(core_add), reg);

					reg = in_be32((void *)GPIO_TCR(core_add)) |
						(0x80000000 >> (i));
					out_be32((void *)GPIO_TCR(core_add), reg);

					reg = in_be32((void *)GPIO_OS(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					out_be32((void *)GPIO_OS(core_add+offs), reg);
					reg = in_be32((void *)GPIO_TS(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					out_be32((void *)GPIO_TS(core_add+offs), reg);
					break;

				case GPIO_ALT1:
					gpio_alt_sel = GPIO_ALT1_SEL;
					break;

				case GPIO_ALT2:
					gpio_alt_sel = GPIO_ALT2_SEL;
					break;

				case GPIO_ALT3:
					gpio_alt_sel = GPIO_ALT3_SEL;
					break;
				}

				if (0 != gpio_alt_sel) {
					reg = in_be32((void *)GPIO_OS(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					reg = reg | (gpio_alt_sel >> (j*2));
					out_be32((void *)GPIO_OS(core_add+offs), reg);

					if (gpio_tab[gpio_core][i].out_val == GPIO_OUT_1) {
						reg = in_be32((void *)GPIO_TCR(core_add))
							| (0x80000000 >> (i));
						out_be32((void *)GPIO_TCR(core_add), reg);
						reg = in_be32((void *)GPIO_TS(core_add+offs))
							& ~(GPIO_MASK >> (j*2));
						out_be32((void *)GPIO_TS(core_add+offs), reg);
					} else {
						reg = in_be32((void *)GPIO_TCR(core_add))
							& ~(0x80000000 >> (i));
						out_be32((void *)GPIO_TCR(core_add), reg);
						reg = in_be32((void *)GPIO_TS(core_add+offs))
							& ~(GPIO_MASK >> (j*2));
						reg = reg | (gpio_alt_sel >> (j*2));
						out_be32((void *)GPIO_TS(core_add+offs), reg);
					}
				}
			}
		}
	}
}

#endif /* GPIO0_BASE */
#endif /* CONFIG_SYS_4xx_GPIO_TABLE */

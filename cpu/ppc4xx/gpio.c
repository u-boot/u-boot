/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/gpio.h>

#if defined(CFG_440_GPIO_TABLE)
gpio_param_s gpio_tab[GPIO_GROUP_MAX][GPIO_MAX] = CFG_440_GPIO_TABLE;
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
		offs2 = 0x100;
		pin2 = (pin - GPIO_MAX/2) << 1;
	}

	mask = 0x80000000 >> pin;
	mask2 = 0xc0000000 >> (pin2 << 1);

	/* first set TCR to 0 */
	out32(GPIO0_TCR + offs, in32(GPIO0_TCR + offs) & ~mask);

	if (in_out == GPIO_OUT) {
		val = in32(GPIO0_OSRL + offs + offs2) & ~mask2;
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
		out32(GPIO0_OSRL + offs + offs2, val);

		/* setup requested output value */
		if (out_val == GPIO_OUT_0)
			out32(GPIO0_OR + offs, in32(GPIO0_OR + offs) & ~mask);
		else if (out_val == GPIO_OUT_1)
			out32(GPIO0_OR + offs, in32(GPIO0_OR + offs) | mask);

		/* now configure TCR to drive output if selected */
		out32(GPIO0_TCR + offs, in32(GPIO0_TCR + offs) | mask);
	} else {
		val = in32(GPIO0_ISR1L + offs + offs2) & ~mask2;
		val |= GPIO_IN_SEL >> pin2;
		out32(GPIO0_ISR1L + offs + offs2, val);
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
		out32(GPIO0_OR + offs, in32(GPIO0_OR + offs) | GPIO_VAL(pin));
	else
		out32(GPIO0_OR + offs, in32(GPIO0_OR + offs) & ~GPIO_VAL(pin));
}

int gpio_read_out_bit(int pin)
{
	u32 offs = 0;

	if (pin >= GPIO_MAX) {
		offs = 0x100;
		pin -= GPIO_MAX;
	}

	return (in32(GPIO0_OR + offs) & GPIO_VAL(pin) ? 1 : 0);
}

#if defined(CFG_440_GPIO_TABLE)
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
					reg = in32(GPIO_IS1(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					reg = reg | (GPIO_IN_SEL >> (j*2));
					out32(GPIO_IS1(core_add+offs), reg);
					break;

				case GPIO_ALT2:
					reg = in32(GPIO_IS2(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					reg = reg | (GPIO_IN_SEL >> (j*2));
					out32(GPIO_IS2(core_add+offs), reg);
					break;

				case GPIO_ALT3:
					reg = in32(GPIO_IS3(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					reg = reg | (GPIO_IN_SEL >> (j*2));
					out32(GPIO_IS3(core_add+offs), reg);
					break;
				}
			}

			if ((gpio_tab[gpio_core][i].in_out == GPIO_OUT) ||
			    (gpio_tab[gpio_core][i].in_out == GPIO_BI)) {

				switch (gpio_tab[gpio_core][i].alt_nb) {
				case GPIO_SEL:
					if (gpio_core == GPIO0) {
						/*
						 * Setup output value
						 * 1 -> high level
						 * 0 -> low level
						 * else -> don't touch
						 */
						reg = in32(GPIO0_OR);
						if (gpio_tab[gpio_core][i].out_val == GPIO_OUT_1)
							reg |= (0x80000000 >> (i));
						else if (gpio_tab[gpio_core][i].out_val == GPIO_OUT_0)
							reg &= ~(0x80000000 >> (i));
						out32(GPIO0_OR, reg);

						reg = in32(GPIO0_TCR) | (0x80000000 >> (i));
						out32(GPIO0_TCR, reg);
					}

					if (gpio_core == GPIO1) {
						/*
						 * Setup output value
						 * 1 -> high level
						 * 0 -> low level
						 * else -> don't touch
						 */
						reg = in32(GPIO0_OR);
						if (gpio_tab[gpio_core][i].out_val == GPIO_OUT_1)
							reg |= (0x80000000 >> (i));
						else if (gpio_tab[gpio_core][i].out_val == GPIO_OUT_0)
							reg &= ~(0x80000000 >> (i));
						out32(GPIO0_OR, reg);

						reg = in32(GPIO1_TCR) | (0x80000000 >> (i));
						out32(GPIO1_TCR, reg);
					}

					reg = in32(GPIO_OS(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					out32(GPIO_OS(core_add+offs), reg);
					reg = in32(GPIO_TS(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					out32(GPIO_TS(core_add+offs), reg);
					break;

				case GPIO_ALT1:
					reg = in32(GPIO_OS(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					reg = reg | (GPIO_ALT1_SEL >> (j*2));
					out32(GPIO_OS(core_add+offs), reg);
					reg = in32(GPIO_TS(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					reg = reg | (GPIO_ALT1_SEL >> (j*2));
					out32(GPIO_TS(core_add+offs), reg);
					break;

				case GPIO_ALT2:
					reg = in32(GPIO_OS(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					reg = reg | (GPIO_ALT2_SEL >> (j*2));
					out32(GPIO_OS(core_add+offs), reg);
					reg = in32(GPIO_TS(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					reg = reg | (GPIO_ALT2_SEL >> (j*2));
					out32(GPIO_TS(core_add+offs), reg);
					break;

				case GPIO_ALT3:
					reg = in32(GPIO_OS(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					reg = reg | (GPIO_ALT3_SEL >> (j*2));
					out32(GPIO_OS(core_add+offs), reg);
					reg = in32(GPIO_TS(core_add+offs))
						& ~(GPIO_MASK >> (j*2));
					reg = reg | (GPIO_ALT3_SEL >> (j*2));
					out32(GPIO_TS(core_add+offs), reg);
					break;
				}
			}
		}
	}
}
#endif /* CFG_440_GPIO_TABLE */

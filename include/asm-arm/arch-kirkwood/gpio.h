/*
 * arch/asm-arm/mach-kirkwood/include/mach/gpio.h
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

/*
 * Based on (mostly copied from) plat-orion based Linux 2.6 kernel driver.
 * Removed kernel level irq handling. Took some macros from kernel to
 * allow build.
 *
 * Dieter Kiermaier dk-arm-linux@gmx.de
 */

#ifndef __KIRKWOOD_GPIO_H
#define __KIRKWOOD_GPIO_H

/* got from kernel include/linux/bitops.h */
#define BITS_PER_BYTE 8
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

#define GPIO_MAX		50
#define GPIO_OFF(pin)		(((pin) >> 5) ? 0x0040 : 0x0000)
#define GPIO_OUT(pin)		(KW_GPIO0_BASE + GPIO_OFF(pin) + 0x00)
#define GPIO_IO_CONF(pin)	(KW_GPIO0_BASE + GPIO_OFF(pin) + 0x04)
#define GPIO_BLINK_EN(pin)	(KW_GPIO0_BASE + GPIO_OFF(pin) + 0x08)
#define GPIO_IN_POL(pin)	(KW_GPIO0_BASE + GPIO_OFF(pin) + 0x0c)
#define GPIO_DATA_IN(pin)	(KW_GPIO0_BASE + GPIO_OFF(pin) + 0x10)
#define GPIO_EDGE_CAUSE(pin)	(KW_GPIO0_BASE + GPIO_OFF(pin) + 0x14)
#define GPIO_EDGE_MASK(pin)	(KW_GPIO0_BASE + GPIO_OFF(pin) + 0x18)
#define GPIO_LEVEL_MASK(pin)	(KW_GPIO0_BASE + GPIO_OFF(pin) + 0x1c)

/*
 * Kirkwood-specific GPIO API
 */

void kw_gpio_set_valid(unsigned pin, int mode);
int kw_gpio_is_valid(unsigned pin, int mode);
int kw_gpio_direction_input(unsigned pin);
int kw_gpio_direction_output(unsigned pin, int value);
int kw_gpio_get_value(unsigned pin);
void kw_gpio_set_value(unsigned pin, int value);
void kw_gpio_set_blink(unsigned pin, int blink);
void kw_gpio_set_unused(unsigned pin);

#define GPIO_INPUT_OK		(1 << 0)
#define GPIO_OUTPUT_OK		(1 << 1)

#endif

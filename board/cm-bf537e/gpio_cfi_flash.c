/*
 * gpio_cfi_flash.c - GPIO-assisted Flash Chip Support
 *
 * Copyright (c) 2009-2010 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <asm/blackfin.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include "gpio_cfi_flash.h"

/* Allow this driver to be shared among boards */
#ifndef GPIO_PIN_1
#define GPIO_PIN_1  GPIO_PF4
#endif
#define GPIO_MASK_1 (1 << 21)
#ifndef GPIO_PIN_2
#define GPIO_MASK_2 (0)
#else
#define GPIO_MASK_2 (1 << 22)
#endif
#ifndef GPIO_PIN_3
#define GPIO_MASK_3 (0)
#else
#define GPIO_MASK_3 (1 << 23)
#endif
#define GPIO_MASK   (GPIO_MASK_1 | GPIO_MASK_2 | GPIO_MASK_3)

void *gpio_cfi_flash_swizzle(void *vaddr)
{
	unsigned long addr = (unsigned long)vaddr;

	gpio_set_value(GPIO_PIN_1, addr & GPIO_MASK_1);

#ifdef GPIO_PIN_2
	gpio_set_value(GPIO_PIN_2, addr & GPIO_MASK_2);
#endif

#ifdef GPIO_PIN_3
	gpio_set_value(GPIO_PIN_3, addr & GPIO_MASK_3);
#endif

	SSYNC();
	udelay(1);

	return (void *)(addr & ~GPIO_MASK);
}

#define __raw_writeq(value, addr) *(volatile u64 *)addr = value
#define __raw_readq(addr) *(volatile u64 *)addr

#define MAKE_FLASH(size, sfx) \
void flash_write##size(u##size value, void *addr) \
{ \
	__raw_write##sfx(value, gpio_cfi_flash_swizzle(addr)); \
} \
u##size flash_read##size(void *addr) \
{ \
	return __raw_read##sfx(gpio_cfi_flash_swizzle(addr)); \
}
MAKE_FLASH(8, b)  /* flash_write8()  flash_read8() */
MAKE_FLASH(16, w) /* flash_write16() flash_read16() */
MAKE_FLASH(32, l) /* flash_write32() flash_read32() */
MAKE_FLASH(64, q) /* flash_write64() flash_read64() */

void gpio_cfi_flash_init(void)
{
	gpio_request(GPIO_PIN_1, "gpio_cfi_flash");
	gpio_direction_output(GPIO_PIN_1, 0);
#ifdef GPIO_PIN_2
	gpio_request(GPIO_PIN_2, "gpio_cfi_flash");
	gpio_direction_output(GPIO_PIN_2, 0);
#endif
#ifdef GPIO_PIN_3
	gpio_request(GPIO_PIN_3, "gpio_cfi_flash");
	gpio_direction_output(GPIO_PIN_3, 0);
#endif
}

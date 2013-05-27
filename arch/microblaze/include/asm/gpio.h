#ifndef _ASM_MICROBLAZE_GPIO_H_
#define _ASM_MICROBLAZE_GPIO_H_

#include <asm-generic/gpio.h>

/* Allocation functions */
extern int gpio_alloc_dual(u32 baseaddr, const char *name, u32 gpio_no0,
			   u32 gpio_no1);
extern int gpio_alloc(u32 baseaddr, const char *name, u32 gpio_no);

#define gpio_status()	gpio_info()
extern void gpio_info(void);

#endif


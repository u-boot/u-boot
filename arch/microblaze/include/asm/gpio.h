#ifndef _ASM_MICROBLAZE_GPIO_H_
#define _ASM_MICROBLAZE_GPIO_H_

#include <asm/io.h>

static inline int gpio_request(unsigned gpio, const char *label)
{
	return 0;
}

static inline int gpio_free(unsigned gpio)
{
	return 0;
}

static inline int gpio_direction_input(unsigned gpio)
{
	return 0;
}

static inline int gpio_direction_output(unsigned gpio, int value)
{
	return 0;
}

static inline int gpio_get_value(unsigned gpio)
{
	return 0;
}

static inline int gpio_set_value(unsigned gpio, int value)
{
	return 0;
}

static inline int gpio_is_valid(int number)
{
	return 0;
}
#endif


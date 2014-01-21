/*
 * Copyright (c) 2013 Xilinx, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ZYNQ_GPIO_H
#define _ZYNQ_GPIO_H

inline int gpio_get_value(unsigned gpio)
{
	return 0;
}

inline int gpio_set_value(unsigned gpio, int val)
{
	return 0;
}

inline int gpio_request(unsigned gpio, const char *label)
{
	return 0;
}

#endif /* _ZYNQ_GPIO_H */
